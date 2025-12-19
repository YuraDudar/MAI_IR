#include "query_parser.hpp"
#include <sstream>
#include <cctype>

bool is_operator_char(char c) {
    return c == '&' || c == '|' || c == '!' || c == '(' || c == ')';
}

std::vector<Token> QueryParser::parse_to_rpn(const std::string& query) {
    std::vector<Token> output_queue;
    std::stack<Token> operator_stack;

    std::vector<Token> tokens;
    for (size_t i = 0; i < query.length(); ++i) {
        char c = query[i];
        if (std::isspace(c)) continue;

        if (c == '(') tokens.push_back({LPAREN, "(", 0});
        else if (c == ')') tokens.push_back({RPAREN, ")", 0});
        else if (c == '!') tokens.push_back({NOT, "!", 3}); 
        else if (c == '&') {
            if (i + 1 < query.length() && query[i+1] == '&') i++;
            tokens.push_back({AND, "&&", 2});
        }
        else if (c == '|') {
            if (i + 1 < query.length() && query[i+1] == '|') i++;
            tokens.push_back({OR, "||", 1});
        }
        else {
            std::string term;
            while (i < query.length() && !is_operator_char(query[i]) && !std::isspace(query[i])) {
                term += query[i];
                i++;
            }
            i--;
            tokens.push_back({TERM, term, 0});
        }
    }

    std::vector<Token> processed_tokens;
    if (!tokens.empty()) {
        processed_tokens.push_back(tokens[0]);
        for (size_t i = 1; i < tokens.size(); ++i) {
            TokenType prev = tokens[i-1].type;
            TokenType curr = tokens[i].type;
            
            bool need_and = false;
            if (prev == TERM && curr == TERM) need_and = true;
            if (prev == TERM && curr == LPAREN) need_and = true;
            if (prev == RPAREN && curr == TERM) need_and = true;
            if (prev == TERM && curr == NOT) need_and = true;

            if (need_and) {
                processed_tokens.push_back({AND, "&&", 2});
            }
            processed_tokens.push_back(tokens[i]);
        }
    } else {
        processed_tokens = tokens;
    }

    for (const auto& token : processed_tokens) {
        if (token.type == TERM) {
            output_queue.push_back(token);
        } else if (token.type == LPAREN) {
            operator_stack.push(token);
        } else if (token.type == RPAREN) {
            while (!operator_stack.empty() && operator_stack.top().type != LPAREN) {
                output_queue.push_back(operator_stack.top());
                operator_stack.pop();
            }
            if (!operator_stack.empty()) operator_stack.pop(); 
        } else {
            while (!operator_stack.empty() && 
                   operator_stack.top().type != LPAREN &&
                   operator_stack.top().precedence >= token.precedence) {
                output_queue.push_back(operator_stack.top());
                operator_stack.pop();
            }
            operator_stack.push(token);
        }
    }

    while (!operator_stack.empty()) {
        output_queue.push_back(operator_stack.top());
        operator_stack.pop();
    }

    return output_queue;
}