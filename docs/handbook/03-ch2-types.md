# 第 2 章 现代类型

> 这一章的问题来自 `jq::Value` 的设计以及 dump/路径查询的实现。核心是：**用类型表达约束**——能编译期查的错误不要留到运行期。

## Q2.1 `std::variant` 是什么？为什么 DOM 用它而不是继承

**疑问**：`Value` 内部是 `std::variant<std::nullptr_t, bool, double, std::string, Array, Object>`，为什么不用基类 + 虚函数？

**解答**：

- `std::variant` 是**类型安全的 union**：任意时刻只装其中一个类型的值，但知道「现在装的是哪个」（`index()`）；
- JSON 的六种类型是**封闭集合**，不会有人在运行时新增一种 JSON 类型——这正是 `variant` 的最佳场景；
- 对比继承 + 虚函数：

| | `std::variant` | 继承 + 虚函数 |
|---|---|---|
| 对象位置 | 栈上（值语义） | 通常堆上（`new`） |
| 拷贝 | 编译器生成的深拷贝 | 要手写 `clone()` |
| 比较 | `operator==` 自动按内容 | 要手写 `equals()` |
| 扩展新类型 | 需要改所有 switch | 加个子类即可（开放） |
| 内存开销 | 取最大成员 + 1 字节 tag（实测 `sizeof(Value) == 56`） | 每个对象 +8 字节 vptr + 堆分配 |
| 分派 | `switch(v.index())` 或 `visit` | 虚函数间接调用 |

实测：`sizeof(jq::Value) == 56`（`std::string` 32 字节、`std::vector` 24、`std::map` 48，最大 48 + tag 1 → 对齐到 8 的倍数 = 56）。

**一句话**：封闭集合用 `variant`（值语义、快、省），开放扩展用继承（多态、可插拔）。

**项目里**：`projects/p1-jq-lite/include/jq/value.hpp`；`docs/design.md` 第 2 节有完整取舍。

## Q2.2 `std::variant` 怎么用：访问、取值、访问者

**疑问**：`std::get<std::string>(storage_)` 是干嘛的？类型不对会怎样？

**解答**：

```cpp
using Storage = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

Storage s = 42.0;                                  // 装 double
double d = std::get<double>(s);                    // ✅
int    n = std::get<int>(s);                       // ❌ 编译错误：variant 里没有 int
// 运行期装错类型：
Storage t = true;
std::get<double>(t);                               // ❌ 抛 std::bad_variant_access

// 安全版：不抛，返回指针
if (const auto* p = std::get_if<double>(&t)) { /* ... */ }

// 访问者模式：一次处理所有类型
std::visit([](const auto& x) { std::cout << x; }, s);
```

`jq::Value` 封装成 `type()` + `is_*()` + `as_*()` 一组接口，就是为了业务代码不直接碰 `variant` 细节：

```cpp
if (v.type() == jq::Type::Number) {
    const double x = v.as_number();   // 内部 std::get<double>，类型不对抛 bad_variant_access
}
```

**一句话**：`get<T>` 用于「我确定类型」，`get_if` 用于「不确定、要探测」，`visit` 用于「每种类型都要处理」。

**项目里**：`src/value.cpp` 的 `as_*` 实现；`dump_impl` 用 `switch(value.type())` 分派（比 `visit` 更直白）。

## Q2.3 `std::string_view` 是什么？为什么解析器参数用它

**疑问**：`parse(std::string_view text)` 和 `parse(const std::string&)` 有什么区别？

**解答**：`string_view` 是「**不拥有**的字符串视图」= 指针 + 长度：

```cpp
std::string s = "hello world";
std::string_view sv = s;          // 零拷贝
sv.substr(0, 5);                  // "hello"（还是零拷贝）
sv.size();                        // 11
sv[0];                            // 'h'
sv.data();                        // 指向 s 的缓冲区（不保证 '\0' 结尾）
std::string copy{sv};             // 要拥有就必须显式拷贝（string_view 构造是 explicit 的）
```

