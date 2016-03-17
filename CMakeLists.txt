
cmake_minimum_required(VERSION 2.8)
project(MorphSnakes)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_definitions(-std=c++11)

add_executable(test1 tests/test1.cpp)
