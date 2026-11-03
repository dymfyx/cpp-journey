cmake_minimum_required(VERSION 3.25)
project(p0_02_buffer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 给 clangd 用

option(ENABLE_SANITIZERS "Build with AddressSanitizer + UBSan" ON)

set(WARNINGS
    -Wall -Wextra -Wpedantic -Wshadow
    -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
    -Wunused -Woverloaded-virtual -Wdouble-promotion)

if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(buffer_lib
    src/buffer.cpp)
target_include_directories(buffer_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(buffer_lib PRIVATE ${WARNINGS} -Werror)

enable_testing()
find_package(GTest REQUIRED)

add_executable(buffer_tests tests/test_buffer.cpp)
target_link_libraries(buffer_tests PRIVATE buffer_lib GTest::gtest_main)
target_compile_options(buffer_tests PRIVATE ${WARNINGS})

include(GoogleTest)
gtest_discover_tests(buffer_tests)
