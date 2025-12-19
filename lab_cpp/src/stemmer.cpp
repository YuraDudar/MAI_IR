#include "stemmer.hpp"
#include <regex>

std::string Stemmer::stem(const std::string& word) {
    std::string rv = word;
    
    static const std::regex reflexive("(ся|сь)$");
    
    static const std::regex adjective("(ее|ие|ые|ое|ими|ыми|ей|ий|ый|ой|ем|им|ым|ом|его|ого|ому|ему|их|ых|ую|юю|ая|яя|ою|ею)$");
    
    static const std::regex participle("((ивш|ывш|ующ)|(ем|нн|вш|ющ|щ))$");
    
    static const std::regex verb(
        "((ила|ыла|ена|ейте|уйте|ите|или|ыли|ей|уй|ил|ыл|им|ым|ен|ило|ыло|ено|ят|ует|уют|ит|ыт|ены|ить|ыть|ишь|ую|ю|"
        "ала|яла|ела|али|яли|ели|ало|яло|ело|" 
        "ал|ял|ел)" 
        ")$");
    
    static const std::regex noun("(а|ев|ов|ие|ье|е|иями|ями|ами|еи|ии|и|ией|ей|ой|ий|й|иям|ям|ием|ем|ам|ом|о|у|ах|иях|ях|ы|ь|ию|ью|ю|ия|ья|я)$");
    
    static const std::regex superlative("(ейш|ейше)$");
    static const std::regex i_ending("и$");
    static const std::regex derivational(".*[^аеиоуыэюя](ост|ость)$");

    auto replace_if_match = [&](const std::regex& re) -> bool {
            std::smatch match;
            if (std::regex_search(rv, match, re)) {
                if (match.position() > 4) { 
                    rv = std::regex_replace(rv, re, "");
                    return true;
                }
            }
            return false;
        };

    replace_if_match(reflexive);

    if (replace_if_match(adjective)) {
        replace_if_match(participle);
    } else {
        if (!replace_if_match(verb)) {
            replace_if_match(noun);
        }
    }
    
    replace_if_match(i_ending);
    
    if (std::regex_match(rv, derivational)) {
         rv = std::regex_replace(rv, std::regex("ость?$"), "");
    }

    replace_if_match(superlative);
    
    return rv;
}