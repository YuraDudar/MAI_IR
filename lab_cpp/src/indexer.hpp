#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "tokenizer.hpp"

struct IndexEntry {
    std::string term;
    uint32_t doc_id;

    bool operator<(const IndexEntry& other) const {
        if (term != other.term) {
            return term < other.term;
        }
        return doc_id < other.doc_id;
    }
};

struct DocMeta {
    uint32_t id;
    std::string title;
    std::string path;
};

class Indexer {
public:
    void build_index(const std::string& corpus_path, const std::string& output_dir);

private:
    void save_forward_index(const std::vector<DocMeta>& docs, const std::string& filename);
    void save_inverted_index(std::vector<IndexEntry>& entries, const std::string& filename);
    
    std::string read_title(const std::string& filepath);
};