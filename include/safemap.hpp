#include <iostream>
#include <map>
#include <mutex>
#include <condition_variable>

template <typename K, typename V>
class TSMap {
private:
    std::map<K, V> m_map;
    std::mutex m_mutex;

public:
    void insert( K& key, V & value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map[key] = value;
    }

    void erase( K key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map.erase(key);
    }

    // Recupera il valore associato a una chiave specifica
    V get( K key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_map[key];
    }

	bool find( K key){
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