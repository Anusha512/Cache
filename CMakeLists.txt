cmake_minimum_required(VERSION 3.2)
project(Cache)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp cache.cpp)
add_executable(Cache ${SOURCE_FILES})