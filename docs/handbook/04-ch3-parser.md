# 第 3 章 解析器、序列化与查询

> 这一章是 `p1-jq-lite` 的全部核心：把一段文本变成内存树（解析），再变回文本（序列化），再按路径从树里取东西（查询）。它是整个仓库里最有「计算机科学」味的部分。

## Q3.1 JSON 文法长什么样？递归下降为什么对

**疑问**：手写解析器从哪开始？为什么用递归下降？

**解答**：JSON 文法本身是递归的（RFC 8259）：

```
value   = object | array | string | number | "true" | "false" | "null"
object  = "{" [ member { "," member } ] "}"
member  = string ":" value
array   = "[" [ value { "," value } ] "]"
string  = '"' chars '"'
number  = [ "-" ] int [ frac ] [ exp ]
```

**递归下降** = 给每个非终结符写一个函数，函数之间互相调用，代码结构和文法**一一对应**：

```cpp
Value parse_value();     // 按首字符分派
Value parse_object();    // 吃掉 '{'，循环 parse member，吃掉 '}'
Value parse_array();     // 吃掉 '['，循环 parse value，吃掉 ']'
std::string parse_string();
Value parse_number();
```

好处：最容易写对、报错位置准确、可读性高。代价：调用栈随嵌套深度增长（所以需要第 3.9 节的深度限制）。

**一句话**：递归下降 = 一个非终结符一个函数；写起来像抄文法。

**项目里**：`src/parser.cpp` 的 `Parser` 类；测试 `tests/test_parser.cpp` 的 13 个用例。

## Q3.2 Parser 的状态有哪些

**疑问**：为什么要有 `pos_`、`line_`、`column_` 三个成员？

**解答**：

```cpp
class Parser {
    std::string_view text_;
    std::size_t pos_ = 0;      // 当前字节下标
    std::size_t line_ = 1;     // 1-based
    std::size_t column_ = 1;   // 1-based
    std::size_t depth_ = 0;    // 当前嵌套深度
    ParseOptions options_;

    char peek() const;          // 偷看，不前进；末尾返回 '\0'
    char take();                // 取一个并前进，同时维护行/列
    void skip_whitespace();     // 跳过 ' ' '\t' '\n' '\r'
    [[noreturn]] void fail(const std::string& message) const;  // 抛 ParseError(message, line_, column_)
};
```

- `peek` 在 `take` 之前判断「是不是我要的字符」，不消费；
- `take` 是**唯一**推进游标的地方，所以行列号维护只写一处；
- 遇到 `'\n'`：`line_++`、`column_ = 1`；否则 `column_++`；
- `fail` 用 `[[noreturn]]` 标注，编译器知道它不返回，`switch` 里就不会报「缺少 return」。

**一句话**：所有前进走同一个函数，行列号就不会算错。

**项目里**：`src/parser.cpp` 第 40–90 行。

## Q3.3 数字解析：`from_chars` + 严格文法

**疑问**：`std::from_chars` 会接受 `01`、`1.`、`.5` 吗？

**解答**：`from_chars` 只转换「它能转的最长前缀」，**不负责 JSON 文法**，所以必须自己先校验：

```cpp
Value parse_number() {
    const std::size_t start = pos_;

    // ① 整数部分
    if (peek() == '-') take();
    if (peek() == '0') {
        take();                              // "0" 后面不能跟数字 → "01" 非法
    } else if (peek() >= '1' && peek() <= '9') {
        while (peek() >= '0' && peek() <= '9') take();
    } else {
        fail("expected digit");
    }

    // ② 小数部分：小数点后必须有数字
    if (peek() == '.') {
        take();
        if (!(peek() >= '0' && peek() <= '9')) fail("expected digit after '.'");
        while (peek() >= '0' && peek() <= '9') take();
    }

    // ③ 指数部分：e/E 后可选 +/-，然后必须有数字
    if (peek() == 'e' || peek() == 'E') {
        take();
        if (peek() == '+' || peek() == '-') take();
        if (!(peek() >= '0' && peek() <= '9')) fail("expected digit in exponent");
        while (peek() >= '0' && peek() <= '9') take();
    }

    // ④ 用 from_chars 把合法前缀转成 double（C++17 起）
    double value = 0.0;
    const auto result = std::from_chars(text_.data() + start, text_.data() + pos_, value);
    if (result.ec != std::errc{}) fail("number out of range");
    return Value(value);
}
```

