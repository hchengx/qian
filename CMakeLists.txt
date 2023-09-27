cmake_minimum_required(VERSION 3.10)
project(qian)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -rdynamic -g -Wno-unused-function -Wno-deprecated -Werror -fPIC -O3 -ggdb")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIB_SRC
    src/log.cc
    )

add_library(qian SHARED ${LIB_SRC})

target_include_directories(qian PUBLIC ${PROJECT_SOURCE_DIR}/include)

