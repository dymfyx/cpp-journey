# jq-lite

用现代 C++20 从零实现的 JSON 库 + 命令行查询工具，核心库无第三方依赖（测试用 GoogleTest，基准用 Google Benchmark）。

- **43 个测试全绿**（13 解析 + 12 序列化 + 16 路径查询 + fuzz 驱动 + JSONTestSuite 驱动）
- **JSONTestSuite 官方测试集**：`y_` 95/95 通过、`n_` 188/188 拒绝
- fuzz：100,000 次随机/破坏输入无崩溃（ASan + UBSan 全程开启）
- 性能（同场对比 nlohmann/json）：解析快 **2.1×**，紧凑序列化 **246 vs 240 MiB/s**，缩进序列化 163 vs 173 MiB/s
- GitHub Actions CI：`Debug + ASan/UBSan` 与 `Release` 双配置
- 支持 `install()` / `find_package(jq-lite)`，可作为外部依赖使用

## 特性

- RFC 8259 递归下降解析器，错误信息带 1-based 行列号
- DOM 用 `std::variant` 实现（`null/bool/number/string/array/object`）
- 序列化：紧凑 / 缩进两种模式；数字用 `std::to_chars` 输出**最短可往返**表示
- 递归深度上限 512 层（`Depth_guard`），超限抛 `ParseError` 而不是栈溢出崩溃
- 路径查询：`.a.b[0]`、`.users[1].name`（jq 子集），编译成段序列后解释执行
- CLI：读文件或 stdin，支持 `-c` 紧凑输出、`-r` 原始字符串
- fuzz 驱动 + Google Benchmark + CI + 文档（`docs/design.md`）

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
│   ├── parser.cpp       # 递归下降解析 + 深度守卫
│   ├── value.cpp        # 构造函数、is_*/as_*、operator==、dump
│   ├── path.cpp         # 路径解析与求值
│   └── main.cpp         # CLI 入口
├── tests/
│   ├── test_parser.cpp      # 13 个解析测试
│   ├── test_dump.cpp        # 12 个序列化测试
│   ├── test_path.cpp        # 16 个路径测试
│   ├── fuzz_parser.cpp      # 随机输入驱动（独立可执行）
│   ├── jsontest_runner.cpp  # JSONTestSuite 驱动
│   ├── jsonsuite/           # 官方 test_parsing 用例（318 个）
│   └── min_fuzz.cpp         # 手写最小 fuzz（练习用）
├── benchmark/
│   ├── bench_parser.cpp     # Google Benchmark（本库）
│   ├── bench_nlohmann.cpp   # 同输入对拍 nlohmann/json
│   ├── doc_gen.hpp          # 两库共用的 1 MB / 80 B 文档生成器
│   └── min_bench.cpp        # 手写最小 benchmark（练习用）
├── docs/
│   ├── design.md            # 设计文档（DOM 布局/深度限制/数字格式/测试策略）
│   └── value-cpp-reference.cpp
├── examples/consumer/   # find_package 使用示例
└── cmake/               # package config（jq-lite-config.cmake.in）
```

## 快速开始

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure     # 43/43
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
#include <jq/path.hpp>
#include <iostream>

int main() {
    const jq::Value doc = jq::parse(R"({"users":[{"name":"a"},{"name":"b"}]})");
    std::cout << jq::evaluate_path(doc, ".users[1].name").as_string() << '\n';  // b
    std::cout << doc.dump(2) << '\n';
}
```

作为依赖：

```cmake
find_package(jq-lite 0.1 CONFIG REQUIRED)
target_link_libraries(app PRIVATE jq::jq_lib)
```

## 性能

同一台机器、同一会话、同一套编译选项，各取 **10 轮 median**：

| 基准 | jq-lite | nlohmann/json | 比值 |
| --- | --- | --- | --- |
| Parse/Tiny（80 B） | 190 ns（331 MiB/s） | 583 ns（108 MiB/s） | jq-lite 快 3.1× |
| Parse/Large（1 MB） | 7.36 ms（133.9 MiB/s） | 15.42 ms（63.9 MiB/s） | jq-lite 快 2.1× |
| Dump/Compact（1 MB） | 4.01 ms（245.8 MiB/s） | 4.10 ms（240.2 MiB/s） | 基本持平 |
| Dump/Pretty（1 MB） | 6.06 ms（162.5 MiB/s） | 5.68 ms（173.4 MiB/s） | nlohmann 快 1.07× |
| EvaluatePath/Large | 80.9 ns | — | — |

测于 Ubuntu 24.04.5 / g++ 13.3.0 / `-O3 -DNDEBUG` / `ENABLE_SANITIZERS=OFF` / 1 MB 文档 / 单线程。

**关于绝对值**：笔记本 + WSL2 的 CPU 频率（睿频/降频）会让同一份代码在不同时间差出 30–40%，所以上表所有数字都来自**同一次会话**，并且 nlohmann 用的是同一个 Google Benchmark 库、同一份输入（`benchmark/doc_gen.hpp`）。跨天比较绝对值没有意义，只能看同场相对值。系统安装的 `libbenchmark` 是 Debug 构建（日志会打印 WARNING），绝对延迟整体偏高，但两库同场比较仍然有效。

Debug + ASan 构建比 Release 慢 60–100 倍，**不要**拿来做性能结论；CI 只跑测试，不做性能断言。

```bash
cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
cmake --build build-rel
./build-rel/jq_bench --benchmark_repetitions=10            # jq-lite
./build-rel/bench_nlohmann --benchmark_repetitions=10     # nlohmann/json（需 apt install nlohmann-json3-dev）
```

### 为什么快 / 为什么慢

