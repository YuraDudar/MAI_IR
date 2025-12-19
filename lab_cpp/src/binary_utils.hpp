#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

namespace BinaryUtils {

    inline void write_u8(std::ofstream& out, uint8_t val) {
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }

    inline void write_u16(std::ofstream& out, uint16_t val) {
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }

    inline void write_u32(std::ofstream& out, uint32_t val) {
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }

    inline void write_string(std::ofstream& out, const std::string& str) {
        out.write(str.data(), str.size());
    }
    
    inline uint32_t read_u32(std::ifstream& in) {
        uint32_t val;
        in.read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    }
}