# jq-lite

用现代 C++20 从零实现一个 JSON 解析器 + 命令行查询工具。P1 项目，10 周。

## 为什么做这个

- 简历上它是「手写词法/语法分析、错误定位、内存布局、性能优化」的完整证据链；
- 后面 P4 光追的场景文件、P3 的 manifest、P2 的配置，都用它读；
- 面试高频题「手写 JSON 解析器」能直接拿它讲。

## 目录

```
p1-jq-lite/
├── CMakeLists.txt
├── include/jq/
│   ├── value.hpp      # DOM 类型（std::variant），基础设施，别改
│   ├── error.hpp      # ParseError（带行号列号），基础设施，别改
│   └── parser.hpp     # 对外接口 parse(string_view) -> Value
├── src/
│   ├── parser.cpp     # ★ 第 1 周你要实现的部分
│   └── value.cpp      # dump() 已实现；第 2 周不看它重写一遍
└── tests/test_parser.cpp   # 13 个验收测试，不要改
```

## 第 1 周（10.23 – 10.29）：递归下降解析器

任务：实现 `src/parser.cpp` 里的 `jq::parse(std::string_view)`。

完成标准：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

**13/13 全绿**，且 ASan/UBSan 没有任何报告。

### 实现提示

1. **先写状态结构体**，不要一上来写 200 行 if：

   ```cpp
   struct Parser {
       std::string_view text;
       std::size_t pos = 0;     // 当前字节位置
       std::size_t line = 1;    // 1-based
       std::size_t column = 1;  // 1-based

       char peek() const;       // 越界返回 '\0'
       char get();              // 前进一个字符并维护行列号
       void skip_ws();
       [[noreturn]] void fail(const std::string& message) const;  // 抛 ParseError
       Value parse_value();
       Value parse_object();
       Value parse_array();
       Value parse_string();
       Value parse_number();
   };
   ```

2. **数字**：先用 `std::from_chars` 把最长合法前缀转成 `double`，转不动就报错。
   注意 `from_chars` 不接受前导 `+`，正好符合 JSON 规范；`01`、`.5`、`1.` 必须在调用前手工拒绝。

3. **字符串**：`\uXXXX` 先转 BMP 内的码点，再按 UTF-8 规则写 3 字节（中文都在 BMP 内，够用）。
   代理对（`\ud83d\ude00`）第 3 周再处理。

4. **错误信息**：格式随意，但必须通过 `ParseError(message, line, column)` 抛出。
   测试只检查「抛的是 ParseError」和「行列号在第 2 行第 8 列之后」。

5. **不要用 `out += 字符` 拼字符串时踩 `\0` 的坑**（这里是解析，输入里出现的 `\0` 应该报错）。

### 常见坑（等你踩）

- 解析完一个值忘记 skip 尾部空白，导致 `"1 "` 报「有多余内容」；
- 数组/对象里 `,` 和 `]`/`}` 的顺序写反，`[1,]` 没拒绝；
- `parse_string` 里忘记处理 `\u` 后面不足 4 位十六进制；
- 行列号只在 `get('\n')` 时更新，导致错误报告偏一列。

## 10 周里程碑

| 周 | 内容 | 验收 |
| --- | --- | --- |
| 1 | 递归下降解析器（值/数组/对象/字符串/数字）+ 错误行列号 | 13/13 |
| 2 | 不看源码重写 `dump()`；补序列化测试（转义、缩进、数字格式） | 25/25 |
| 3 | CLI `jq-lite`：读文件/stdin、路径查询 `.a.b[0]`、`-r` 原始输出、`-c` 紧凑输出 | 手工用例 + 3 个 CLI 测试 |
| 4 | 错误信息打磨；跑通 JSONTestSuite（y_ 全过、n_ 全拒） | 通过率 ≥ 95% |
| 5–6 | fuzz（libFuzzer，ASan）24 小时无崩溃；接 Google Benchmark 对比 nlohmann/json | 吞吐 ≥ 100 MB/s |
| 7 | `docs/design.md`：DOM 布局、递归深度限制、错误恢复取舍 | review 通过 |
| 8 | CMake install/export、GitHub Actions（Ubuntu + Windows）、README 定稿 | CI 全绿 |
| 9–10 | 扩展任选：NDJSON 流式解析 / JSON Pointer / 深度限制 + 栈溢出防护 | 自选验收 |

## 参考

- RFC 8259（JSON 规范）
- `nlohmann/json`（看它的 DOM 和错误信息设计，不要抄实现）
- JSONTestSuite（github.com/nst/JSONTestSuite）
