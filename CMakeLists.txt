cmake_minimum_required(VERSION 3.10)
project(mini-muduo)

set(CXX_FLAGS
 -g
 -Wall
 -std=c++17
 )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(muduo)
add_subdirectory(samples/echo)
add_subdirectory(samples/http)