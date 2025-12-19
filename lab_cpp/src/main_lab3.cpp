#include <iostream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <iomanip>
#include "custom_map.hpp"
#include "tokenizer.hpp"

namespace fs = std::filesystem;

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    std::cout << "=== Lab 3: Tokenization & Zipf Law ===" << std::endl;
    std::string corpus_path = "../../corpus_txt";

    if (!fs::exists(corpus_path)) {
        std::cerr << "Error: Directory " << corpus_path << " not found!" << std::endl;
        std::cerr << "Please run export_corpus.py first." << std::endl;
        return 1;
    }

    CustomMap freq_dict(100000); 
    Tokenizer tokenizer;

    long long total_files = 0;
    long long total_bytes = 0;
    
    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "Reading files from: " << corpus_path << std::endl;
    for (const auto& entry : fs::directory_iterator(corpus_path)) {
        if (entry.path().extension() == ".txt") {
            std::string filepath = entry.path().string();
            
            total_bytes += fs::file_size(entry.path());
            total_files++;

            tokenizer.tokenize_file(filepath, freq_dict);
            
            if (total_files % 1000 == 0) {
                std::cout << "Processed " << total_files << " files..." << std::endl;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;

    std::cout << "\n=== STATISTICS ===" << std::endl;
    std::cout << "Total files: " << total_files << std::endl;
    std::cout << "Total size:  " << total_bytes / 1024.0 / 1024.0 << " MB" << std::endl;
    std::cout << "Unique tokens: " << freq_dict.size() << std::endl;
    
    long long total_tokens_count = 0;
    long long total_token_length = 0;
    auto all_items = freq_dict.get_all_items();
    
    for (const auto& item : all_items) {
        total_tokens_count += item.value;
        total_token_length += (item.key.length() * item.value);
    }
    
    std::cout << "Total tokens: " << total_tokens_count << std::endl;
    if (total_tokens_count > 0) {
        std::cout << "Avg token length: " << (double)total_token_length / total_tokens_count << std::endl;
    }

    std::cout << "\n=== PERFORMANCE ===" << std::endl;
    std::cout << "Time elapsed: " << elapsed.count() << " seconds" << std::endl;
    double speed_kb_s = (total_bytes / 1024.0) / elapsed.count();
    std::cout << "Processing speed: " << speed_kb_s << " KB/s" << std::endl;

    std::ofstream csv_file("zipf_data.csv");
    csv_file << "word,frequency\n";
    for (const auto& item : all_items) {
        csv_file << item.key << "," << item.value << "\n";
    }
    csv_file.close();
    std::cout << "\nData for Zipf plot saved to 'zipf_data.csv'" << std::endl;

    return 0;
}