- **解析赢在直接**：递归下降直接构造 `std::variant` DOM，没有 SAX 中间层，也没有逐字符 UTF-8 校验（nlohmann 两者都有）。
- **序列化已追平**：数字从 `snprintf("%.17g")` 换成 `std::to_chars`（libstdc++ 内部是 Ryu 算法），Dump/Compact 从 162 → 246 MiB/s，与 nlohmann 的 Grisu2/Dragonbox 持平。
- **缩进模式还差一点**（1.07×）：缩进路径每个元素都要调 `append_indent`（换行 + `indent*depth` 个空格），这是下一步可以优化的点。
- **取舍声明**：nlohmann/json 功能远多于 jq-lite（UTF-8 校验、SAX 接口、定制序列化、CBOR/MessagePack），这个对比只说明「核心路径的性能」，不是「谁更好」。

## 设计取舍

**1. `std::map<std::string, Value>` 而不是 `std::unordered_map`**
dump 的输出必须稳定可复现（相同语义的文档序列化后逐字节一致），`map` 的有序性让这一点天然成立；`Value` 的 `operator==` 也会退化成按序递归比较。代价是单次查 key 从期望 O(1) 变成 O(log n)，小对象（几十个 key）上差异可以忽略。

**2. 数字用 `std::to_chars` 而不是 `snprintf`**
`to_chars` 在编译期确定类型、不解析格式串、不查 locale、不分配内存，输出「最短且能往返」的十进制表示（`0.1` 而不是 `0.10000000000000001`）。`snprintf` 要走 printf 的通用路径（格式串解析、locale、任意精度大数运算），是 dump 路径上最贵的一步。

**3. 输入用 `std::string_view`（零拷贝），但 key 和字符串值仍然拷贝**
解析器只读输入、不拥有它，`string_view` 让调用方可以传 `std::string`/字面量/`char*` 而不产生拷贝；但 `Value` 的字符串必须拥有内存（`std::string`），否则源缓冲区释放后 DOM 里就是悬空引用。这就是「值语义 DOM」的代价：字符串内容拷贝一次，换来的是生命周期自洽。

**4. `std::variant` 而不是继承 + 虚函数**
JSON 的六种类型是封闭集合，用 `variant` 可以在栈上存放、按值拷贝/比较、不需要 `new`/`delete`。代价是 `Value` 对象较大（实测 56 字节）且不能扩展新的 JSON 类型——但对 JSON 来说这不是需求。详见 `docs/design.md` 第 2 节。

**5. 递归下降 + 显式行列号 + 512 层深度上限**
递归下降的代码结构和 JSON 文法一一对应，最容易写对、最容易报出准确的错误位置。代价是调用栈随嵌套深度线性增长，所以加 `Depth_guard` 把上限锁在 512 层：超限抛 `ParseError`，调用方可以捕获并继续服务，而不是被栈溢出 abort。512 是工程经验值（真实数据 3–4 层、测试集有 500 层、8 MB 栈只占几百 KB），不是规范要求。

## 测试

```bash
./build/jq_tests --gtest_filter='ParserTest.ParsesNumbers'   # 单测
./build/jq_fuzz 20000 12648430                               # fuzz：迭代数 + 种子（12648430 = 0xC0FFEE）
ctest --test-dir build --output-on-failure                   # 全部（含 fuzz 5000 次与 JSONTestSuite）
```

fuzz 做两件事：随机生成合法 JSON 检查 `parse(dump(v)) == v`；随机破坏后只允许抛 `ParseError`。失败会打印输入、dump、错误信息，并立即退出——配合固定种子可完整复现。

### JSONTestSuite 官方测试集

`tests/jsonsuite/` 收录了 [nst/JSONTestSuite](https://github.com/nst/JSONTestSuite) 的 `test_parsing`（318 个文件）：

```bash
./build/jq_jsontestsuite                     # y_ 95/95、n_ 188/188、i_ 接受 24 / 拒绝 11
ctest --test-dir build -R JSONTestSuite      # 同一件事，走 ctest
```

`n_structure_100000_opening_arrays.json`（10 万层 `[`）这类输入会把递归下降的调用栈打爆，由解析器里的 `Depth_guard` 把嵌套上限锁在 512 层并抛 `ParseError`——这条规则写在 `tests/jsonsuite/README.md`。

## 10 周里程碑

| 周 | 内容 | 状态 |
| --- | --- | --- |
| 1 | 递归下降解析器（值/数组/对象/字符串/数字）+ 错误行列号 | ✅ 13/13 |
| 2 | 克隆实现序列化 `dump()`（转义、缩进、数字格式） | ✅ 25/25 |
| 3 | CLI + 路径查询 `.a.b[0]`（编译成段序列后解释执行） | ✅ 41/41 |
| 4 | fuzz 驱动 + Google Benchmark + GitHub Actions CI | ✅ 42/42 |
| 5 | `install()`/`install(EXPORT)` 导出、`find_package`、与 nlohmann 对拍、手写最小 benchmark/fuzz | ✅ |
| 6 | JSONTestSuite 接入（y_ 95/95、n_ 188/188）、512 层深度限制、`docs/design.md` | ✅ 43/43 |
| 7 | `std::to_chars` 替换 `snprintf`（Dump/Compact 162 → 246 MiB/s，追平 nlohmann） | ✅ |
| 8 | 扩展任选：NDJSON 流式解析 / JSON Pointer / 深度上限可配置 | 待做 |

## 参考

- RFC 8259（JSON 规范）
- `nlohmann/json`（参考它的 DOM 与错误信息设计，未抄实现）
- JSONTestSuite（github.com/nst/JSONTestSuite）
- GoogleTest / Google Benchmark
