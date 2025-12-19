#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "custom_map.hpp"

class Tokenizer {
public:
    void tokenize_file(const std::string& filepath, CustomMap& map);

    static std::string to_lower_utf8(const std::string& str);
    static bool is_separator(char c);
};