关键点：`from_chars` 不认前导 `+`、不认 `01`、不认 `.5`——这正是 JSON 的要求；但 `1e999` 会返回 `result_out_of_range`，要转成 `ParseError`。

**一句话**：手写文法校验负责「形状」，`from_chars` 负责「数值」，两者分工。

**项目里**：`src/parser.cpp` 的 `parse_number`；边界自测：`42`/`-3.25`/`1e3`/`2.5E-2`/`1.0e+2` 合法，`01`/`-`/`1.`/`.5`/`1e`/`+1` 必须报错。

## Q3.4 字符串解析：转义与 UTF-8

**疑问**：`\u4e2d` 怎么变回「中」？为什么要区分「码点」和「UTF-8 字节」？

**解答**：

- JSON 字符串里的 `\uXXXX` 是 **UTF-16 码元**（4 位十六进制）；
- Unicode **码点**（code point）是抽象编号（U+4E2D = 20013）；
- 文件里实际存的是 **UTF-8 字节序列**（`中` = `E4 B8 AD` 三个字节）。

解析流程：

1. 遇到 `\u` 读 4 位十六进制 → 得到码点；
2. 如果码点在代理区（`D800–DFFF`）→ 需要再读一个 `\uXXXX` 组成代理对（本项目暂未处理，超出 BMP 的字符会保留为码点或报错）；
3. 把码点按 UTF-8 规则编码成 1–4 字节，追加到结果字符串。

```cpp
std::string encode_utf8(unsigned codepoint) {
    std::string out;
    if (codepoint <= 0x7F) {
        out.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else if (codepoint <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
    }
    return out;
}
```

另外两条规则：

- 控制字符（`U+0000–U+001F`）在 JSON 里**不允许裸出现**，必须转义 → 解析时遇到直接 `fail`；
- 反斜杠转义表：`\" \\ \/ \b \f \n \r \t \uXXXX`，其余组合报错。

**一句话**：`\u` 是码点，存进内存要编码成 UTF-8；控制字符必须转义。

**项目里**：`src/parser.cpp` 的 `parse_string` + `read_hex4`；测试 `ParsesUnicodeEscapes`、`RejectsControlCharacter`。

## Q3.5 数组与对象：逗号、括号、错误定位

**疑问**：`[1,]`、`{"a":}` 为什么会被「自然拒绝」？

**解答**：因为解析循环是「解析一个元素 → 看下一个字符」：

```cpp
Value parse_array() {
    take();                        // '['
    Array arr;
    if (peek() == ']') { take(); return Value(std::move(arr)); }   // 空数组
    while (true) {
        arr.push_back(parse_value());     // 这里遇到 ']' 会 fail("unexpected character")
        skip_whitespace();
        if (peek() == ',') { take(); continue; }
        if (peek() == ']') { take(); return Value(std::move(arr)); }
        fail("expected ',' or ']'");
    }
}
```

- `[1,]`：逗号后 `parse_value` 看到 `]` → 报错；
- `{"a":}`：冒号后 `parse_value` 看到 `}` → 报错；
- 错误位置由 `fail` 带上当前 `line_`/`column_`，测试 `ErrorCarriesLineAndColumn` 验证。

**一句话**：不写「专门拒绝尾逗号」的代码，是「缺少元素」这个自然情况被拒绝。

**项目里**：`src/parser.cpp` 的 `parse_array`/`parse_object`；测试 `RejectsTrailingComma`。

## Q3.6 dump 序列化：三个辅助函数的分工

**疑问**：`dump()` 为什么拆成 `dump_string`、`append_indent`、`dump_impl`？

**解答**：

