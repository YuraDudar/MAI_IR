#include "indexer.hpp"
#include "binary_utils.hpp"
#include "stemmer.hpp"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

std::string Indexer::read_title(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string title;
    if (std::getline(file, title)) {
        if (!title.empty() && title.back() == '\r') title.pop_back();
        return title;
    }
    return "No Title";
}

void Indexer::build_index(const std::string& corpus_path, const std::string& output_dir) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<DocMeta> docs;
    std::vector<IndexEntry> all_entries; 
    
    all_entries.reserve(10000000); 

    std::cout << "1. Scanning corpus and tokenizing..." << std::endl;
    
    Tokenizer tokenizer;
    uint32_t current_doc_id = 0;
    long long total_text_size = 0;

    for (const auto& entry : fs::directory_iterator(corpus_path)) {
        if (entry.path().extension() == ".txt") {
            std::string path = entry.path().string();
            total_text_size += fs::file_size(entry.path());
            
            // 1. Метаданные
            DocMeta meta;
            meta.id = current_doc_id;
            meta.path = path;
            meta.title = read_title(path);
            docs.push_back(meta);

            // 2. Токенизация 
            CustomMap doc_tokens;
            tokenizer.tokenize_file(path, doc_tokens);
            
            auto items = doc_tokens.get_all_items();
            for (const auto& item : items) {
                all_entries.push_back({item.key, current_doc_id});
            }

            current_doc_id++;
            if (current_doc_id % 1000 == 0) {
                std::cout << "\rProcessed " << current_doc_id << " docs..." << std::flush;
            }
        }
    }
    std::cout << "\nTotal documents: " << docs.size() << std::endl;

    // 3. Сортировка 
    std::cout << "2. Sorting " << all_entries.size() << " entries..." << std::endl;
    std::sort(all_entries.begin(), all_entries.end());

    // 4. Сохранение
    std::cout << "3. Writing indexes to disk..." << std::endl;
    fs::create_directories(output_dir);
    
    save_forward_index(docs, output_dir + "/docs_index.bin");
    save_inverted_index(all_entries, output_dir + "/inverted_index.bin");

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    double total_mb = total_text_size / 1024.0 / 1024.0;
    
    std::cout << "\n=== INDEXING REPORT ===" << std::endl;
    std::cout << "Total time: " << elapsed.count() << " sec" << std::endl;
    std::cout << "Indexing Speed: " << (total_mb / elapsed.count()) << " MB/s" << std::endl;
    std::cout << "Speed per doc: " << (elapsed.count() / docs.size() * 1000) << " ms/doc" << std::endl;
}

void Indexer::save_forward_index(const std::vector<DocMeta>& docs, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    
    BinaryUtils::write_u32(out, 0x53434F44); 
    BinaryUtils::write_u32(out, (uint32_t)docs.size());
    
    for (const auto& doc : docs) {
        uint16_t title_len = (uint16_t)std::min(doc.title.size(), (size_t)65535);
        BinaryUtils::write_u16(out, title_len);
        out.write(doc.title.data(), title_len);
        
        BinaryUtils::write_u16(out, 0); 
    }
}

void Indexer::save_inverted_index(std::vector<IndexEntry>& entries, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    
    uint32_t unique_terms = 0;
    long long total_term_len = 0;

    if (!entries.empty()) {
        unique_terms = 1;
        total_term_len += entries[0].term.size();
        for (size_t i = 1; i < entries.size(); ++i) {
            if (entries[i].term != entries[i-1].term) {
                unique_terms++;
                total_term_len += entries[i].term.size();
            }
        }
    }

    std::cout << "Total unique terms: " << unique_terms << std::endl;
    if (unique_terms > 0)
        std::cout << "Avg term length: " << (double)total_term_len / unique_terms << " bytes" << std::endl;

    BinaryUtils::write_u32(out, 0x5A584449); 
    BinaryUtils::write_u8(out, 1);           
    BinaryUtils::write_u32(out, unique_terms);
    
    long long dictionary_start = out.tellp();
    
    struct DictFixup {
        long long offset_pos_in_file; 
        uint32_t doc_freq;
        std::vector<uint32_t> postings; 
    };
    std::vector<DictFixup> fixups;
    fixups.reserve(unique_terms);
    
    if (entries.empty()) return;

    size_t i = 0;
    while (i < entries.size()) {
        std::string current_term = entries[i].term;
        std::vector<uint32_t> current_postings;
        
        while (i < entries.size() && entries[i].term == current_term) {
            if (current_postings.empty() || current_postings.back() != entries[i].doc_id) {
                current_postings.push_back(entries[i].doc_id);
            }
            i++;
        }
        
        uint8_t len = (uint8_t)std::min(current_term.size(), (size_t)255);
        BinaryUtils::write_u8(out, len);
        out.write(current_term.data(), len);
        
        BinaryUtils::write_u32(out, (uint32_t)current_postings.size()); 
        
        long long offset_pos = out.tellp();
        BinaryUtils::write_u32(out, 0); 
        
        fixups.push_back({offset_pos, (uint32_t)current_postings.size(), std::move(current_postings)});
    }
    
    long long postings_start = out.tellp();
    
    for (auto& fix : fixups) {
        long long current_pos = out.tellp();
        
        out.seekp(fix.offset_pos_in_file);
        BinaryUtils::write_u32(out, (uint32_t)current_pos);
        
        out.seekp(current_pos);
        
        for (uint32_t doc_id : fix.postings) {
            BinaryUtils::write_u32(out, doc_id);
        }
    }
}