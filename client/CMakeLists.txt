cmake_minimum_required(VERSION 3.16)

project(client-udp)

set(SOURCES
    client-udp.cpp
)

set(HEADERS)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
