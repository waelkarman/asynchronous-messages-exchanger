cmake_minimum_required(VERSION 3.16)

project(custom-exceptions)

set(SOURCES
    create_socket_exception.cpp
    recv_data_exception.cpp
    send_data_exception.cpp
    broken_pipe_exception.cpp
)

set(HEADERS
    create_socket_exception.hpp
    recv_data_exception.hpp
    send_data_exception.hpp
    broken_pipe_exception.hpp
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)