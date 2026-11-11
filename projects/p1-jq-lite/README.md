# jq-lite

用现代 C++20 从零实现的 JSON 库 + 命令行查询工具，无第三方依赖（测试用 GoogleTest，基准用 Google Benchmark）。

- **42 个测试全绿**（13 解析 + 12 序列化 + 16 路径查询 + 1 fuzz 驱动）
- fuzz：100,000 次随机/破坏输入无崩溃（ASan + UBSan 全程开启）
- 性能：1 MB 文档解析约 **192 MiB/s**、序列化约 **205 MiB/s**（Release，g++ 13.3.0）
- GitHub Actions CI：`Debug + ASan/UBSan` 与 `Release` 双配置

## 特性

- RFC 8259 递归下降解析器，错误信息带 1-based 行列号
- DOM 用 `std::variant` 实现（`null/bool/number/string/array/object`）
- 序列化：紧凑 / 缩进两种模式，`%.17g` 保证 double 往返无损
- 路径查询：`.a.b[0]`、`.users[1].name`（jq 子集），编译成段序列后解释执行
- CLI：读文件或 stdin，支持 `-c` 紧凑输出、`-r` 原始字符串
- fuzz 驱动 + Google Benchmark + CI

## 目录

```
p1-jq-lite/
├── CMakeLists.txt
├── include/jq/
│   ├── value.hpp        # DOM 类型（std::variant）与 dump 接口
│   ├── error.hpp        # ParseError（带行列号）
│   ├── parser.hpp       # parse(std::string_view) -> Value
│   └── path.hpp         # PathSegment / parse_path / evaluate_path
├── src/
│   ├── parser.cpp       # 递归下降解析
│   ├── value.cpp        # 构造函数、is_*/as_*、operator==、dump
│   ├── path.cpp         # 路径解析与求值
│   └── main.cpp         # CLI 入口
├── tests/
│   ├── test_parser.cpp  # 13 个解析测试
│   ├── test_dump.cpp    # 12 个序列化测试
│   ├── test_path.cpp    # 16 个路径测试
│   └── fuzz_parser.cpp  # 随机输入驱动（独立可执行）
├── benchmark/bench_parser.cpp  # Google Benchmark
├── examples/consumer/   # find_package 使用示例（第 5 周）
└── cmake/               # package config（第 5 周）
```

## 快速开始

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure     # 42/42
```

CLI：

```bash
echo '{"a":{"b":[1,2,{"c":"d"}]}}' | ./build/jq_lite '.a.b[2].c'   # "d"
echo '{"a":1}' | ./build/jq_lite -c '.'                            # {"a":1}
./build/jq_lite -r '.name' config.json                             # 不带引号的字符串
```

库：

```cpp
#include <jq/parser.hpp>
#include <iostream>

