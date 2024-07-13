#pragma once

#include <chrono>
#include <iostream>

/**
* This code is used to measure the time in Cplus code snippets.
*
*/

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    void reset();

    double elapsed() const;

    ~Timer();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};