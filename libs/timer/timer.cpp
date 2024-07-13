#include <timer.hpp>

void Timer::reset() {
    start_ = std::chrono::high_resolution_clock::now();
}

double Timer::elapsed() const {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    return duration.count();
}

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
    std::cout << "Time on destruction: " << duration.count() << " ms" << std::endl;
}
