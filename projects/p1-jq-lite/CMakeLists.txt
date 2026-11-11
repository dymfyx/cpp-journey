cmake_minimum_required(VERSION 3.25)
project(jq_lite VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

option(ENABLE_SANITIZERS "Build with AddressSanitizer + UBSan" ON)

set(WARNINGS
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
    -Woverloaded-virtual -Wdouble-promotion)
set(SANITIZER_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer -g)

add_library(jq_lib STATIC
    src/parser.cpp
    src/value.cpp
    src/path.cpp)
target_include_directories(jq_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(jq_lib PRIVATE ${WARNINGS} -Werror)
if(ENABLE_SANITIZERS)
    # BUILD_INTERFACE：sanitizer 只在「本仓库自己构建」时生效，
    # 不允许泄漏到 install 出去的目标里（否则消费者的每次构建都被强制开 ASan）。
    target_compile_options(jq_lib PUBLIC $<BUILD_INTERFACE:${SANITIZER_FLAGS}>)
    target_link_options(jq_lib PUBLIC $<BUILD_INTERFACE:${SANITIZER_FLAGS}>)
endif()

enable_testing()
find_package(GTest REQUIRED)

add_executable(jq_tests tests/test_parser.cpp)
target_link_libraries(jq_tests PRIVATE jq_lib GTest::gtest_main)
target_compile_options(jq_tests PRIVATE ${WARNINGS})

add_executable(jq_dump_tests tests/test_dump.cpp)
target_link_libraries(jq_dump_tests PRIVATE jq_lib GTest::gtest_main)
target_compile_options(jq_dump_tests PRIVATE ${WARNINGS})

add_executable(jq_path_tests tests/test_path.cpp)
target_link_libraries(jq_path_tests PRIVATE jq_lib GTest::gtest_main)
target_compile_options(jq_path_tests PRIVATE ${WARNINGS})

add_executable(jq_lite src/main.cpp)
target_link_libraries(jq_lite PRIVATE jq_lib)
target_compile_options(jq_lite PRIVATE ${WARNINGS})

include(GoogleTest)
gtest_discover_tests(jq_tests)
gtest_discover_tests(jq_dump_tests)
gtest_discover_tests(jq_path_tests)

# ---------------------------------------------------------------------------
# 第 4 周：fuzz 驱动 + benchmark
# ---------------------------------------------------------------------------

add_executable(jq_fuzz tests/fuzz_parser.cpp)
target_link_libraries(jq_fuzz PRIVATE jq_lib)
target_compile_options(jq_fuzz PRIVATE ${WARNINGS} -Werror)
add_test(NAME FuzzParser COMMAND jq_fuzz 5000)

option(ENABLE_BENCHMARK "Build Google Benchmark targets" ON)
if(ENABLE_BENCHMARK)
    find_package(benchmark QUIET)
    if(benchmark_FOUND)
        add_executable(jq_bench benchmark/bench_parser.cpp)
        target_link_libraries(jq_bench PRIVATE jq_lib benchmark::benchmark)
        target_compile_options(jq_bench PRIVATE ${WARNINGS})

        # 第 5 周：和 nlohmann/json 对拍（同一个 doc_gen.hpp 生成输入）
        find_package(nlohmann_json QUIET)
        if(nlohmann_json_FOUND)
            add_executable(bench_nlohmann benchmark/bench_nlohmann.cpp)
            target_link_libraries(bench_nlohmann PRIVATE nlohmann_json::nlohmann_json benchmark::benchmark)
            # 第三方头文件不套 -Werror 警告集，否则报的全是别人的代码
            target_compile_options(bench_nlohmann PRIVATE -O2)
        else()
            message(STATUS "nlohmann_json not found, bench_nlohmann disabled (sudo apt install nlohmann-json3-dev)")
        endif()
    else()
        message(STATUS "Google Benchmark not found, jq_bench disabled (sudo apt install libbenchmark-dev)")
    endif()
endif()

# ---------------------------------------------------------------------------
# 第 5 周：你手写的练习（手写 benchmark / 手写最小 fuzz）
#   这两个文件现在是空的骨架，填完才算完成第 5 周。
# ---------------------------------------------------------------------------

add_executable(min_bench benchmark/min_bench.cpp)
target_link_libraries(min_bench PRIVATE jq_lib)
target_compile_options(min_bench PRIVATE ${WARNINGS} -Werror)

add_executable(min_fuzz tests/min_fuzz.cpp)
target_link_libraries(min_fuzz PRIVATE jq_lib)
target_compile_options(min_fuzz PRIVATE ${WARNINGS} -Werror)

# ---------------------------------------------------------------------------
# 第 5 周：install / export，让别的项目能 find_package(jq-lite)
#
# 注意：install 出来的 .a 不能带 sanitizer 插桩，否则消费者链接时报
#   undefined reference to `__asan_option_detect_stack_use_after_return`
# 所以验证 install/export 时要用 Release + ENABLE_SANITIZERS=OFF 的构建目录：
#   cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
#   cmake --build build-rel
#   cmake --install build-rel --prefix /tmp/jq-stage
# ---------------------------------------------------------------------------

if(ENABLE_SANITIZERS)
    message(STATUS "ENABLE_SANITIZERS=ON: 这份构建目录里的 jq_lib 带 ASan/UBSan 插桩，"
                   "install 出去会让消费者链接失败。验证 install/export 请用 Release + "
                   "-DENABLE_SANITIZERS=OFF 的构建目录。")
endif()

install(TARGETS jq_lib
    EXPORT jq-lite-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT jq-lite-targets
    NAMESPACE jq::
    FILE jq-lite-targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jq-lite)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/jq-lite-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/jq-lite-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jq-lite)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/jq-lite-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/jq-lite-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/jq-lite-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jq-lite)
