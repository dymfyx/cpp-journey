// 第 5 周：和 nlohmann/json 对拍。
//
// 目的：同一个 1MB 文档、同一台机器、同一套编译选项，把
//   jq-lite 的 parse / dump 和 nlohmann::json 的 parse / dump 放在一起比。
//
// 跑法（必须用 Release 构建，否则数字没有意义）：
//   cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
//   cmake --build build-rel
//   ./build-rel/jq_bench --benchmark_filter='Parse/Large|Dump'      # jq-lite
//   ./build-rel/bench_nlohmann --benchmark_filter='Parse/Large|Dump' # nlohmann
//
// 说明：
//   - 输入文档由 doc_gen.hpp 生成，两边完全一致；
//   - nlohmann 默认 dump() 是紧凑模式，dump(2) 是缩进模式；
//   - 两个库的 DOM 设计不同（nlohmann 用 union + 有序 map，jq-lite 用 std::variant），
//     所以「谁快谁慢」的结论要分项看：解析、紧凑序列化、缩进序列化分别比。

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

#include "doc_gen.hpp"

namespace {

using json = nlohmann::json;

void report_bytes(benchmark::State& state, std::size_t bytes) {
    state.SetBytesProcessed(static_cast<std::int64_t>(state.iterations()) *
                            static_cast<std::int64_t>(bytes));
}

void benchmark_parse(benchmark::State& state, const std::string& text) {
    for (auto _ : state) {
        const json value = json::parse(text);
        benchmark::DoNotOptimize(value.size());
    }
    report_bytes(state, text.size());
}

const json& value_large() {
    static const json value = json::parse(bench_docs::large());
    return value;
}

}  // namespace

BENCHMARK([](benchmark::State& state) { benchmark_parse(state, bench_docs::tiny()); })
    ->Name("Nlohmann/Parse/Tiny");

BENCHMARK([](benchmark::State& state) { benchmark_parse(state, bench_docs::large()); })
    ->Name("Nlohmann/Parse/Large");

BENCHMARK([](benchmark::State& state) {
    const json& value = value_large();
    for (auto _ : state) {
        const std::string out = value.dump();
        benchmark::DoNotOptimize(out.size());
    }
    report_bytes(state, bench_docs::large().size());
})->Name("Nlohmann/Dump/Compact");

BENCHMARK([](benchmark::State& state) {
    const json& value = value_large();
    for (auto _ : state) {
        const std::string out = value.dump(2);
        benchmark::DoNotOptimize(out.size());
    }
    report_bytes(state, bench_docs::large().size());
})->Name("Nlohmann/Dump/Pretty");

BENCHMARK_MAIN();
