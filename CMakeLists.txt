
cmake_minimum_required(VERSION 2.8)
project(MorphSnakes)

find_package(Threads)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_definitions(-std=c++11)

if(CMAKE_THREAD_LIBS_INIT)
    add_executable(test1 tests/test1.cpp)
    target_link_libraries(test1 ${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_THREAD_LIBS_INIT)
