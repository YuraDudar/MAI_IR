#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

template <class T, class U>
void AssertEqual(const T& t, const U& u, const std::string& hint = {}) {
    if (t != u) {
        std::ostringstream os;
        os << "Assertion failed: " << t << " != " << u;
        if (!hint.empty()) {
            os << " hint: " << hint;
        }
        throw std::runtime_error(os.str());
    }
}

void Assert(bool b, const std::string& hint) {
    AssertEqual(b, true, hint);
}

template <class TestFunc>
void RunTest(TestFunc func, const std::string& test_name) {
    try {
        func();
        std::cerr << "[ OK ] " << test_name << std::endl;
    } catch (std::exception& e) {
        std::cerr << "[FAIL] " << test_name << " -> " << e.what() << std::endl;
        exit(1); 
    }
}