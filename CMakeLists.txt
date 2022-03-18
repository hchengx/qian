cmake_minimum_required(VERSION 3.10)
project(qian)
set(CMAKE_EXPORTS_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(qian)

add_subdirectory(tests)