cmake_minimum_required(VERSION 3.16)

project(timer)

set(SOURCES
    timer.cpp
)

set(HEADERS
    timer.hpp
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
