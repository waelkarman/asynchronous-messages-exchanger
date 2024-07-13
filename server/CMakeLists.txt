cmake_minimum_required(VERSION 3.16)

project(udp-server)

set(SOURCES
    udp-server.cpp
)

set(HEADERS)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
