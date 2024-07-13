cmake_minimum_required(VERSION 3.16)

project(udp-task)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(libs)