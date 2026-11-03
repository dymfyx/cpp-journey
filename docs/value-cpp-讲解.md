# value.cpp 逐段讲解（第 2 周参考实现）

文件：`C:\Users\dym54\Desktop\c++项目\projects\p1-jq-lite\src\value.cpp`（175 行）
作用：`Value` 的实现。**你第 2 周要重写的只是其中 dump 相关的 4 个函数**，这里按文件顺序讲清楚每一段在干嘛、为什么这么写。

---

## 1. 文件结构总览

| 行号 | 内容 | 角色 |
|---|---|---|
| 1–10 | 注释 + include | 说明「基础设施，别改」 |
| 12–121 | 匿名 namespace：`append_indent`、`dump_string`、`dump_impl` | 序列化工具（**你要重写的**） |
| 123–155 | `type()`、`is_*()`、`as_*()` | 类型查询与取值 |
| 157–165 | `size()` | 容器元素个数 |
| 167 | `operator==` | 整棵树比较 |
| 169–173 | `Value::dump(int indent)` | 序列化入口 |

匿名 namespace 的作用：里面三个函数只在**本文件内可见**（内部链接），不会和别的翻译单元重名，也不需要写进头文件。

---

## 2. `append_indent`（14–20）

```cpp
void append_indent(std::string& out, int indent, int depth) {
    if (indent < 0) return;                 // 紧凑模式：什么都不做
    out.push_back('\n');
    out.append(static_cast<std::size_t>(indent * depth), ' ');
}
```

- `indent < 0` 表示紧凑模式（`dump()` 默认值就是 -1），直接返回；
- 否则追加「一个换行 + `indent * depth` 个空格」；
- `depth` 是当前嵌套层数：顶层 0、数组元素 1、嵌套一层 2……
- `static_cast<std::size_t>` 因为 `std::string::append` 要 `size_t`，而 `indent * depth` 是 `int`，不转会触发 `-Wsign-conversion`（项目开了 `-Werror`）。

它被调用的位置很关键：**每个元素前面一次**（`depth+1`），**收尾括号前面一次**（`depth`），这样 `]`/`}` 能回到外层缩进。

---

## 3. `dump_string`（22–59）

把字符串转成 JSON 字符串字面量，**前后各加一个双引号**：

```cpp
out.push_back('"');
for (const char raw : text) {
    const auto ch = static_cast<unsigned char>(raw);  // 0..255，避免 char 符号问题
    switch (ch) {
        case '"':  out += "\\\""; break;   // "  →  \"
        case '\\': out += "\\\\"; break;   // \  →  \\
        case '\b': out += "\\b";  break;   // 退格等 5 个控制字符 → 两字符转义
        ... '\f' '\n' '\r' '\t'
        default:
            if (ch < 0x20U) {              // 其它控制字符 → \u00XX
                char buffer[8] = {};
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(ch));
                out += buffer;
            } else {
                out.push_back(static_cast<char>(ch));   // 普通字符 / UTF-8 字节 原样
            }
    }
}
out.push_back('"');
```

要点：
- `char` 在 Linux 上是有符号的，中文 UTF-8 字节是 0x80–0xFF → 负数，直接比较会误判，所以先转 `unsigned char`；
- 5 个命名转义（`\b \f \n \r \t`）单独处理，其余 `< 0x20` 统一 `\u00XX`；
- 中文按字节原样输出（JSON 允许 UTF-8 直接出现）；
- 输出端 `push_back(static_cast<char>(ch))` 把字节塞回 `std::string`。

---

## 4. `dump_impl`（61–119）—— 核心递归

签名：

```cpp
void dump_impl(std::string& out, const Value& value, int indent, int depth);
```

- `out` 是**引用**：所有层级往同一个字符串里追加。如果每层返回新字符串再拼接，会变成 O(n²) 拷贝；
- `value` 是 `const&`：不拷贝整棵树；
- `indent`：-1 紧凑，>=0 缩进宽度；
- `depth`：当前层数，决定缩进空格数。

主体是 `switch (value.type())`，六个分支每个都以 `return` 结束：

| 类型 | 处理 |
|---|---|
| `Type::Null` | `out += "null"` |
| `Type::Bool` | `out += bool ? "true" : "false"` |
| `Type::Number` | `snprintf("%.17g")` 到 buffer，再追加。`%.17g` 是 double 能精确往返的最短十进制精度（所以 `0.1` 输出 `0.10000000000000001`） |
| `Type::String` | `dump_string(out, value.as_string())` |
| `Type::ArrayValue` | 见下 |
| `Type::ObjectValue` | 见下 |

### 数组分支（78–95）

