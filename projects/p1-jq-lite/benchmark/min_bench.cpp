// 第 5 周练习 1：手写 benchmark（不许用 Google Benchmark）
//
// 目标：测出 jq::parse 的解析吞吐，打印 `Parse: 123.4 MiB/s`。
//
// 提示（先自己写，卡住再看）：
//   1. 自己造一份约 1MB 的 JSON 文本（思路参考 benchmark/bench_parser.cpp 的 make_doc；
//      用 std::string 拼，别用 to_string 拼巨量字符串——可以 reserve 一下）。
//   2. 用 std::chrono::steady_clock 记录「连续 parse N 次」的墙钟时间：
//          const auto begin = std::chrono::steady_clock::now();
//          for (int i = 0; i < N; ++i) { volatile auto v = jq::parse(text); }
//          const auto end = std::chrono::steady_clock::now();
//      volatile 只是防止编译器把结果整个优化掉；真实项目用 DoNotOptimize。
//   3. 吞吐 = (N × 文档字节数) / 秒数 / (1024 × 1024)，单位 MiB/s。
//   4. 先 warmup 几轮再计时（第一轮可能冷缓存，数字偏低）。
//
// 验收：./build/min_bench 打印的数字和 ./build-rel/jq_bench 的 Parse/Large（约 192 MiB/s）
//       同一个数量级。差一个数量级就说明计时或字节数算错了。

#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

#include <jq/parser.hpp>

int main() {
    // TODO(练习 1)：在这里实现
    std::cout << "TODO: 手写 benchmark 还没写\n";
    return 0;
}
