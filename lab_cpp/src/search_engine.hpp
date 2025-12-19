#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "query_parser.hpp"

struct TermInfo {
    uint32_t doc_freq;
    uint32_t offset;
};

class DictionaryMap {
private:
    struct Node {
        std::string key;
        TermInfo value;
    };
    
    std::vector<std::vector<Node>> buckets;
    size_t table_size;

    size_t get_hash(const std::string& key) const {
        size_t hash = 5381;
        for (char c : key) hash = ((hash << 5) + hash) + c;
        return hash;
    }

public:
    DictionaryMap(size_t size = 50000) : table_size(size) {
        buckets.resize(table_size);
    }

    void insert(const std::string& key, TermInfo val) {
        size_t idx = get_hash(key) % table_size;
        buckets[idx].push_back({key, val});
    }

    TermInfo* find(const std::string& key) {
        size_t idx = get_hash(key) % table_size;
        for (auto& node : buckets[idx]) {
            if (node.key == key) {
                return &node.value;
            }
        }
        return nullptr;
    }
};

struct SearchResult {
    uint32_t doc_id;
    std::string title;
    std::string url;
};

class SearchEngine {
public:
    void load_index(const std::string& index_dir);
    std::vector<SearchResult> search(const std::string& query);
    uint32_t get_total_docs() const { return static_cast<uint32_t>(doc_titles.size()); }
    static std::vector<uint32_t> intersect_postings(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b);
    static std::vector<uint32_t> union_postings(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b);
    static std::vector<uint32_t> difference_postings(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b);

private:
    std::string index_dir;
    
    DictionaryMap dictionary; 
    
    std::vector<std::string> doc_titles;

    std::vector<uint32_t> get_postings(const std::string& term);
    std::vector<uint32_t> get_all_doc_ids();
    std::vector<uint32_t> execute_rpn(const std::vector<Token>& rpn);
};