**关键约束**：

1. `string_view` **不拥有内存**：源字符串销毁/扩容后它悬空；
2. `data()` **不保证** `\0` 结尾，不能直接传给 C 函数（如 `std::fopen`）；
3. 返回 `string_view` 的函数必须保证底层数据活得够久——所以 `parse` 接收它、但不存它。

**为什么解析器用它**：调用方可以传 `std::string`、字面量、`char* + 长度`，全都不拷贝；解析过程中只读、不拥有，正好匹配「解析是纯函数式读取」的语义。

**一句话**：`string_view` = 只读借用的字符串；适合参数，不适合成员变量/返回值。

**项目里**：`include/jq/parser.hpp` 的 `parse(std::string_view)`；`dump_string` 里 `const std::string&` 则是因为字符串已存在（不需要视图）。

## Q2.4 `std::map` 还是 `std::unordered_map`？

**疑问**：JSON 对象用 `std::map<std::string, Value>`，为什么不用哈希表？

**解答**：

| | `std::map`（红黑树） | `std::unordered_map`（哈希） |
|---|---|---|
| 查找 | O(log n) | 期望 O(1)，最坏 O(n) |
| 遍历顺序 | **按 key 有序** | 无序（依赖哈希，随机化） |
| 内存 | 每节点指针多 | 桶数组 + 节点 |
| 迭代器稳定性 | 插入不失效 | rehash 后迭代器失效 |

选 `map` 的两个理由：

1. **dump 输出稳定**：相同语义的文档，序列化结果逐字节一致（`{"age":19,"name":"dym"}` 永远 age 在前）——这是可复现性；
2. **相等比较**：`operator==` 对两个 map 直接按序比较，不用先把 key 排序。

代价：小对象上 O(log n) 与 O(1) 差别可忽略（几十个 key）；大对象（几万 key）查找会慢。这是明确的取舍，写在 `docs/design.md` 里。

**一句话**：要确定性输出就用 `map`；只做查找、不关心顺序再考虑 `unordered_map`。

**项目里**：`include/jq/value.hpp` 的 `using Object = std::map<std::string, Value>;`；`dump` 测试 `DumpObjectsSortKeys` 依赖这个顺序。

## Q2.5 `std::unique_ptr` 与所有权表达

**疑问**：什么时候用裸指针、什么时候用 `unique_ptr`、什么时候用引用？

**解答**：

| 表达 | 含义 | 用法 |
|---|---|---|
| `T&` | 不拥有、一定存在 | 函数参数/成员（生命周期由外部保证） |
| `T*` | 不拥有、可能为空 | 可选参数、观察者 |
| `std::unique_ptr<T>` | **独占拥有** | 工厂返回、成员资源、容器多态元素 |
| `std::shared_ptr<T>` | 共享拥有（引用计数） | 确实需要多所有者时（谨慎） |
| `std::optional<T>` | 可能没有值（值语义） | 返回值「可能没有」 |

`jq-lite` 的 `Value` 全用值语义（`std::string`/`vector`/`map`），不需要指针；`p2-webserver` 的 `Socket` 用 RAII 包装 `int fd` 而不是 `unique_ptr`，效果一样（析构关闭）。

规则：**能值语义就值语义；要拥有才 `unique_ptr`；只借用就 `T&`/`T*`。**

**一句话**：让类型说明「谁负责释放」——裸指针什么也不说明。

**项目里**：`include/jq/value.hpp`（值语义）、`include/p2/socket.hpp`（RAII 句柄）。

## Q2.6 `std::optional` 与 `std::expected`（C++23）

**疑问**：函数可能失败、可能没有值，返回什么？

**解答**：

- 失败原因需要携带 → 抛异常（`jq::ParseError`、`jq::QueryError`）；
- 只是「可能没有」→ `std::optional<T>`：

```cpp
std::optional<std::string> find_key(const Object& obj, const std::string& key);
```

- 错误需要作为值返回、不想用异常（高性能/无异常环境）→ `std::expected<T, E>`（C++23）：

