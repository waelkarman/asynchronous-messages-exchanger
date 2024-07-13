cmake_minimum_required(VERSION 3.16)

project(server-udp)

set(SOURCES
    server-udp.cpp
)

set(HEADERS)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
