# jq-lite 设计文档

> 这份文档回答一个问题：**为什么 jq-lite 是这样写的，而不是另一种写法。**
> 读完应该能说出 3 个关键取舍和各自的代价。

## 1. 目标与范围

- **做什么**：从零实现一个 RFC 8259 的 JSON 库——递归下降解析（错误定位到行列）、`std::variant` DOM、紧凑/缩进两种序列化、`.a.b[0]` 路径查询，外加一个 `jq_lite` 命令行工具。
- **不做什么**：不做流式/增量解析（整份文档一次性读进内存再建树）；不做 schema 校验（只校验 JSON 语法，不管业务字段）；不做 JSON Pointer / JSON Patch（那是 RFC 6901/6902 的另一套语法）；不接受超过 512 层的嵌套；不做非 UTF-8 输入的转码。
- **使用方式**：作为库用（`find_package(jq-lite)` + `jq::jq_lib`，见 `examples/consumer/`），或作为命令行用（`jq_lite [-c] [-r] <路径> [文件]`）。

## 2. DOM 布局：为什么是 `std::variant`

- **六个类型如何表示**：一个 `std::variant<std::nullptr_t, bool, double, std::string, std::vector<Value>, std::map<std::string, Value>>` 内联存放六种 JSON 值，没有继承、没有虚函数、没有单独的 `new Value`。
- **对象大小**：`sizeof(Value) == 56` 字节（Ubuntu 24.04 / g++ 13 实测：`std::string` 32、`std::vector<Value>` 24、`std::map` 48、`double` 8；variant 取最大分支 48 + 类型索引 1，对齐到 56）。
- **与「继承 + 多态」的对比**：继承方案每个节点要一次堆分配、对象里多一个 8 字节 vptr、拷贝要手写 `clone()`、相等比较要虚函数；variant 方案值语义、栈上构造、`operator==` 自动逐分支递归、`as_string()` 能直接返回引用。代价是每个节点固定 56 字节（比小对象大）、类型集合封闭（不能扩展新的 JSON 类型）、拷贝是深拷贝。
- **代价是否可接受**：1 MB 文档约 13000 个对象 × 每对象 3 个成员 × 56 字节，加上字符串内容，内存放大约 3–4 倍。这是 DOM 模型的固有成本，换来的是随机访问和原地修改能力；如果只需要「读一遍就扔」，应该用流式解析，那正是本项目明确不做的。

## 3. 递归下降与错误定位

- **结构**：`Parser` 对象持有 `text_`（`std::string_view`）、`pos_`、`line_`、`column_`，`parse_value()` 按当前字符分派到 `parse_object()` / `parse_array()` / `parse_string()` / `parse_number()` 或字面量，每种值对应一条文法规则、一个函数。
- **行列号怎么维护**：每读一个字符 `column_` 加 1；读到 `'\n'` 时 `line_` 加 1、`column_` 归 1。行列号都从 1 开始。
- **错误信息为什么带位置**：`fail(message)` 用当前的 `line_`/`column_` 抛 `ParseError(message, line, column)`，调用方拿到的就是「第几行第几列、期望什么、实际看到什么」；`test_parser.cpp` 里的 `ErrorCarriesLineAndColumn` 专门校验这一点。
- **与手写状态机 / 生成器的对比**：递归下降的代码结构和 JSON 文法一一对应，最容易写对、最容易在出错处给出准确位置；手写状态机（DFA）更快但难写难改；yacc/ANTLR 这类生成器省去手写解析代码，但引入依赖和生成代码，错误信息也更难控。本项目优先正确性与可读性——192 MiB/s 的性能已经够用。

## 4. 递归深度限制与 DoS 防护

