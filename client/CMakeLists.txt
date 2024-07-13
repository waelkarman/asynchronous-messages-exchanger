cmake_minimum_required(VERSION 3.16)

project(client-udp)

set(SOURCES
    client-udp.cpp
)

set(HEADERS)

add_subdirectory(datapacket)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    datapacket
)