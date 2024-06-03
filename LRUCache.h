#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <iostream>
#include <list>
#include <algorithm>
#include <tuple>

class LRUCache {
public:
    LRUCache(size_t maxSize) : maxSize(maxSize) {}      
    // Function to insert a tuple of two integers
    void insert(std::tuple<int, int> value) {
        // Check if the value already exists in the list
        auto it = std::find(cache.begin(), cache.end(), value);
        if (it != cache.end()) {
            // Move the existing element to the front
            cache.splice(cache.begin(), cache, it);
        } else {
            // If the list is full, remove the oldest element
            if (cache.size() >= maxSize) {
                cache.pop_back();
            }
            // Insert the new element at the front
            cache.push_front(value);
        }
    }

    // Function to get a tuple if it exists in the list
    std::tuple<int, int> get(int first, int second) {
        auto it = std::find(cache.begin(), cache.end(), std::make_tuple(first, second));
        if (it != cache.end()) {
            // Move the accessed item to the front of the list
            cache.splice(cache.begin(), cache, it);
            return *it;
        }
        return std::make_tuple(-1, -1); // Return a default value indicating not found
    }

    // Function to return the list for iteration
    const std::list<std::tuple<int, int>>& getCache() const {
        return cache;
    }

private:
    size_t maxSize;
    std::list<std::tuple<int, int>> cache; // List to maintain the order of elements
};

#endif // LRU_CACHE_H
