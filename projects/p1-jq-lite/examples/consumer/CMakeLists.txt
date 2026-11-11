cmake_minimum_required(VERSION 3.25)
project(jq_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 关键：这行证明 jq-lite 真的能被一个「外部项目」当成普通依赖使用。
find_package(jq-lite 0.1 CONFIG REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE jq::jq_lib)
