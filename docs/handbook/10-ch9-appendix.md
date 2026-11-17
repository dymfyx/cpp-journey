# 第 9 章 附录

## A. 命令速查

### 环境与构建

```bash
# 进入项目（WSL）
cd /mnt/c/Users/dym54/Desktop/c++项目/projects/p1-jq-lite

# 配置（首次或改了 CMakeLists）
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF

# 构建（增量）
cmake --build build
cmake --build build-rel

# 安装到前缀（必须用 Release + sanitizer OFF）
cmake --install build-rel --prefix /tmp/jq-stage
```

### 测试与基准

```bash
ctest --test-dir build --output-on-failure          # 全部
./build/jq_tests --gtest_filter='ParserTest.*'      # 单个套件
./build/jq_tests --gtest_list_tests                 # 列出所有用例
./build/jq_fuzz 20000 12648430                      # fuzz：迭代数 + seed
./build/jq_jsontestsuite                            # 官方 JSONTestSuite
./build-rel/jq_bench --benchmark_repetitions=10     # 性能（看 median）
./build-rel/jq_bench --benchmark_filter='Dump'      # 过滤
./build-rel/bench_nlohmann                          # 与 nlohmann 对拍
```

### Git（注意分工）

```bash
# WSL：本地提交
git add -A && git commit -m "feat: ..."
git status --short
git log --oneline -5

# Windows PowerShell：推送（WSL 连不上 github.com:443）
git -C "C:\Users\dym54\Desktop\c++项目" push
```

## B. 文件地图

```
cpp-journey/
├── README.md                 # 仓库介绍（第一人称）
├── ROADMAP.md                # 三年路线（第一人称）
├── docs/
│   ├── HANDOFF.md            # 交接文档：环境/协议/坑/待办
│   ├── design.md             # P1 设计文档：DOM/深度/数字/测试
│   ├── value-cpp-讲解.md      # value.cpp 逐段讲解
│   └── handbook/             # 本书源文件（00-front ~ 10-back）
├── projects/
│   ├── p0-01-raii/           # RAII File（15 测试）
│   ├── p0-02-buffer/         # Buffer 五法则（15 测试）
│   ├── p1-jq-lite/           # JSON 解析器/序列化/查询（68 测试）
│   └── p2-webserver/         # epoll HTTP 服务器（进行中）
├── scripts/                  # WSL 安装、知库导入、手册构建等脚本
└── .github/workflows/ci.yml  # GitHub Actions
```

关键文件职责：

| 文件 | 职责 |
|---|---|
| `p1-jq-lite/src/parser.cpp` | 递归下降解析 + 深度守卫 |
| `p1-jq-lite/src/value.cpp` | DOM 构造/查询/序列化（dump） |
| `p1-jq-lite/src/path.cpp` | jq 风格路径查询 |
| `p1-jq-lite/src/pointer.cpp` | RFC 6901 JSON Pointer |
| `p1-jq-lite/include/jq/*.hpp` | 公开接口（不要改签名） |
| `p1-jq-lite/tests/` | 单元测试 + fuzz + JSONTestSuite |
| `p1-jq-lite/benchmark/` | Google Benchmark + nlohmann 对拍 |
| `p2-webserver/include/p2/*.hpp` | Socket/Epoll/EchoServer 接口 |
| `scripts/md2html.py` + `build-handbook.ps1` | 本书生成流水线 |

## C. 术语表

| 术语 | 一句话解释 |
|---|---|
| RAII | 资源获取即初始化：构造拿资源、析构放资源 |
| 拷贝/移动 | 拷贝复制内容；移动转移所有权（源对象被掏空） |
| `noexcept` | 「这个函数不抛异常」的类型契约，影响 vector 扩容选移动还是拷贝 |
| 栈展开 | 异常传播时逐层析构自动对象 |
| `std::variant` | 类型安全的 union，六选一（JSON 的 DOM 用它） |
| SSO | 短字符串优化：≤15 字节存在 `std::string` 对象内部，不分配堆 |
| `string_view` | 不拥有的只读字符串视图（指针 + 长度） |
| fd | 进程 fd 表的下标，内核对象的句柄号 |
| epoll | Linux IO 多路复用：内核只报告就绪的 fd |
| LT / ET | 水平触发（有数据就报）/ 边缘触发（状态变化才报，必须读到 EAGAIN） |
| 粘包 | TCP 是字节流、没有消息边界，导致「多条消息粘在一起」 |
| DoS | 拒绝服务：用畸形输入耗尽资源（如递归深度打爆栈） |
| fuzz | 随机/变异输入测试，断言「只允许抛预期异常」 |
| benchmark | 基准测试，用多次重复的中位数衡量性能 |
| Ryu | 最短往返浮点打印算法（`std::to_chars` 的实现基础之一） |
| ABI | 应用二进制接口（栈展开表、参数传递等约定） |

## D. 更新记录

| 版本 | 日期 | 变更 |
|---|---|---|
| 1.0 | 2026-11-17 | 首版：项目全景 + 语言基础 + 现代类型 + 解析器 + 工具链 + Linux 网络 + 性能 + 学习方法 + 深层问题 |

## E. 怎么继续用这本书

1. **面试前**：读第 0 章（项目介绍）+ 每章「一句话」+ 检查表；
2. **写新项目时**：先翻第 7 章（预习卡、节奏、AI 边界）；
3. **卡住时**：按第 7.7 节（最小复现 + 二分）；
4. **每完成一个项目**：回来补一章，把「我踩的坑」写进对应位置。

> 这本书会随项目增长。每次更新，改对应章节 + 加一行「更新记录」即可。