| 函数 | 职责 | 关键点 |
|---|---|---|
| `dump_string(out, text)` | 字符串按 JSON 规则转义 | 遍历 `unsigned char`，处理 `"`、`\`、控制字符、`\u` |
| `append_indent(out, indent, depth)` | 换行 + 缩进 | 紧凑模式（indent<0）不做任何事 |
| `dump_impl(out, value, indent, depth)` | 递归分派六种类型 | 数组/对象的逗号在元素前、收尾在最后 |

```cpp
void append_indent(std::string& out, int indent, int depth) {
    if (indent < 0) return;                       // 紧凑模式
    out.push_back('\n');
    out.append(static_cast<std::size_t>(indent * depth), ' ');
}
```

数组分支的关键顺序：

```
'[' → 每个元素：前一个元素之后先加 ','，再 append_indent(depth+1)，再递归 dump_impl
    → 收尾：append_indent(depth) + ']'
```

**为什么逗号要放在「当前元素前」而不是「前一个元素后」**：这样最后一个元素后面不会多出逗号，收尾只需补换行和括号。

**一句话**：缩进管格式，转义管合法，递归管结构；逗号位置决定代码是否简单。

**项目里**：`src/value.cpp`；测试 `DumpCompact`/`DumpPretty`/`DumpStringsEscape`。

## Q3.7 路径查询：把路径编译成指令，再解释执行

**疑问**：`.users[1].name` 是怎么变成一次数组/对象查找的？

**解答**：两步走——**编译**成段序列，再**解释执行**：

```cpp
struct PathSegment {
    bool is_index = false;
    std::string key;
    std::size_t index = 0;
};

