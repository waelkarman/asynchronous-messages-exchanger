#pragma once
#include <iostream>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

using namespace std;

template <typename K, typename V>
class TSMap {
private:
    map<K, V> m_map;
    mutable shared_mutex m_rw_mutex;

public:
    void insert(const K& key, const V& value) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        m_map[key] = value;
    }

    void erase(const K& key) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        m_map.erase(key);
    }

    V get(const K& key) const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            return it->second;
        }
        throw out_of_range("Key not found");
    }

    bool find(const K& key) const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        return m_map.find(key) != m_map.end();
    }

    bool empty() const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        return m_map.empty();
    }
};