- **问题**：递归下降的调用栈随嵌套层数线性增长。遇到 `n_structure_100000_opening_arrays.json` 这种十万层 `[` 的输入，栈会被耗尽，ASan 报 `stack-overflow`，进程直接 `abort`——一个 100 KB 的文件就能让程序崩溃。
- **方案**：在 `Parser` 里维护当前深度 `depth_`，`parse_array` / `parse_object` 入口构造 `Depth_guard`：构造时 `depth_ + 1`，超过 `k_max_depth = 512` 就先 `--depth_` 再抛 `ParseError`（对象构造未完成时析构函数不会执行，必须手动回滚）；析构时 `--depth_`。这样无论函数正常返回还是异常展开，深度都能正确还原；守卫的拷贝/赋值被 `= delete`，避免重复计数。
- **为什么是 512**：官方 `y_` 用例最深只有 3–4 层；`i_structure_500_nested_arrays.json` 是 500 层，我们选择放行；8 MB 默认栈下 512 层只占几百 KB，而十万层必炸；`nlohmann/json` 默认上限是 1024，取同一量级。这不是规范要求，是「覆盖真实数据 + 放得过 500 层测试 + 留足栈余量」的工程经验值。
- **这是哪类安全问题**：属于**资源耗尽型 DoS（拒绝服务）**，不是注入——攻击者不注入任何代码，只用深嵌套耗尽调用栈，进程 `abort` 退出；放在服务器上，一个请求就能打死整个进程。加了守卫之后，超限变成可捕获的 `ParseError`，调用方可以返回 400 并继续服务。
- **代价与后续**：嵌套超过 512 层的合法文档会被拒绝（与文件字节数无关）。上限目前是编译期常量，后续可以做成 `parse(text, options)` 的参数；更彻底的做法是把递归改成显式栈（迭代解析），把「多深算深」交给内存而不是固定的层数。

## 5. 数字格式：`%.17g` 与下一步

- **为什么需要 roundtrip**：解析得到的 `double` 序列化后，再解析回来必须还是同一个值（逐位相等）。否则 `parse → dump → parse` 会悄悄改数据。
- **`%.17g` 的取舍**：17 位有效数字是 IEEE-754 double 的往返上界，能保证任意 double 都无损；代价是输出不好读——`0.1` 会打印成 `0.10000000000000001`，体积也略大。
- **与 `nlohmann/json` 的差距**：解析我们快 2.1×（1 MB 文档 8.63 ms vs 18.24 ms），但 `dump` 慢 1.6×（131 vs 213 MiB/s）。瓶颈在 `snprintf`：每个数字一次格式化，而 nlohmann 用 Grisu/Dragonbox 算法做 double→文本。下一步改用 `std::to_chars(double)`（C++17 起、GCC 13 已支持），目标是追平 nlohmann。

## 6. 测试策略：三层防御各抓什么

- **单测**：43 个手写用例（13 解析 + 12 序列化 + 16 路径 + 1 fuzz 驱动 + 1 JSONTestSuite 驱动），固定行为、失败定位精确、回归快。
- **fuzz**：随机生成合法 JSON 和随机破坏输入。断言两件事——`parse(dump(v)) == v`（节点级相等，不是字符串相等），以及畸形输入**只允许**抛 `ParseError`（ASan/UBSan 参与，崩溃/越界即失败）。10 万次迭代零崩溃。
- **JSONTestSuite**：官方测试集 318 个文件，`y_` 95/95 全部接受、`n_` 188/188 全部拒绝、`i_` 35 个（接受 24 / 拒绝 11，官方允许实现自定）。
- **三层各自抓不到的东西**：单测抓不到「人想不到的畸形输入」；fuzz 抓不到语义错误（比如往返一致但格式不符合规范）；JSONTestSuite 抓不到性能退化和内存泄漏（它只检查接受/拒绝）。另外三层都不测并发——单线程库，这符合预期。

## 7. 已知限制与后续

- **不支持**：超出 `double` 精度的整数（>2^53 会丢精度，JSON 允许任意精度数字）；Unicode 代理对（`\ud83d\ude00` 这类 4 字节 emoji 暂不处理）；流式解析；注释和尾逗号（JSON5 扩展）。
- **性能**：Parse/Large 约 192 MiB/s、Dump/Compact 约 131 MiB/s、Dump/Pretty 约 109 MiB/s（1 MB 文档，Release + `-O3`，g++ 13.3.0）。目标是把 Dump 提到 200 MiB/s 以上。
- **下一步**：用 `std::to_chars` 重写 dump 的数字输出；支持 JSON Pointer；支持 NDJSON（每行一个 JSON 对象，流式场景最常用）；把嵌套深度上限做成参数；补齐代理对。
