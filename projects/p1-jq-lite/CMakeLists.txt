cmake_minimum_required(VERSION 3.25)
project(jq_lite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Build with AddressSanitizer + UBSan" ON)

set(WARNINGS
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
    -Woverloaded-virtual -Wdouble-promotion)
set(SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer -g)

add_library(jq_lib STATIC
    src/parser.cpp
    src/value.cpp)
target_include_directories(jq_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(jq_lib PRIVATE ${WARNINGS} -Werror)
if(ENABLE_SANITIZERS)
    target_compile_options(jq_lib PUBLIC ${SANITIZER_FLAGS})
    target_link_options(jq_lib PUBLIC ${SANITIZER_FLAGS})
endif()

enable_testing()
find_package(GTest REQUIRED)

add_executable(jq_tests tests/test_parser.cpp)
target_link_libraries(jq_tests PRIVATE jq_lib GTest::gtest_main)
target_compile_options(jq_tests PRIVATE ${WARNINGS})

add_executable(jq_dump_tests tests/test_dump.cpp)
target_link_libraries(jq_dump_tests PRIVATE jq_lib GTest::gtest_main)
target_compile_options(jq_dump_tests PRIVATE ${WARNINGS})

include(GoogleTest)
gtest_discover_tests(jq_tests)
gtest_discover_tests(jq_dump_tests)
