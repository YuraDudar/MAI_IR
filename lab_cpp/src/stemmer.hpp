#pragma once
#include <string>
#include <regex>

class Stemmer {
public:
    static std::string stem(const std::string& word);
};