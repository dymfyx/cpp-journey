# 第 0 章 项目全景

## 0.1 我在做什么（一段话版本）

> 我在用「项目驱动」的方式学习现代 C++：不再刷语法题，而是从零写**有实用价值、能上 GitHub、能被面试官追问**的系统——先写 RAII 资源管理小项目打地基，再写一个完整的 JSON 库（解析 + 序列化 + 路径查询 + CLI + fuzz + benchmark + CI + 设计文档），然后转向网络（epoll HTTP 服务器），之后再按 AI 基模方向做 CUDA 算子、mini 推理引擎、分布式训练。整个仓库叫 **cpp-journey**，一个 monorepo，记录我从大一到大三的成长轨迹。

## 0.2 仓库结构

```
cpp-journey/
├── README.md                  # 仓库总览（第一人称叙述）
├── ROADMAP.md                 # 三年路线图（含 P1 复盘与实测数据）
├── docs/
│   ├── HANDOFF.md             # 交接给下一个 AI 的完整说明
│   ├── design.md              # P1 设计文档（DOM/深度/数字格式/测试策略）
│   ├── value-cpp-讲解.md      # P1 参考实现讲解
│   ├── python-track.md        # 12 周 Python 工程训练计划
│   └── handbook/              # 本书源文件
├── projects/
│   ├── p0-01-raii/            # 阶段 0：RAII File 封装（15 测试）
│   ├── p0-02-buffer/          # 阶段 0：Buffer 五大特殊成员（15 测试）
│   ├── p1-jq-lite/            # P1：JSON 库 + CLI（68 测试）
│   └── p2-webserver/          # P2：epoll HTTP 服务器（进行中）
└── scripts/                   # 环境准备、数据下载、本书的构建脚本
```

## 0.3 项目清单与状态

| 项目 | 技术点 | 测试 | 状态 |
|---|---|---|---|
| p0-01-raii | RAII、构造/析构、移动语义、异常安全 | 15 | ✅ 完成 |
| p0-02-buffer | 深拷贝、copy-and-swap、自赋值、`new[]/delete[]` | 15 | ✅ 完成 |
| p1-jq-lite | 递归下降解析、`std::variant` DOM、序列化、路径查询、JSON Pointer、CLI、fuzz、benchmark、CI、install/export | 68 | ✅ 完成 |
| p2-webserver | socket、epoll、HTTP 解析、线程池、定时器 | 11 | 🔄 第 1 周进行中 |

## 0.4 P1 的关键数字（面试可以直接说）

| 指标 | 数值 | 说明 |
|---|---|---|
| 测试数量 | **68** 个（13 解析 + 12 序列化 + 16 路径 + 20 Pointer + 4 ParseOptions + fuzz + JSONTestSuite） | `ctest` 全绿 |
| 官方测试集 | JSONTestSuite **y_ 95/95 全部接受、n_ 188/188 全部拒绝** | 符合 RFC 8259 |
| fuzz | 100,000 次随机输入零崩溃，ASan/UBSan 开启 | 随机合法文档往返 + 随机破坏只允许抛 `ParseError` |
| 解析吞吐 | **133.9 MiB/s**（1MB 文档，Release，g++ 13.3.0） | 比 nlohmann/json 63.9 MiB/s **快 2.1×** |
| 紧凑序列化 | **245.8 MiB/s** | 用 `std::to_chars` 后从 162 提升 |
| 缩进序列化 | 162.5 MiB/s | nlohmann 173.4，略慢 |
| 代码规模 | 约 2500 行 C++（含测试） | 无第三方运行时依赖 |
| CI | GitHub Actions 双配置（Debug+ASan / Release） | 每次 push 自动跑 |
| 安装 | `install/export` + `find_package(jq-lite 0.1)` | 外部项目一行即可使用 |

## 0.5 时间线（真实记录）

| 日期 | 事件 |
|---|---|
| 2026-10-20 | 开始：装 WSL2 + Ubuntu 24.04，搭 RAII 项目 |
| 2026-10-23 | P1 第 1 周：递归下降解析器 13/13 |
| 2026-10-29 | P1 第 2 周：自己重写 `dump()` 25/25 |
| 2026-11-03 | P1 第 3 周：CLI 路径查询 41/41 |
| 2026-11-06 | P1 第 4 周：fuzz + benchmark + CI 42/42 |
| 2026-11-11 | P1 第 5 周：install/export + nlohmann 对拍 |
| 2026-11-12 | P1 第 6 周：JSONTestSuite 接入 + 深度限制 43/43 |
| 2026-11-15 | P1 第 7 周：`std::to_chars` 优化 + README/design.md |
| 2026-11-16 | JSON Pointer（20 测试）+ ParseOptions（4 测试）→ 68/68；仓库 README/ROADMAP 改第一人称 |
| 2026-11-17 | P2 第 1 周启动：TCP socket 层 |

## 0.6 坑的分类索引（对应章节）

| 类别 | 典型坑 | 在哪章 |
|---|---|---|
| 资源管理 | 构造抛异常时的泄漏、移动后源对象未置空、自赋值 | 第 1 章 |
| 语言细节 | `volatile` 禁止优化、`noexcept` 里留 `todo()`、`explicit` 缺省 | 第 1 章 |
| 类型设计 | `variant` 大小、`string_view` 生命周期、`map` vs `unordered_map` | 第 2 章 |
| 解析器 | 越界读、UTF-8 转义、`%.17g` 与最短表示、递归深度 | 第 3 章 |
| 构建 | `-Werror` 全家桶、`-Wshadow`、Debug+ASan 库被链接、`enable_testing` | 第 4 章 |
| 系统 | `::close` 递归、`SO_REUSEADDR`、`EAGAIN`、CLOEXEC | 第 5 章 |
| 性能 | Debug 下 benchmark、跨会话比较、中位数 | 第 6 章 |
| 方法 | 照抄代码、不会调试、理论滞后 | 第 7、8 章 |

## 0.7 一句话总结第 0 章

> 我不是在「学完 C++ 再写项目」，而是在**用项目倒逼把每个语法点变成能讲清楚的工程决策**；衡量标准不是「写出来了」，而是「测试全绿 + 性能有数字 + 坑能复述」。
