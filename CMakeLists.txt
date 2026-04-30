cmake_minimum_required(VERSION 3.16)
project(orderbook CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strict warnings — your code should compile clean
add_compile_options(-Wall -Wextra -Wpedantic -O2)

add_executable(orderbook
    src/main.cpp
    src/OrderBook.cpp
    src/PriceLevel.cpp
)

target_include_directories(orderbook PRIVATE include)

# ── Google Benchmark ────────────────────────────────────────────────────────
find_package(benchmark REQUIRED)

add_executable(bench_orderbook
    bench/bench_orderbook.cpp
    src/OrderBook.cpp
    src/PriceLevel.cpp
)

target_include_directories(bench_orderbook PRIVATE include)
target_link_libraries(bench_orderbook PRIVATE benchmark::benchmark)

# ── clang-tidy lint target ───────────────────────────────────────────────────
find_program(CLANG_TIDY clang-tidy HINTS /opt/homebrew/opt/llvm/bin)
if(CLANG_TIDY)
    set(LINT_SOURCES
        ${CMAKE_SOURCE_DIR}/src/OrderBook.cpp
        ${CMAKE_SOURCE_DIR}/src/PriceLevel.cpp
        ${CMAKE_SOURCE_DIR}/src/main.cpp
    )
    add_custom_target(lint
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR}
            --extra-arg=--sysroot=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
            ${LINT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy"
    )
endif()
