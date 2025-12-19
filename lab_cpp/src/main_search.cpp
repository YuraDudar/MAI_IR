#include <iostream>
#include <string>
#include "search_engine.hpp"

std::string escape_json(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\b') res += "\\b";
        else if (c == '\f') res += "\\f";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else res += c;
    }
    return res;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    bool json_mode = false;
    if (argc > 1 && std::string(argv[1]) == "--json") {
        json_mode = true;
    }

    std::string index_dir = "../../index_data";
    SearchEngine engine;
    
    try {
        engine.load_index(index_dir);
    } catch (const std::exception& e) {
        std::cerr << "Error loading index: " << e.what() << std::endl;
        return 1;
    }

    if (!json_mode) {
        std::cout << "Interactive Search Ready. Type 'exit' to quit." << std::endl;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") break;
        if (line.empty()) continue;

        try {
            auto results = engine.search(line);
            
            if (json_mode) {
                std::cout << "{ \"count\": " << results.size() << ", \"results\": [";
                for (size_t i = 0; i < results.size(); ++i) {
                    std::cout << "{ \"id\": " << results[i].doc_id 
                              << ", \"title\": \"" << escape_json(results[i].title) << "\" }";
                    if (i < results.size() - 1) std::cout << ",";
                }
                std::cout << "] }" << std::endl; 
            } else {
                std::cout << "Found " << results.size() << " docs." << std::endl;
                for (size_t i = 0; i < std::min((size_t)10, results.size()); ++i) {
                     std::cout << "[" << results[i].doc_id << "] " << results[i].title << std::endl;
                }
            }
        } catch (const std::exception& e) {
            if (json_mode) std::cout << "{ \"error\": \"" << escape_json(e.what()) << "\" }" << std::endl;
            else std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}