```cpp
std::expected<double, ParseError> to_number(std::string_view sv);
```

`jq-lite` 的选择：解析错误用异常（带行列号，便于栈展开清理守卫），没有用 `expected`——这是很标准的库设计取向。

**一句话**：`optional` 表达「无值」，异常表达「失败原因需要传播」，`expected` 是两者结合。

**项目里**：`include/jq/error.hpp` 的 `ParseError`；`src/pointer.cpp` 的 `QueryError`。

## Q2.7 为什么 `Value(0.1).dump() == "0.1"` 而以前是 `"0.10000000000000001"`

**疑问**：同一个数字，为什么两种输出？

**解答**：

- `0.1` 在二进制里是无限循环小数，`double` 只能存最接近的近似值；
- `%.17g` 打印**17 位有效数字** → `0.10000000000000001`（精确还原二进制值）；
- `std::to_chars`（默认 `chars_format::general`）打印**能往返的最短表示** → `"0.1"`（读回来还是同一个 double）。

两者都正确，区别是「最短」还是「最精确」。JSON 场景下最短更好：更短、人类可读、`parse(dump(v)) == v` 依然成立。

**一句话**：`%.17g` 给精确值，`to_chars` 给最短往返值；后者更符合 JSON 习惯。

**项目里**：`src/value.cpp` 的 `Type::Number` 分支（第 7 周优化，Dump/Compact 162→246 MiB/s）；`docs/design.md` 第 5 节。

## Q2.8 模板的最小必要知识

**疑问**：项目里没怎么写模板，需要学到什么程度？

**解答**：本项目只需要看懂/写这几种：

```cpp
// 1. 函数模板：对任意类型求最大值
template <class T>
const T& max_of(const T& a, const T& b);

// 2. 类模板：容器（vector/map 都是）
std::vector<jq::Value> arr;

// 3. 可变参数模板 + 折叠表达式（调试打印用得到）
template <class... Ts>
void print_all(const Ts&... xs) { ((std::cout << xs << ' '), ...); }

// 4. 概念（C++20）：约束模板参数
template <class T>
requires std::integral<T>
T twice(T x) { return x * 2; }
```

深层内容（模板元编程、SFINAE、CRTP）等到需要时再学——**能读懂标准库的模板签名**是当前阶段的目标。

**一句话**：模板 = 编译期生成代码的模具；先会「用」和「读」，再学「造」。

**项目里**：`std::variant`、`std::map`、GoogleTest 的 `TEST` 宏都是模板；`Value` 的构造函数对 `int`/`double` 做了普通重载（不是模板），够用。

## Q2.9 `std::string` 的 SSO 与移动

**疑问**：`std::string` 32 字节，装的是什么？

**解答**：libstdc++ 的 `std::string` 是 32 字节：指针 8 + 长度 8 + union（本地缓冲 16 / 容量 8）。短字符串（≤15 字节）**存在对象内部**（SSO，Small String Optimization），不分配堆内存；长字符串才堆分配。

所以：

- `sizeof(std::string) == 32` 与内容长度无关；
- 移动短字符串可能只是复制 16 字节内部缓冲（比堆指针转移还快）；
- 把 `std::string` 放在 `Value` 里，让 `Value` 变成 56 字节——每次拷贝都要拷这些字节。

**一句话**：小字符串不分配（SSO），但对象本身不小；大对象拷贝要小心。

**项目里**：`sizeof(jq::Value) == 56` 的原因（`docs/design.md` 实测）。

## Q2.10 本章检查表

- [ ] 能说出 `variant` 与继承的四个对比维度
- [ ] 能解释 `get` / `get_if` / `visit` 各自适用场景
- [ ] 能说出 `string_view` 的三条约束（不拥有、不保证 `\0`、生命周期）
- [ ] 能解释 JSON 对象为什么选 `map`
- [ ] 能说明 `%.17g` 与 `to_chars` 的输出差异及取舍
- [ ] 知道 `std::optional`/`std::expected` 分别解决什么问题