```cpp
const Array& items = value.as_array();
if (items.empty()) { out += "[]"; return; }     // ① 空容器特判
out.push_back('[');
for (std::size_t i = 0; i < items.size(); ++i) {
    if (i != 0U) out.push_back(',');            // ② 逗号加在"元素前"，只有非首元素才加
    append_indent(out, indent, depth + 1);      // ③ 先换行缩进到 depth+1
    dump_impl(out, items[i], indent, depth + 1);// ④ 递归元素
}
append_indent(out, indent, depth);              // ⑤ 收尾换行回到 depth
out.push_back(']');
```

为什么①要特判：否则 `dump(2)` 会输出 `"[\n]"`，而规格要求空数组输出 `[]`（测试 `DumpEmptyContainers` 专门测这个）。

为什么②逗号在元素前：这样最后一个元素后面不会多逗号；配合⑤，`]` 独占一行。

### 对象分支（96–117）

结构一样，区别：
- 用 `for (const auto& [key, item] : members)` 结构化绑定遍历 `std::map`；
- 用 `bool first = true` 判断逗号（数组用 `i != 0U`，两者等价，只是写法不同）；
- key 也要走 `dump_string`（key 里的引号/反斜杠一样要转义）；
- key 和 value 之间的冒号：`out += (indent < 0) ? ":" : ": ";`——紧凑模式 `"a":1`，缩进模式 `"a": 1`（有一个空格）；
- `std::map` 按 key 排序，所以输出顺序是按 key 字典序，不是插入顺序。

### 实例追踪

`Value(Object{{"a", Value(Array{Value(1)})}}).dump(2)`：

```text
dump_impl(对象, depth=0)
  '{'
  append_indent(2, 1)          → "\n  "
  dump_string("a")             → "\"a\""
  ": "
  dump_impl(数组, depth=1)
    '['
    append_indent(2, 2)        → "\n    "
    dump_impl(1, depth=2)      → "1"
    append_indent(2, 1)        → "\n  "
    ']'
  append_indent(2, 0)          → "\n"
  '}'
```

最终：`"{\n  \"a\": [\n    1\n  ]\n}"`。

---

## 5. `type()` 与 `is_*`、`as_*`（123–155）

```cpp
Type Value::type() const noexcept {
    if (is_null())   return Type::Null;
    ...
    return Type::ObjectValue;      // 兜底：variant 只剩这一种可能
}
```

- 按顺序判断，最后一个不用 `if`——variant 只有 6 种，前 5 个排除了就必然是对象，这样函数一定有返回值；
- `is_*` 全是 `std::holds_alternative<T>(storage_)` 的一行封装；
- `as_*` 全是 `std::get<T>(storage_)` 的一行封装，**返回引用**（`as_string`/`as_array`/`as_object` 返回 `const T&`，避免拷贝整个容器），类型不对抛 `std::bad_variant_access`；
- `as_array()`/`as_object()` 还有非 const 重载，让调用方可以改内容（比如 `value.as_array().push_back(...)`）。

`type()` 是 `noexcept` 且只读，dump 递归里每层都会调用它。

---

## 6. `size()`（157–165）

```cpp
if (const auto* items = std::get_if<Array>(&storage_)) return items->size();
if (const auto* members = std::get_if<Object>(&storage_)) return members->size();
return 0;
```

- `get_if` 不匹配时返回 `nullptr`，**不抛异常**，所以适合这种「问一下再取」的场景；
- 字符串/数字/bool/null 一律返回 0（这是设计取舍：想要字符串长度用 `as_string().size()`）。

---

## 7. `operator==`（167）

```cpp
return storage_ == other.storage_;
```

variant 的 `==` 规则：先比 index（类型必须一致），再比里面存的值；数组/对象会递归比较元素。所以整棵 JSON 树相等是一次 `==` 搞定。

注意：`double` 走 `==`，所以 `NaN != NaN`；JSON 一般不含 NaN，影响不大。

---

## 8. `dump()` 入口（169–173）

```cpp
std::string Value::dump(int indent) const {
    std::string out;
    dump_impl(out, *this, indent, 0);
    return out;
}
```

建一个空字符串 → 从 depth 0 开始递归 → 返回（RVO/移动，不会多拷贝）。

---

## 9. 你重写时真正的考点

要复现的行为（测试已经固定）：

1. 紧凑：`[1,2]`、`{"a":1}`；缩进：`[\n  1,\n  2\n]`；
2. 空容器在两种模式下都是 `[]` / `{}`；
3. 字符串转义七类 + 控制字符 `\u00XX` + UTF-8 原样；
4. 数字 `%.17g`；
5. 对象 key 也要转义、`map` 排序、缩进时冒号后有空格；
6. 缩进模式下逗号在行尾、换行在下一个元素前、收尾括号前回到外层缩进。

实现方式不必和上面一样：比如数组和对象可以统一用 `bool first`，也可以用 `std::visit` 替代 `switch`——**只要 25 个测试全绿就合格**。
