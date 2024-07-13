#pragma once
#include <iostream>
#include <map>
#include <mutex>

template <typename K, typename V>
class TSMap {
private:
    std::map<K, V> m_map;
    std::mutex m_mutex;

public:
    void insert(const K& key,const V& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map[key] = value;
    }

    void erase(const K& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map.erase(key);
    }

    V get(const K& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_map[key];
    }

	bool find(const K& key){
		if(m_map.find(key) != m_map.end()){
			return true;
		}else{
			return false;
		}
	}

    bool empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_map.empty();
    }
};