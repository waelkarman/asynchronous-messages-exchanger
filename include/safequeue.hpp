#pragma once
#include <iostream> 
#include <mutex> 
#include <queue> 

template <typename T> 
class TSQueue { 
private: 
	std::queue<T> m_queue;  
	std::mutex m_mutex; 

public: 
	void push(const T& item) 
	{ 
		std::lock_guard<std::mutex> lock(m_mutex); 
		m_queue.push(item); 
	} 

    bool empty() 
	{ 
		std::lock_guard<std::mutex> lock(m_mutex); 
		return m_queue.empty(); 
	} 

	void pop() 
	{ 
		std::lock_guard<std::mutex> lock(m_mutex); 
		m_queue.pop(); 
	} 

    T front() 
	{ 
		std::lock_guard<std::mutex> lock(m_mutex); 
		return m_queue.front();
	} 

	void printQueue() {
        std::queue<T> tempQueue = m_queue;
        std::cout << "Queue content: ";
        while (!tempQueue.empty()) {
            std::cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        std::cout << std::endl;
    }
}; 

