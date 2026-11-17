# 第 4 章 构建、测试与工具链

> 这一章的问题来自「代码能跑之后还要什么」。答案：**可重复构建、自动检查、可安装、可测性能**——这些才是「工程」和「作业」的区别。

## Q4.1 CMake 到底在做什么

**疑问**：`cmake -S . -B build -G Ninja` 然后 `cmake --build build`，两步分别在干嘛？

**解答**：

| 阶段 | 命令 | 输入 | 输出 |
|---|---|---|---|
| 配置（configure） | `cmake -S . -B build -G Ninja` | `CMakeLists.txt` + 工具链 | `build/build.ninja`、`CMakeCache.txt`、`compile_commands.json` |
| 构建（build） | `cmake --build build` | `build.ninja` | 可执行文件 / 静态库 |

CMake **不编译**，它是「构建系统生成器」：把跨平台描述翻译成 Ninja/Makefile/VS 工程；真正编译的是 Ninja 调用的 `g++`。

为什么用 CMake 而不是手写 g++ 命令：依赖追踪、增量编译、多 target、安装/导出、IDE 集成、CI 一致性。

**一句话**：CMake 生成构建文件，Ninja 执行编译；改 CMakeLists 才会重新 configure。

**项目里**：`projects/p1-jq-lite/CMakeLists.txt`。

## Q4.2 严格编译选项的清单

**疑问**：为什么 `-Werror` 让人烦，还要开？

**解答**：这个项目用的告警集合：

```cmake
-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
-Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused
-Woverloaded-virtual -Wdouble-promotion
```

其中几个是「C++ 工程必修」：

| 选项 | 抓什么 | 我踩过的例子 |
|---|---|---|
| `-Wconversion` | 隐式数值截断 | `int` → `size_t`（append 长度参数） |
| `-Wsign-conversion` | 有符号/无符号混用 | `value * 10 + (text[i] - '0')` |
| `-Wshadow` | 内层变量遮蔽外层 | fuzz 里内层 `value` 遮蔽外层 |
| `-Wold-style-cast` | C 风格转换 | `(unsigned char)c` 要改成 `static_cast` |
| `-Wunused` | 定义了没用的函数/变量 | 实现完后忘了删 `todo()` 占位函数 |

`-Werror` 把告警变错误：**告警必须当场修，不能欠债**。

**一句话**：`-Werror` 让「以后再说」变成「现在就说」。

**项目里**：`CMakeLists.txt` 的 `WARNINGS` 变量；`treat-warnings-as-errors` 只加在库 target 上（第三方头文件不开）。

## Q4.3 Sanitizer：ASan / UBSan / TSan

**疑问**：ASan 是什么？为什么 Debug 构建要开它？

**解答**：

| 工具 | 抓什么 | 开销 |
|---|---|---|
| ASan（AddressSanitizer） | 越界读写、use-after-free、double free、栈溢出 | 2–3× 时间，2–3× 内存 |
| UBSan（UndefinedBehaviorSanitizer） | 有符号溢出、空指针解引用、错位访问 | 较小 |
| TSan（ThreadSanitizer） | 数据竞争 | 5–15× |

CMake 里的写法：

```cmake
option(ENABLE_SANITIZERS "..." ON)
if(ENABLE_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()
```

ASan 的报错信息会给出「哪一行、哪个变量、栈上还是堆上」，这是它值钱的地方。它抓不到所有问题（比如 `string_view` 越界一字节可能落在同一对象内），所以还要靠 fuzz 的逻辑断言。

**一句话**：Sanitizer 是「带人工摄像头的构建」，用 2× 时间换「内存错误当场暴露」。

**项目里**：`CMakeLists.txt` 的 `ENABLE_SANITIZERS`；CI 的 Debug job 强制开。

## Q4.4 GoogleTest 与 ctest 的关系

**疑问**：`./build/jq_tests` 和 `ctest` 有什么区别？

**解答**：

- **GoogleTest** 是测试框架：`TEST(Suite, Name)` 定义用例，自己打印 `[ OK ]`/`[ FAILED ]`；
- **ctest** 是 CMake 的测试驱动器：跑 `add_test()` 注册的**任意程序**（可以是 gtest 二进制，也可以是脚本）。

```cmake
include(GoogleTest)
gtest_discover_tests(jq_tests)          # 把每个 TEST 注册成独立的 ctest 用例
add_test(NAME FuzzParser COMMAND jq_fuzz 5000)
```

好处：一个 `ctest` 命令跑所有类型的测试（单测、fuzz 冒烟、JSONTestSuite），输出统一、失败可见。

常用参数：`ctest --test-dir build --output-on-failure`（只打印失败）、`-R <正则>`（过滤）、`-j4`（并行）。

**一句话**：gtest 定义测试，ctest 统一执行；`add_test` 能把任何可执行文件变成测试。

**项目里**：`CMakeLists.txt` 的 `gtest_discover_tests` 和 `add_test(FuzzParser ...)`；`ctest` 68 个目标。

## Q4.5 `install()` 与 `find_package`：把项目变成「库」

**疑问**：为什么要写 `install(TARGETS ... EXPORT ...)`？

**解答**：让别人的项目一行接入：

```cmake
# 安装侧
include(GNUInstallDirs)
install(TARGETS jq_lib EXPORT jq-lite-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT jq-lite-targets NAMESPACE jq:: FILE jq-lite-targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jq-lite)

# 使用侧
find_package(jq-lite 0.1 CONFIG REQUIRED)
target_link_libraries(consumer PRIVATE jq::jq_lib)
```

两个坑（都踩过）：

