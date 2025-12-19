#include "tokenizer.hpp"
#include "stemmer.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

size_t get_utf8_char_len(char c) {
    if ((c & 0x80) == 0) return 1;          // ASCII 
    if ((c & 0xE0) == 0xC0) return 2;       // 110xxxxx 
    if ((c & 0xF0) == 0xE0) return 3;       // 1110xxxx
    if ((c & 0xF8) == 0xF0) return 4;       // 11110xxx
    return 1; 
}

// А-Я (U+0410..U+042F) -> а-я (U+0430..U+044F)
// Ё (U+0401) -> ё (U+0451)
std::string Tokenizer::to_lower_utf8(const std::string& str) {
    std::string res;
    res.reserve(str.size());

    for (size_t i = 0; i < str.size(); ) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        size_t len = get_utf8_char_len(str[i]);

        if (len == 1) {
            if (c >= 'A' && c <= 'Z') {
                res += (char)(c + 32);
            } else {
                res += str[i];
            }
            i++;
            continue;
        }

        if (len == 2 && i + 1 < str.size()) {
            unsigned char c2 = static_cast<unsigned char>(str[i+1]);
            if (c == 0xD0 && (c2 >= 0x90 && c2 <= 0xAF)) {
                res += (char)0xD0;
                res += (char)(c2 + 0x20);
            }
            else if (c == 0xD0 && c2 == 0x81) {
                res += (char)0xD1;
                res += (char)0x91;
            }
            else {
                res += str[i];
                res += str[i+1];
            }
            i += 2;
        } else {
            for (size_t k = 0; k < len && i + k < str.size(); k++) {
                res += str[i + k];
            }
            i += len;
        }
    }
    return res;
}

bool Tokenizer::is_separator(char c) {
    static const std::string separators = " \t\n\r.,!?:;\"'()[]{}-<>/|\\*&^%$#@~`=";
    return separators.find(c) != std::string::npos;
}

void Tokenizer::tokenize_file(const std::string& filepath, CustomMap& map) {
    std::ifstream file(filepath, std::ios::binary); 
    if (!file.is_open()) return;

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string content(size, ' ');
    file.seekg(0);
    file.read(&content[0], size);
    file.close();

    std::string current_token;
    
    for (size_t i = 0; i < content.size(); ) {
        unsigned char c = content[i];
        size_t len = get_utf8_char_len(content[i]);
        
        if (len == 1 && is_separator(content[i])) {
            if (!current_token.empty()) {
                std::string lower = to_lower_utf8(current_token);
                std::string stemmed = Stemmer::stem(lower); 
                if (!stemmed.empty()) {
                    map.increment(stemmed);
                }
                current_token.clear();
            }
            i++;
            continue;
        }
        
        for (size_t k = 0; k < len && i + k < content.size(); k++) {
            current_token += content[i + k];
        }
        i += len;
    }

    if (!current_token.empty()) {
        std::string lower = to_lower_utf8(current_token);
        std::string stemmed = Stemmer::stem(lower); 
        if (!stemmed.empty()) {
            map.increment(stemmed);
        }
        current_token.clear();
    }
}