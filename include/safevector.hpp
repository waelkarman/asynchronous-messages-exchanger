#pragma once
#include <iostream>
#include <mutex>
#include <vector>

using namespace std;

template <typename T>
class TSVector {
private:
    vector<T> m_vector;
    mutex m_mutex;

public:

    void push_back(const T& item) {
        lock_guard<mutex> lock(m_mutex);
        m_vector.push_back(item);
    }

    void push_back(T&& item) {
        lock_guard<mutex> lock(m_mutex);
        m_vector.push_back(move(item));
    }

    void erase(typename vector<T>::iterator item) {
        lock_guard<mutex> lock(m_mutex);
        m_vector.erase(item);
    }

    typename vector<T>::iterator begin() {
        lock_guard<mutex> lock(m_mutex);
        return m_vector.begin();
    }

    typename vector<T>::iterator end() {
        lock_guard<mutex> lock(m_mutex);
        return m_vector.end();
    }

    bool size() {
        lock_guard<mutex> lock(m_mutex);
        return m_vector.size();
    }

    void pop_back() {
        lock_guard<mutex> lock(m_mutex);
        m_vector.pop_back();
    }

    T& back() {
        lock_guard<mutex> lock(m_mutex);
        return m_vector.back();
    }

    const T& back() const {
        lock_guard<mutex> lock(m_mutex);
        return m_vector.back();
    }

    bool empty() {
        lock_guard<mutex> lock(m_mutex);
        return m_vector.empty();
    }
};

