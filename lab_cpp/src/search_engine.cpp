#include "search_engine.hpp"
#include "binary_utils.hpp"
#include "stemmer.hpp"
#include "tokenizer.hpp"
#include <algorithm>
#include <stack>
#include <numeric>
#include <iostream>
#include <set>

void SearchEngine::load_index(const std::string& dir) {
    index_dir = dir;
    std::cerr << "Loading index from " << dir << "..." << std::endl;

    std::ifstream docs_in(dir + "/docs_index.bin", std::ios::binary);
    if (!docs_in.is_open()) throw std::runtime_error("Cannot open docs_index.bin");

    uint32_t sig = BinaryUtils::read_u32(docs_in);
    if (sig != 0x53434F44) throw std::runtime_error("Invalid docs index signature");

    uint32_t count = BinaryUtils::read_u32(docs_in);
    doc_titles.resize(count);

    for (uint32_t i = 0; i < count; ++i) {
        uint16_t title_len;
        docs_in.read((char*)&title_len, 2);
        
        std::string title(title_len, ' ');
        docs_in.read(&title[0], title_len);
        doc_titles[i] = title;

        uint16_t url_len;
        docs_in.read((char*)&url_len, 2);
        if (url_len > 0) docs_in.ignore(url_len);
    }
    std::cerr << "Loaded " << count << " document titles." << std::endl;

    std::ifstream inv_in(dir + "/inverted_index.bin", std::ios::binary);
    if (!inv_in.is_open()) throw std::runtime_error("Cannot open inverted_index.bin");

    sig = BinaryUtils::read_u32(inv_in);
    if (sig != 0x5A584449) throw std::runtime_error("Invalid inverted index signature");

    uint8_t ver; 
    inv_in.read((char*)&ver, 1);

    uint32_t term_count = BinaryUtils::read_u32(inv_in);
    std::cerr << "Loading " << term_count << " terms..." << std::endl;

    dictionary = DictionaryMap(static_cast<size_t>(term_count * 1.5));

    for (uint32_t i = 0; i < term_count; ++i) {
        uint8_t term_len;
        inv_in.read((char*)&term_len, 1);
        
        std::string term(term_len, ' ');
        inv_in.read(&term[0], term_len);
        
        uint32_t doc_freq = BinaryUtils::read_u32(inv_in);
        uint32_t offset = BinaryUtils::read_u32(inv_in);
        
        dictionary.insert(term, {doc_freq, offset});
    }
}

std::vector<uint32_t> SearchEngine::get_postings(const std::string& term) {
    TermInfo* info = dictionary.find(term);
    
    if (info == nullptr) return {};

    std::ifstream inv_in(index_dir + "/inverted_index.bin", std::ios::binary);
    if (!inv_in.is_open()) return {};

    inv_in.seekg(info->offset);

    std::vector<uint32_t> result(info->doc_freq);
    inv_in.read((char*)result.data(), result.size() * sizeof(uint32_t));
    
    return result;
}


std::vector<uint32_t> SearchEngine::intersect_postings(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    std::vector<uint32_t> res;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<uint32_t> SearchEngine::union_postings(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    std::vector<uint32_t> res;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<uint32_t> SearchEngine::difference_postings(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    std::vector<uint32_t> res;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(res));
    return res;
}

std::vector<uint32_t> SearchEngine::get_all_doc_ids() {
    std::vector<uint32_t> all(doc_titles.size());
    std::iota(all.begin(), all.end(), 0); 
    return all;
}

std::vector<uint32_t> SearchEngine::execute_rpn(const std::vector<Token>& rpn) {
    std::stack<std::vector<uint32_t>> stack;

    for (const auto& token : rpn) {
        if (token.type == TERM) {
            std::string term = Tokenizer::to_lower_utf8(token.value);
            term = Stemmer::stem(term);
            stack.push(get_postings(term));
        } 
        else if (token.type == NOT) {
            if (stack.empty()) continue;
            auto op1 = stack.top(); stack.pop();
            auto all_docs = get_all_doc_ids();
            stack.push(difference_postings(all_docs, op1));
        }
        else {
            if (stack.size() < 2) continue;
            auto op2 = stack.top(); stack.pop();
            auto op1 = stack.top(); stack.pop();

            if (token.type == AND) stack.push(intersect_postings(op1, op2));
            else if (token.type == OR) stack.push(union_postings(op1, op2));
        }
    }
    if (stack.empty()) return {};
    return stack.top();
}

std::vector<SearchResult> SearchEngine::search(const std::string& query) {
    auto rpn = QueryParser::parse_to_rpn(query);
    auto doc_ids = execute_rpn(rpn);
    std::vector<SearchResult> results;
    results.reserve(doc_ids.size());
    for (uint32_t id : doc_ids) {
        if (id < doc_titles.size()) {
            results.push_back({id, doc_titles[id], ""}); 
        }
    }
    return results;
}