int main() {
    const jq::Value doc = jq::parse(R"({"users":[{"name":"a"},{"name":"b"}]})");
    std::cout << jq::evaluate_path(doc, ".users[1].name").as_string() << '\n';  // b
    std::cout << doc.dump(2) << '\n';
}
```

## 性能

| 基准 | 延迟（中位数） | 吞吐 |
| --- | --- | --- |
| Parse/Tiny（80 B） | 204 ns | ~308 MiB/s |
| Parse/Large（1 MB） | 8.63 ms | ~114 MiB/s |
| Dump/Compact（1 MB） | 7.50 ms | ~131 MiB/s |
| Dump/Pretty（1 MB） | 9.0 ms | ~109 MiB/s |
| EvaluatePath/Large | 85 ns | — |

测于 Ubuntu 24.04.5 / g++ 13.3.0 / `-O3 -DNDEBUG` / `ENABLE_SANITIZERS=OFF` / 1 MB 文档 / 单线程，
每项取 5 次重复的中位数。笔记本 + WSL2 的噪声约 ±10%，换机器会不同。

Debug + ASan 构建的数字比 Release 慢 60–100 倍，**不要**拿来做性能结论；CI 只跑测试，不做性能断言。

### 与 nlohmann/json 对比

同一个 1 MB 文档、同一套编译选项、同一个 Google Benchmark（`jq_bench` vs `bench_nlohmann`，
各取 5 次中位数）：

| 项目 | jq-lite | nlohmann::json | 比值 |
| --- | --- | --- | --- |
| 解析 80 B | 204 ns | 608 ns | jq-lite 快 3.0× |
| 解析 1 MB | 8.63 ms | 18.24 ms | jq-lite 快 2.1× |
| 紧凑序列化 1 MB | 7.50 ms | 4.63 ms | nlohmann 快 1.6× |
| 缩进序列化 1 MB | 9.0 ms | 6.8 ms | nlohmann 快 1.3× |

为什么：

- **解析赢在实现简单**：递归下降直接构造 `std::variant` DOM，没有 SAX 中间层，也没有完整的
  UTF-8 校验（nlohmann 两者都有）；
- **序列化输在数字格式化**：每个 `double` 都走 `snprintf("%.17g")`，这是整条 dump 路径上最贵的
  操作；nlohmann 用自己的 float→text 算法（Grisu/Dragonbox 一类）。
- 下一步优化方向已明确：把 `snprintf` 换成 `std::to_chars`（C++17 的 `double` 重载），
  预计紧凑序列化能追回大半差距——放在第 7 周做。
- 需要说清楚的取舍：nlohmann/json 功能远多于 jq-lite（UTF-8 校验、SAX 接口、定制序列化、
  CBOR/MessagePack），这个对比只说明「核心路径的性能」，不是「谁更好」。

```bash
cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
cmake --build build-rel
./build-rel/jq_bench            # jq-lite
./build-rel/bench_nlohmann      # nlohmann/json（需要 sudo apt install nlohmann-json3-dev）
```

## 设计取舍

**1. `std::map<std::string, Value>` 而不是 `std::unordered_map`**
dump 的输出必须稳定可复现（相同语义的文档序列化后逐字节一致），`map` 的有序性让这一点天然成立；`Value` 的 `operator==` 也会退化成按序递归比较。代价是单次查 key 从期望 O(1) 变成 O(log n)，小对象（几十个 key）上差异可以忽略。

**2. 数字用 `%.17g` 而不是 `%.6g` 或 `std::to_string`**
`double` 转十进制再转回来必须逐位相等，17 位有效数字是 IEEE-754 double 的往返上界。`std::to_string` 只给 6 位小数，`1e300` 这类值会直接丢精度；`%.6g` 无法保证 roundtrip。

**3. 输入用 `std::string_view`（零拷贝），但 key 和字符串值仍然拷贝**
解析器只读输入、不拥有它，`string_view` 让调用方可以传 `std::string`/字面量/`char*` 而不产生拷贝；但 `Value` 的字符串必须拥有内存（`std::string`），否则源缓冲区释放后 DOM 里就是悬空引用。这就是「值语义 DOM」的代价：字符串内容拷贝一次，换来的是生命周期自洽。

**4. `std::variant` 而不是继承 + 虚函数**
JSON 的六种类型是封闭集合，用 `variant` 可以在栈上存放、按值拷贝/比较、不需要 `new`/`delete`。代价是 `Value` 对象较大（约 50–60 字节，取决于 `std::string`/`std::vector` 的实现）且不能扩展新的 JSON 类型——但对 JSON 来说这不是需求。

**5. 递归下降 + 显式行列号**
递归下降的代码结构和 JSON 文法一一对应，最容易写对、最容易报出准确的错误位置；手写状态机更快但难维护，用 yacc/ANTLR 则引入生成器和依赖。性能上解析约 114 MiB/s、序列化约 131 MiB/s，够用；正确性和可读性优先。

## 测试

```bash
./build/jq_tests --gtest_filter='ParserTest.ParsesNumbers'   # 单测
./build/jq_fuzz 20000 12648430                               # fuzz：迭代数 + 种子（12648430 = 0xC0FFEE）
ctest --test-dir build --output-on-failure                   # 全部（含 fuzz 5000 次）
```

fuzz 做两件事：随机生成合法 JSON 检查 `parse(dump(v)) == v`；随机破坏后只允许抛 `ParseError`。失败会打印输入、dump、错误信息，并立即退出——配合固定种子可完整复现。

## 10 周里程碑

| 周 | 内容 | 状态 |
| --- | --- | --- |
| 1 | 递归下降解析器（值/数组/对象/字符串/数字）+ 错误行列号 | ✅ 13/13 |
| 2 | 克隆实现序列化 `dump()`（转义、缩进、`%.17g`） | ✅ 25/25 |
| 3 | CLI + 路径查询 `.a.b[0]`（编译成段序列后解释执行） | ✅ 41/41 |
| 4 | fuzz 驱动 + Google Benchmark + GitHub Actions CI | ✅ 42/42 |
| 5 | `install()`/`install(EXPORT)` 导出、`find_package(jq-lite)`、手写最小 benchmark 与 fuzz | install/export ✅、nlohmann 对拍 ✅、练习待写 |
| 6 | JSONTestSuite（`y_` 全过、`n_` 全拒）+ `docs/design.md` | 待做 |
| 7–8 | 扩展任选：NDJSON 流式解析 / JSON Pointer / 递归深度限制与栈溢出防护 | 待做 |

## 参考

- RFC 8259（JSON 规范）
- `nlohmann/json`（参考它的 DOM 与错误信息设计，未抄实现）
- JSONTestSuite（github.com/nst/JSONTestSuite）
- GoogleTest / Google Benchmark
