cmake_minimum_required(VERSION 3.25)
project(p2-webserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_SANITIZERS "Build with AddressSanitizer + UBSan" ON)

if (ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif ()

set(P2_WARNINGS
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
    -Woverloaded-virtual -Wdouble-promotion
)

find_package(Threads REQUIRED)

# 网络库：第 1 周只有 socket / epoll / echo server
add_library(p2_net STATIC
    src/socket.cpp
    src/epoll.cpp
    src/echo_server.cpp
)
target_include_directories(p2_net PUBLIC include)
target_compile_options(p2_net PRIVATE ${P2_WARNINGS} -Werror)
target_link_libraries(p2_net PUBLIC Threads::Threads)

# 手动测试用的 echo 服务器
add_executable(p2_echo src/main.cpp)
target_link_libraries(p2_echo PRIVATE p2_net)
target_compile_options(p2_echo PRIVATE ${P2_WARNINGS} -Werror)

# 测试
enable_testing()
include(GoogleTest)
find_package(GTest REQUIRED)

foreach (t IN ITEMS test_socket test_epoll test_echo_server)
    add_executable(${t} tests/${t}.cpp)
    target_link_libraries(${t} PRIVATE p2_net GTest::gtest_main)
    target_compile_options(${t} PRIVATE ${P2_WARNINGS})
    gtest_discover_tests(${t})
endforeach ()
