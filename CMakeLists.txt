cmake_minimum_required(VERSION 3.16)
project(orderbook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strict warnings — your code should compile clean
add_compile_options(-Wall -Wextra -Wpedantic -O2)

add_executable(orderbook
    src/main.cpp
    src/OrderBook.cpp
)

target_include_directories(orderbook PRIVATE include)
