cmake_minimum_required(VERSION 3.10)

project(PokerBot VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main main.cpp utils.cpp hash.cpp)
target_compile_options(main PRIVATE -std=c++20 -O2) 
