#pragma once
#include <iostream>
#include <mutex>
#include <deque>

using namespace std;

template <typename T>
class TSQueue {
private:
    deque<T> m_queue;
    mutex m_mutex;

public:
    void push(const T& item) {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push_back(item);
    }
	
	void push_front(const T& item) {
        lock_guard<mutex> lock(m_mutex);
        m_queue.push_front(item);
    }

    bool empty() {
        lock_guard<mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void pop() {
        lock_guard<mutex> lock(m_mutex);
        m_queue.pop_front();
    }

    T front() {
        lock_guard<mutex> lock(m_mutex);
        return m_queue.front();
    }

    void printQueue() {
        lock_guard<mutex> lock(m_mutex);
        deque<T> tempQueue = m_queue;
        cout << "Queue content: ";
        while (!tempQueue.empty()) {
            cout << tempQueue.front() << " ";
            tempQueue.pop_front();
        }
        cout << endl;
    }
};