1. **Debug + ASan 的静态库不能给别人链接**：机器码里带 `__asan_*` 调用，消费者链接报 undefined reference。所以 `install` 必须来自 `Release -DENABLE_SANITIZERS=OFF` 的构建目录；
2. **`install()` 不能用 `BUILD_INTERFACE`**：`$<BUILD_INTERFACE:...>` 只在构建树有效，安装后消费者就找不到；`$<INSTALL_INTERFACE:...>` 才是给安装树的。

**一句话**：`install/export` 把「我的代码」变成「别人 `find_package` 能用的包」。

**项目里**：`CMakeLists.txt` 末尾、`cmake/jq-lite-config.cmake.in`、`examples/consumer/`。

## Q4.6 GitHub Actions 的两个 job

**疑问**：CI 为什么跑两套构建？

**解答**：

```yaml
strategy:
  matrix:
    include:
      - name: Debug + ASan/UBSan      # 抓内存错误、UB
        build_type: Debug
        sanitizers: ON
      - name: Release                 # 抓优化后行为、NDEBUG 下编译问题
        build_type: Release
        sanitizers: OFF
```

两套分别抓不同问题：

- Debug + ASan：内存错误、未定义行为、断言；
- Release：`NDEBUG` 去掉 `assert` 后的逻辑问题、优化暴露的 UB、以及性能相关代码在 `-O3` 下才出现的告警。

CI 的意义：每次 push 在**干净机器**上从零构建，防止「我本地能跑」。

**一句话**：CI 是「别人替你在新电脑上装一遍」的自动化版本。

**项目里**：`.github/workflows/ci.yml`；push 后在 Actions 页面看结果。

## Q4.7 fuzz 怎么驱动

**疑问**：没有 libFuzzer 也能 fuzz 吗？

**解答**：能，本项目就是手写的驱动：

```cpp
int main(int argc, char** argv) {
    const int iterations = argc > 1 ? std::atoi(argv[1]) : 20000;
    const unsigned seed = argc > 2 ? std::strtoul(argv[2], nullptr, 10) : 0xC0FFEE;
    std::mt19937 rng(seed);

    int ok = 0, rejected = 0;
    for (int i = 0; i < iterations; ++i) {
        const std::string doc = random_json(rng);          // 合法文档
        const jq::Value v = jq::parse(doc);                // 必须成功
        if (jq::parse(v.dump()) != v) { /* 往返失败 */ }
        ++ok;

        std::string broken = doc;
        mutate(broken, rng);                               // 随机破坏
        try {
            jq::parse(broken);
        } catch (const jq::ParseError&) {
            ++rejected;                                    // 预期
        } catch (const std::exception& e) {
            /* 其他异常 = bug */
        }
    }
}
```

固定 seed 是为了**失败可复现**；真实工程会换成 libFuzzer（编译期加 `-fsanitize=fuzzer`，用覆盖率引导变异）。

**一句话**：fuzz = 随机输入 + 明确断言；固定种子让崩溃可复现。

**项目里**：`tests/fuzz_parser.cpp`；`jq_fuzz 20000 12648430`。

## Q4.8 Google Benchmark 怎么读结果

**疑问**：`--benchmark_repetitions=10` 输出的 mean/median/stddev 怎么用？

**解答**：

- `Time`：单次迭代时间；
- `bytes_per_second`：需要代码里 `state.SetBytesProcessed()`；
- `_mean` / `_median` / `_stddev` / `_cv`：重复多轮后的统计；
- **看 median**（中位数）：机器噪声大时比 mean 稳；
- **看 cv（变异系数）**：>5% 说明测量不稳定，结论要谨慎。

系统 `libbenchmark` 如果是 Debug 构建，日志会打印 `***WARNING*** Library was built as DEBUG`，绝对数字偏高，但同一会话内的**相对对比**仍然有效。

**一句话**：报 median 和 cv，别只报一个漂亮的 mean。

**项目里**：`benchmark/bench_parser.cpp`；README 性能表注明构建类型与中位数。

## Q4.9 调试工具：gdb / ASan / strace

**疑问**：程序崩了怎么查？

**解答**：按「症状 → 工具」选：

| 症状 | 工具 | 命令 |
|---|---|---|
| 段错误/越界 | ASan 构建 | `cmake -DENABLE_SANITIZERS=ON` 后直接跑 |
| 崩溃现场 | gdb | `gdb -ex run -ex bt --args ./build/jq_lite` |
| 系统调用失败 | strace | `strace -f -e trace=network ./build/echo_server` |
| 卡死/等什么 | gdb attach / `/proc/<pid>/stack` | `gdb -p <pid>` 后 `thread apply all bt` |
| 内存泄漏 | valgrind / ASan LeakSanitizer | `ASAN_OPTIONS=detect_leaks=1` |
| 性能热点 | perf / gprof | `perf record -g ./build/jq_bench` |

**一句话**：先让 ASan 把错误变成「带行号的报告」，再决定要不要 gdb。

**项目里**：README「调试」一节；本次会话用 ASan 抓到过 `stack-overflow`（深度守卫前）和 `stack-buffer-overflow`（删边界检查实验）。

## Q4.10 本章检查表

- [ ] 能解释 configure 与 build 两阶段
- [ ] 能说出 `-Wconversion`/`-Wsign-conversion`/`-Wshadow` 各抓什么
- [ ] 能说明 ASan 与 UBSan 的分工
- [ ] 能写出 `install(TARGETS ... EXPORT ...)` 的最小骨架
- [ ] 能解释 CI 为什么跑 Debug+ASan 和 Release 两套
- [ ] 能说明 fuzz 的两条断言和固定 seed 的作用
- [ ] 能读懂 benchmark 的 mean/median/stddev/cv
