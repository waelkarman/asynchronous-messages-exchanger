#pragma once
#include <iostream>
#include <mutex>
#include <vector>
#include <shared_mutex>
#include <stdexcept>

using namespace std;

template <typename T>
class TSVector {
private:
    vector<T> m_vector;
    mutable shared_mutex m_rw_mutex;

public:
    void push_back(const T& item) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        m_vector.push_back(item);
    }

    void push_back(T&& item) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        m_vector.push_back(move(item));
    }

    typename vector<T>::iterator erase(typename vector<T>::iterator item) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        return m_vector.erase(item);
    }

    typename vector<T>::iterator begin() {
        return m_vector.begin();
    }

    typename vector<T>::iterator end() {
        return m_vector.end();
    }

    size_t size() const { 
        shared_lock<shared_mutex> lock(m_rw_mutex);
        return m_vector.size();
    }

    void pop_back() {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        if (m_vector.empty()) {
            throw out_of_range("Cannot pop from an empty vector");
        }
        m_vector.pop_back();
    }

    T& back() {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        if (m_vector.empty()) {
            throw out_of_range("Cannot access back of an empty vector");
        }
        return m_vector.back();
    }

    const T& back() const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        if (m_vector.empty()) {
            throw out_of_range("Cannot access back of an empty vector");
        }
        return m_vector.back();
    }

    bool empty() const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        return m_vector.empty();
    }
};
