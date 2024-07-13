cmake_minimum_required(VERSION 3.16)

project(datapacket)

set(SOURCES
    datapacket.cpp
)

set(HEADERS
    datapacket.hpp
)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
