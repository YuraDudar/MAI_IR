cmake_minimum_required(VERSION 3.10)
project(SearchEngineLabs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src)

# === ЛАБОРАТОРНАЯ 3: Токенизация и Ципф ===
add_executable(lab3 
    src/main_lab3.cpp
    src/tokenizer.cpp 
    src/stemmer.cpp
)

# === ЛАБОРАТОРНАЯ 4 (Часть 1): Построение индекса ===
add_executable(lab4_indexer 
    src/main_lab4.cpp
    src/indexer.cpp       
    src/tokenizer.cpp 
    src/stemmer.cpp
)

# === ЛАБОРАТОРНАЯ 4 (Часть 2): Поиск ===
add_executable(lab4_search
    src/main_search.cpp
    src/search_engine.cpp
    src/query_parser.cpp
    src/tokenizer.cpp
    src/stemmer.cpp
)

# === АВТОТЕСТЫ ===
add_executable(run_tests 
    src/tests/tests.cpp 
    src/tokenizer.cpp 
    src/stemmer.cpp
    src/query_parser.cpp   
    src/search_engine.cpp  
)