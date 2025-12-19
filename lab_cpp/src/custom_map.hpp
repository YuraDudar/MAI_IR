#pragma once

#include <vector>
#include <string>
#include <iostream>

struct Node {
    std::string key;
    int value;
    
    Node(std::string k, int v) : key(k), value(v) {}
};

class CustomMap {
private:
    std::vector<std::vector<Node>> buckets;
    size_t table_size;
    size_t element_count;

    size_t get_hash(const std::string& key) const {
        size_t hash = 5381;
        for (char c : key) {
            hash = ((hash << 5) + hash) + c; 
        }
        return hash;
    }

public:
    CustomMap(size_t size = 10000) : table_size(size), element_count(0) {
        buckets.resize(table_size);
    }

    void increment(const std::string& key) {
        size_t index = get_hash(key) % table_size;
        
        for (auto& node : buckets[index]) {
            if (node.key == key) {
                node.value++;
                return;
            }
        }
        
        buckets[index].emplace_back(key, 1);
        element_count++;
    }

    int get(const std::string& key) const {
        size_t index = get_hash(key) % table_size;
        for (const auto& node : buckets[index]) {
            if (node.key == key) {
                return node.value;
            }
        }
        return 0; 
    }

    std::vector<Node> get_all_items() const {
        std::vector<Node> items;
        items.reserve(element_count);
        for (const auto& bucket : buckets) {
            for (const auto& node : bucket) {
                items.push_back(node);
            }
        }
        return items;
    }

    size_t size() const {
        return element_count;
    }
};