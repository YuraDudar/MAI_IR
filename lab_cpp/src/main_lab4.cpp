#include <iostream>
#include <filesystem>
#include <chrono>
#include "indexer.hpp"

namespace fs = std::filesystem;

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    std::cout << "=== Lab 4: Inverted Index Builder ===" << std::endl;

    std::string corpus_path = "../../corpus_txt";
    std::string index_output = "../../index_data"; 

    if (!fs::exists(corpus_path)) {
        std::cerr << "Error: Corpus not found at " << corpus_path << std::endl;
        std::cerr << "Please run export_corpus.py (Lab 1) first." << std::endl;
        return 1;
    }

    try {
        Indexer indexer;
        std::cout << "Starting indexing process..." << std::endl;
        
        indexer.build_index(corpus_path, index_output);
        
        std::cout << "\nSUCCESS: Index saved to: " << fs::absolute(index_output) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}