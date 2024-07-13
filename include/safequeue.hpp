// C++ implementation of the above approach 
#include <condition_variable> 
#include <iostream> 
#include <mutex> 
#include <queue> 

// Thread-safe queue 
template <typename T> 
class TSQueue { 
private: 
	// Underlying queue 
	std::queue<T> m_queue; 

	// mutex for thread synchronization 
	std::mutex m_mutex; 

public: 
	// Pushes an element to the queue 
	void push(T item) 
	{ 
		std::lock_guard<std::mutex> lock(m_mutex); 
		m_queue.push(item); 
	} 

    bool empty() 
	{ 
		std::lock_guard<std::mutex> lock(m_mutex); 
		return m_queue.empty(); 
	} 

	// Pops an element off the queue 
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
        std::cout << "Message Queue content: ";
        while (!tempQueue.empty()) {
            std::cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        std::cout << std::endl;
    }
}; 

