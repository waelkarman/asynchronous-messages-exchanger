#pragma once
#include <iostream> 
#include <mutex> 
#include <queue> 

using namespace std;
template <typename T> 
class TSQueue { 
private: 
	queue<T> m_queue;  
	mutex m_mutex; 

public: 
	void push(const T& item) 
	{ 
		lock_guard<mutex> lock(m_mutex); 
		m_queue.push(item); 
	} 

    bool empty() 
	{ 
		lock_guard<mutex> lock(m_mutex); 
		return m_queue.empty(); 
	} 

	void pop() 
	{ 
		lock_guard<mutex> lock(m_mutex); 
		m_queue.pop(); 
	} 

    T front() 
	{ 
		lock_guard<mutex> lock(m_mutex); 
		return m_queue.front();
	} 

	void printQueue() {
        queue<T> tempQueue = m_queue;
        cout << "Queue content: ";
        while (!tempQueue.empty()) {
            cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        cout << endl;
    }
}; 