std::vector<PathSegment> parse_path(std::string_view path);   // 编译
Value evaluate_path(const Value& root, std::string_view path); // 执行
```

`evaluate_path` 的核心是一个 `current` 指针沿树下降：

```cpp
const Value* current = &root;
for (const auto& seg : segments) {
    if (current->is_object()) {
        const auto& obj = current->as_object();
        const auto it = obj.find(seg.key);
        if (it == obj.end()) throw QueryError("missing key: " + seg.key);
        current = &it->second;
    } else if (current->is_array()) {
        // 校验纯数字、无前导零、不越界
        current = &current->as_array()[seg.index];
    } else {
        throw QueryError("cannot descend into scalar");
    }
}
return *current;      // 返回拷贝（可接受的小代价）
```

这里有两个易错点（我都踩过）：

1. **两个独立的 `if`**：对象分支下降后，又用同一个 `seg` 去判断数组分支 → 报「unexpected type」。必须是 `if/else if/else`；
2. **循环不变式**：每轮开头要么是 `.` 要么是 `[`；`i = 0` 开始，先可选跳过 `.`，再分支，不要手动「跳过第一个点」。

**一句话**：路径 = 指令序列；执行 = 沿指针走，每步校验类型和边界。

**项目里**：`src/path.cpp`、`include/jq/path.hpp`；测试 16 个 `PathTest`。

## Q3.8 JSON Pointer（RFC 6901）

**疑问**：JSON Pointer 和 jq 路径有什么区别？

**解答**：

- jq 路径：`.a.b[0]`（点 + 方括号，人类友好，jq 工具语法）；
- JSON Pointer：`/a/b/0`（每段前斜杠，**标准**，被 JSON Patch、JSON Schema、OpenAPI 广泛使用）。

RFC 6901 的两个转义：

| 原文 | 转义 |
|---|---|
| `~` | `~0` |
| `/` | `~1` |

解码顺序（先 `~1` 还是 `~0` 很重要）：**先处理 `~1`，再处理 `~0`**，否则 `~01` 会被解成 `~1` 再当成 `/`（错误）——正确结果是字面 `~1`。

```cpp
std::string decode_pointer_token(std::string_view raw) {
    std::string out;
    out.reserve(raw.size());
    for (std::size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] != '~') { out.push_back(raw[i]); continue; }
        if (i + 1 >= raw.size()) throw QueryError("'~' at end");
        switch (raw[++i]) {
            case '0': out.push_back('~'); break;
            case '1': out.push_back('/'); break;
            default: throw QueryError("invalid escape");
        }
    }
    return out;
}
```

评估逻辑与 jq 路径相同：对象按 key、数组按十进制下标（**禁止前导零**，`/01` 非法）。

**一句话**：Pointer 是标准化的路径；转义顺序错了，`~01` 就会解析错。

**项目里**：`src/pointer.cpp`、`tests/test_pointer.cpp`（20 个测试）。

## Q3.9 递归深度限制与 DoS

**疑问**：为什么 10 万层的 `[` 会让程序崩溃？为什么限制是 512？

**解答**：

- 递归下降每嵌套一层就多一个栈帧（ASan 下每帧更大）；
- 8MB 栈 ÷ 每帧几百字节 ≈ 几万层，10 万层的输入必然 **stack-overflow**；
- ASan 检测到后直接 `abort`（进程死，不是异常）。

修复用 RAII 守卫：

```cpp
struct Depth_guard {
    explicit Depth_guard(Parser& p) : parser_(p) {
        ++parser_.depth_;
        if (parser_.depth_ > parser_.options_.max_depth) {
            --parser_.depth_;                 // 构造没完成，析构不会执行，必须手动回滚
            parser_.fail("maximum nesting depth exceeded");
        }
    }
    ~Depth_guard() { --parser_.depth_; }
    Depth_guard(const Depth_guard&) = delete;
    Depth_guard& operator=(const Depth_guard&) = delete;
private:
    Parser& parser_;
};
```

`parse_array`/`parse_object` 各加一行 `Depth_guard guard(*this);`。超限时抛 `ParseError` → 栈展开 → 每层守卫析构、深度还原 → 调用方可以捕获（可恢复），而不是 abort。

**为什么是 512**：官方 `y_` 用例最深 3–4 层；`i_structure_500_nested_arrays.json` 有 500 层需要放行；`nlohmann/json` 默认 1024，取同一量级；8MB 栈下 512 层只占几百 KB。这是**工程经验值，不是规范要求**——所以后来做成了 `ParseOptions{max_depth}` 参数。

**一句话**：深度限制把「栈溢出 abort」变成「可捕获的 ParseError」，这是 DoS 防护。

**项目里**：`src/parser.cpp` 的 `Depth_guard`；`tests/test_parser_options.cpp`（4 个测试）；`docs/design.md` 第 4 节。

## Q3.10 测试金字塔：单测、fuzz、官方用例、基准

**疑问**：写了单元测试不就够了吗？

**解答**：四层，各管一类风险：

| 层 | 工具 | 抓什么 | 项目里 |
|---|---|---|---|
| 单元测试 | GoogleTest | 手写规则、边界 | 68 个测试 |
| fuzz 驱动 | 自己写的随机生成 | 人想不到的输入组合、崩溃 | `jq_fuzz` 10 万次零崩溃 |
| 官方用例 | JSONTestSuite | 规范符合性（`y_` 必须收、`n_` 必须拒） | 95/95、188/188 |
| 基准 | Google Benchmark | 性能回归 | `jq_bench` 与 nlohmann 对拍 |

fuzz 的关键断言有两条：

1. **合法输入**：`parse(dump(v))` 必须等于 `v`（往返）；
2. **畸形输入**：只允许抛 `jq::ParseError`，不允许崩溃、不允许抛别的异常。

**一句话**：单测覆盖「我知道的」，fuzz 覆盖「我没想到的」，官方用例覆盖「规范要求的」。

**项目里**：`tests/fuzz_parser.cpp`、`tests/jsonsuite/`、`.github/workflows/ci.yml`。

## Q3.11 本章检查表

- [ ] 能默写 JSON 文法，并说明递归下降如何对应
- [ ] 能解释 `peek`/`take`/`skip_whitespace`/`fail` 的分工
- [ ] 能说出 `from_chars` 与 JSON 数字文法的分工
- [ ] 能画出 `\uXXXX` → 码点 → UTF-8 字节的流程
- [ ] 能解释 dump 的逗号放在「元素前」的原因
- [ ] 能说明路径查询「编译 + 解释执行」两步
- [ ] 能解释 `~0`/`~1` 的解码顺序
- [ ] 能讲清 Depth_guard 的 RAII 回滚和 512 的四条理由
- [ ] 能说出四层测试各抓什么
