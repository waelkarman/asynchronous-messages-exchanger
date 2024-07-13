#pragma once
#include <iostream>
#include <map>
#include <mutex>

using namespace std;
template <typename K, typename V>
class TSMap {
private:
    map<K, V> m_map;
    mutex m_mutex;

public:
    void insert(const K& key,const V& value) {
        lock_guard<mutex> lock(m_mutex);
        m_map[key] = value;
    }

    void erase(const K& key) {
        lock_guard<mutex> lock(m_mutex);
        m_map.erase(key);
    }

    V get(const K& key) {
        lock_guard<mutex> lock(m_mutex);
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
        lock_guard<mutex> lock(m_mutex);
        return m_map.empty();
    }
};