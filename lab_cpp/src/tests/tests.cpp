#include "test_runner.hpp"
#include "../custom_map.hpp"
#include "../tokenizer.hpp"
#include "../stemmer.hpp"
#include "../query_parser.hpp"
#include "../search_engine.hpp" 
#include <algorithm>
#include <set>


void TestCustomMapStress() {
    CustomMap map(10); 
    
    std::vector<std::string> keys;
    for (int i = 0; i < 100; ++i) {
        keys.push_back("key_" + std::to_string(i));
    }

    for (const auto& k : keys) {
        map.increment(k);
    }

    for (const auto& k : keys) {
        AssertEqual(map.get(k), 1, "Lost key in collision: " + k);
    }
    
    AssertEqual((int)map.size(), 100, "Total size mismatch");
    
    map.increment("key_0");
    AssertEqual(map.get("key_0"), 2, "Increment logic fail");
}


void TestStemmerExtended() {
    AssertEqual(Stemmer::stem("бегал"), "бег", "Verb 'al' removal");
    AssertEqual(Stemmer::stem("смотрела"), "смотр", "Verb 'la/ela' removal"); 
    AssertEqual(Stemmer::stem("гуляли"), "гул", "Verb 'li/yali' removal");
    
    AssertEqual(Stemmer::stem("компьютеры"), "компьютер", "Plural removal");
    AssertEqual(Stemmer::stem("окнами"), "окн", "Instrumental case");
    
    AssertEqual(Stemmer::stem("красный"), "красн", "Adjective 'yj'");
    AssertEqual(Stemmer::stem("большая"), "больш", "Adjective 'aya'");
    
    AssertEqual(Stemmer::stem("дом"), "дом", "Short word protection (<4 bytes body)");
    AssertEqual(Stemmer::stem("дома"), "дом", "Short word stemming (>4 bytes body)");
}


void TestBooleanLogic() {
    std::vector<uint32_t> docs_a = {1, 5, 10, 20};
    std::vector<uint32_t> docs_b = {5, 8, 10, 100};
    
    auto res_and = SearchEngine::intersect_postings(docs_a, docs_b);
    AssertEqual((int)res_and.size(), 2, "AND size mismatch");
    AssertEqual((int)res_and[0], 5, "AND element 1");
    AssertEqual((int)res_and[1], 10, "AND element 2");
    
    auto res_or = SearchEngine::union_postings(docs_a, docs_b);
    AssertEqual((int)res_or.size(), 6, "OR size mismatch");
    AssertEqual((int)res_or[2], 8, "OR check sort order");
    
    auto res_not = SearchEngine::difference_postings(docs_a, docs_b);
    AssertEqual((int)res_not.size(), 2, "NOT size mismatch");
    AssertEqual((int)res_not[0], 1, "NOT element 1");
    AssertEqual((int)res_not[1], 20, "NOT element 2");
    
    std::vector<uint32_t> empty;
    AssertEqual((int)SearchEngine::intersect_postings(docs_a, empty).size(), 0, "AND with empty");
    AssertEqual((int)SearchEngine::union_postings(docs_a, empty).size(), 4, "OR with empty");
}


std::string RpnToString(const std::vector<Token>& rpn) {
    std::string res;
    for (const auto& t : rpn) {
        if (!res.empty()) res += " ";
        res += t.value;
    }
    return res;
}

void TestQueryParser() {
    auto rpn1 = QueryParser::parse_to_rpn("A || B");
    AssertEqual(RpnToString(rpn1), "A B ||", "Simple OR");

    auto rpn2 = QueryParser::parse_to_rpn("A || B && C");
    AssertEqual(RpnToString(rpn2), "A B C && ||", "Precedence AND > OR");

    auto rpn3 = QueryParser::parse_to_rpn("(A || B) && C");
    AssertEqual(RpnToString(rpn3), "A B || C &&", "Parentheses logic");
    
    auto rpn4 = QueryParser::parse_to_rpn("путин налоги");
    AssertEqual(RpnToString(rpn4), "путин налоги &&", "Implicit AND");

    auto rpn5 = QueryParser::parse_to_rpn("Москва !метро");
    AssertEqual(RpnToString(rpn5), "Москва метро ! &&", "Implicit AND with NOT");
    
    auto rpn6 = QueryParser::parse_to_rpn("  A    B  ");
    AssertEqual(RpnToString(rpn6), "A B &&", "Whitespace tolerance");
}

int main() {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    
    std::cerr << "=== RUNNING EXTENDED TESTS ===" << std::endl;
    
    RunTest(TestCustomMapStress, "CustomMap Stress Test");
    RunTest(TestStemmerExtended, "Stemmer Extended Russian");
    RunTest(TestBooleanLogic,    "Boolean Set Operations");
    RunTest(TestQueryParser,     "Shunting-Yard Query Parser");
    
    return 0;
}