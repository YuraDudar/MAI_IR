#pragma once
#include <string>
#include <vector>
#include <stack>
#include <iostream>

enum TokenType { TERM, AND, OR, NOT, LPAREN, RPAREN };

struct Token {
    TokenType type;
    std::string value;
    int precedence;
};

class QueryParser {
public:
    static std::vector<Token> parse_to_rpn(const std::string& query);
};