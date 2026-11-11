// 第 4 周：benchmark。
//
// 用 Google Benchmark 量三件事：
//   1. 解析吞吐（MB/s）——小文档（典型配置 ~80B）和大文档（~1MB）；
//   2. 序列化吞吐（MB/s）——紧凑模式和缩进模式；
//   3. 路径查询（次/秒）——在 1MB 文档上走 .records[1000].name。
//
// 跑法：
//   ./build/jq_bench                    # 全部跑一遍
//   ./build/jq_bench --benchmark_filter=Parse   # 只跑解析
//   ./build/jq_bench --benchmark_min_time=0.2   # 每项至少跑 0.2 秒（默认 1 秒）
//
// 注意：这个目标链接的是开启了 ASan/UBSan 的 jq_lib，数字会比 release 慢 2~3 倍。
// 想看真实性能，需要另配 Release 构建（第 5 周的事）。

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "jq/parser.hpp"
#include "jq/path.hpp"
#include "jq/value.hpp"

#include "doc_gen.hpp"

namespace {

// 生成逻辑见 doc_gen.hpp（与 bench_nlohmann 共用同一份字节）
using bench_docs::large;
using bench_docs::tiny;

const jq::Value& value_large() {
    static const jq::Value value = jq::parse(large());
    return value;
}

void report_bytes(benchmark::State& state, std::size_t bytes) {
    state.SetBytesProcessed(static_cast<std::int64_t>(state.iterations()) *
                            static_cast<std::int64_t>(bytes));
}

void benchmark_parse(benchmark::State& state, const std::string& text) {
    for (auto _ : state) {
        const jq::Value value = jq::parse(text);
        benchmark::DoNotOptimize(value.size());
    }
    report_bytes(state, text.size());
}

}  // namespace

// 解析：典型配置文件
BENCHMARK([](benchmark::State& state) { benchmark_parse(state, tiny()); })->Name("Parse/Tiny");

// 解析：1MB 文档
BENCHMARK([](benchmark::State& state) { benchmark_parse(state, large()); })->Name("Parse/Large");

// 序列化：紧凑模式
BENCHMARK([](benchmark::State& state) {
    const jq::Value& value = value_large();
    for (auto _ : state) {
        const std::string out = value.dump(-1);
        benchmark::DoNotOptimize(out.size());
    }
    report_bytes(state, large().size());
})->Name("Dump/Compact");

// 序列化：缩进模式
BENCHMARK([](benchmark::State& state) {
    const jq::Value& value = value_large();
    for (auto _ : state) {
        const std::string out = value.dump(2);
        benchmark::DoNotOptimize(out.size());
    }
    report_bytes(state, large().size());
})->Name("Dump/Pretty");

// 路径查询：每次都要 parse_path + 走 2 步
BENCHMARK([](benchmark::State& state) {
    const jq::Value& value = value_large();
    for (auto _ : state) {
        const jq::Value result = jq::evaluate_path(value, ".records[1000].name");
        benchmark::DoNotOptimize(result.dump().size());
    }
})->Name("EvaluatePath/Large");

BENCHMARK_MAIN();
