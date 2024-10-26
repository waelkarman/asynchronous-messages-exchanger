#pragma once
#include <iostream>
#include <mutex>
#include <deque>
#include <shared_mutex>
#include <stdexcept>

using namespace std;

template <typename T>
class TSDeQueue {
private:
    deque<T> m_queue;
    mutable shared_mutex m_rw_mutex; 

public:
    void push(const T& item) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        m_queue.push_back(item);
    }

    void push_front(const T& item) {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        m_queue.push_front(item);
    }

    bool empty() const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        return m_queue.empty();
    }

    void pop() {
        unique_lock<shared_mutex> lock(m_rw_mutex);
        if (m_queue.empty()) {
            throw out_of_range("Cannot pop from an empty queue");
        }
        m_queue.pop_front();
    }

    T front() const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        if (m_queue.empty()) {
            throw out_of_range("Cannot access front of an empty queue");
        }
        return m_queue.front();
    }

    void printQueue() const {
        shared_lock<shared_mutex> lock(m_rw_mutex);
        cout << "Queue content: ";
        for (const auto& item : m_queue) {
            cout << item << " ";
        }
        cout << endl;
    }
};
