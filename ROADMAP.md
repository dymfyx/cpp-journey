# 现代 C++ 三年项目路线图（大一上 → 大三下）· AI 基模方向版

> 制定：2026-10-20 ｜ 按 AI 系统方向调整：2026-10-23 ｜ 按「AI 基模（预训练 / 训练系统 / 后训练 / 推理）」再调整：2026-10-29
> 适用：中山大学 2026 级人工智能专业（OI 背景、零工程经验） ｜ 目标：基模训练系统 / 推理引擎 / GPU 算子方向，本科先拿实习与科研入口
>
> 本文是总纲。每完成一个项目，在对应小节回填「复盘」，并更新 §8 检查表。时间表可滚动调整。
>
> **双轨说明**：主线 = 基模工程（训练系统 / 推理引擎 / GPU 算子），研究轨（读论文、改架构、做实验、申 PhD）见 §11。两条路前两年内容约 80% 重合，大三按兴趣分叉。
>
> **已确认硬件**：NVIDIA GeForce RTX 5060 Laptop GPU（8 GB 显存，驱动 592.01，WSL2 CUDA 直通已验证 `/dev/dxg` 存在）。
> 8 GB 显存能跑 0.5B–3B 量化模型、能训 30–124M 小模型、能写算子；训不了 7B 全参。基模实验靠**按小时租云 GPU**（AutoDL 等，几十元/天）补齐。

---

## 0. 起点、目标与三条规矩

### 0.1 你的起点（已和你确认）

- **有**：OI 训练出来的算法、数据结构、调试、读代码能力，C 语法，STL 常用容器。
- **缺**：面向对象与对象模型、RAII、move 语义、智能指针、模板、异常/错误处理、并发、CMake/测试/CI、Linux 工程环境、CUDA / 深度学习系统。

### 0.2 三年目标

1. 拥有 3–4 个「能讲 30 分钟」的项目：P3 自动微分张量库 + **手写小 GPT 并训练**（理解预训练）、P4 CUDA 算子（含 attention / Adam / 量化）、P5 推理引擎、P6 分布式训练，P1/P2 作为工程底座。基模方向最值钱的是「**从零训过一个小模型 + 自己写过训练/推理系统**」。
2. 能独立走完：需求 → 接口设计 → 实现 → 测试 → 性能分析（含 GPU profiler）→ 文档。
3. 工具链熟练：CMake / Ninja / clangd / gdb / sanitizers / GoogleTest / Google Benchmark / CUDA Toolkit / Nsight Compute / PyTorch / GitHub Actions。
4. 数学与论文：线代、概率、优化基础扎实；至少精读并复现 2 篇基模论文（如 FlashAttention、Chinchilla/DPO 之一）。
5. **大三上寒假（2029.1–2）用这份简历去投 2029 暑期实习**（基模训练系统 / 推理 / 算子方向），同时准备科研夏令营 / 保研材料。

### 0.3 给 OI 选手立三条规矩

1. **不许** `#include <bits/stdc++.h>`、不许 800 行单文件、不许全局变量当状态。工程代码 = 接口 + 实现 + 测试 + 构建。
2. **先想资源生命周期**（谁分配、谁释放、失败怎么办），再想算法——这是 OOP 和 RAII 的全部意义。
3. **性能必须可度量**：每个项目都要有 benchmark 和「优化前/后」数据。GPU 项目的度量单位是 **tokens/s、FLOPS、显存带宽利用率、相对 PyTorch/cuBLAS 的倍数**，不是「感觉快了」。

---

## 1. 路线总览

| 阶段 | 时间 | 项目 | 难度 | 语言/技术 | 核心能力 | 简历价值 |
|---|---|---|---|---|---|---|
| 阶段 0 | 2026.10–11（4 周） | 工具链 + 现代 C++ + GPU 环境 | ★ | C++17/20、CUDA 环境 | 类/RAII/move/模板/CMake/GTest/nvcc 跑通 | 地基 |
| P1 | 2026.11–2027.1（10 周） | jq-lite：JSON 解析器 + 查询 CLI | ★★ | C++17/20 | 对象建模、错误处理、测试、benchmark；顺带解析 tokenizer.json | 通用 C++ 基本功 |
| P2 | 2027.3–6（**8 周**，压缩） | WebServer：epoll 多线程 HTTP + 推理服务接口 | ★★★ | C++20 | 并发、网络、RAII、压测、动态 batching | serving 底座（给 P5 用） |
| 暑假 2027 | 2027.7–8 | **nanoGPT 复现 + 租卡训练小模型 + pybind11** | ★★★ | PyTorch、CUDA 12.8+、pybind11 | 训练闭环、Transformer 细节、Python 绑定 | 第一个「真训过模型」的作品 |
| P3 | 2027.9–2028.1（14 周） | MiniTensor：张量 + autograd + **手写小 GPT 并训练** | ★★★★ | C++20 | 算子、计算图、反向传播、训练循环 | 基模训练入门王牌 |
| P4 | 2028.3–6（14 周） | CUDA 算子库（matmul / attention / layernorm / **Adam 融合** / 量化） | ★★★★ | CUDA、C++20 | kernel 优化、profiling、数值稳定性 | 算子硬技能 |
| 暑假 2028 | 2028.7–8 | 实习 / 论文复现（FlashAttention、DPO）+ 租卡实验 | — | — | — | 简历定稿 |
| P5 | 2028.9–2029.1（14 周） | MiniInfer：LLM 推理引擎（KV cache / 量化 / batching） | ★★★★★ | C++20 + CUDA | 推理系统、服务化、调度 | 最值钱的项目 |
| P6 | 2029.3–6（14 周） | **分布式训练系统**（DDP → ZeRO/张量并行 → 小模型多卡训练） | ★★★★★ | C++20 + NCCL 概念 | 并行策略、allreduce、checkpoint | 基模系统核心加分 |

**时间不够时的保底方案**：P1 + P3（含小 GPT）+ P5 做深；P4 至少完成 matmul + softmax/layernorm + attention 三个 kernel；P2 压到 6–8 周；P6 只做 DDP + ring allreduce。宁可 3 个讲得透，不要 6 个都半成品。

**贯穿全程（每周固定 2–3 小时，不占项目时间）**：CS336 / CS224N 自学 + 每周一篇论文笔记 + 每 2–3 个月一次租卡实验 + **Python 工程训练（§13.3）**。这些事决定你「像不像做基模的人」，项目只是载体。

**和上一版的差异**：上一版是 AI infra（推理引擎/算子），本版转向基模：P2 压缩为服务底座；P3 增加「手写小 GPT + 训练」；P4 增加训练侧 kernel（Adam 融合）与 attention；暑假 2027 从「CUDA 入门」改为「nanoGPT 复现 + 租卡训练」；P6 从「分布式训练/推理集群」聚焦为训练系统（ZeRO/张量并行）。原存储/图形/编译器项目仍在 `ROADMAP-archive.md`。

---

## 2. 阶段 0：工具链与现代 C++ 心智模型（4 周）

**为什么需要这个阶段**：直接上项目会卡在「编译不过 / 不会组织代码 / 看不懂报错」。4 周把地基打好，之后所有时间都花在项目上。

### 2.1 环境（已基本完成，2026-10-20/23）

| 项 | 状态 |
|---|---|
| 主环境 | ✅ WSL2 + Ubuntu 24.04 + VS Code Remote-WSL |
| 编译器 | ✅ gcc 13.3 / clang 18.1.3（任选其一为主，建议 gcc） |
| 构建 | ✅ CMake 3.28 + Ninja 1.11 |
| 调试 | ✅ gdb + VS Code 调试面板 |
| 语言服务 | ✅ clangd（配 `compile_commands.json`）+ WSL 扩展 |
| GPU | ✅ RTX 5060 Laptop 8 GB，WSL 内 `nvidia-smi` 可见 |
| CUDA Toolkit | ⬜ 待装（Blackwell 架构需 CUDA 12.8+，建议 13.x） |
| pybind11 / PyTorch | ⬜ 暑假 2027 前装（PyTorch 作训练/对拍基线） |

> 选 WSL2 而非纯 Windows 的原因：sanitizers 在 MinGW 上不可用；P2 需要 epoll；perf/ncu 只在 Linux 上顺；实习工作环境也是 Linux。

### 2.2 语言清单（按顺序学，每个主题配一个练习）

| 主题 | 要掌握的点 | 练习 | 状态 |
|---|---|---|---|
| 类 / 构造 / 析构 / 封装 | 不变量、`const` 成员函数、初始化列表、`explicit` | RAII `File` 类 | ✅ 已做（p0-01） |
| 拷贝语义 | 拷贝构造、拷贝赋值、自赋值、异常安全 | `Buffer` 类 | ✅ 已做（p0-02） |
| 移动语义 | 右值引用、`std::move`、`noexcept`、rule of 0/5 | 给 `Buffer` 加移动构造/赋值，放进 `vector` | ✅ 已做 |
| 智能指针 | `unique_ptr` / `shared_ptr` / `weak_ptr`、循环引用 | 用 `unique_ptr` 写链表和二叉树 | ⬜ |
| 模板 | 函数/类模板、特化、C++20 concepts | 写 `Matrix<T>`（P3 的张量雏形） | ⬜ |
| STL | 迭代器、`<algorithm>`、ranges 初探 | 用算法重写 3 道 OI 题 | ⬜ |
| 错误处理 | 异常 vs `std::expected` / `optional`、`noexcept` | 把返回 `-1` 的接口改成 `expected` | ⬜ |
| 现代工具箱 | `optional` / `variant` / `string_view` / `span` / `format` | 用 `variant` 写一个状态机 | ⬜ |
| 编译与链接 | 头文件、ODR、`inline`、`static`、声明与定义分离 | 把单文件拆成 3 个 .cpp + 头文件 | ⬜ |
| **GPU 最小闭环** | `nvidia-smi`、`nvcc`、kernel 语法、grid/block、统一内存 | 写一个 vector add kernel，和 CPU 版对比 100 万向量 | ⬜ 周末做 |

### 2.3 第一个 CMake 工程模板

```
00-playground/
├── CMakeLists.txt
├── .clang-format
├── include/hello/hello.hpp
├── src/hello.cpp
├── src/main.cpp
└── tests/test_hello.cpp
```

```cmake
cmake_minimum_required(VERSION 3.24)
project(hello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # 给 clangd 用

add_library(hello src/hello.cpp)
target_include_directories(hello PUBLIC include)
target_compile_options(hello PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)

add_executable(hello_main src/main.cpp)
target_link_libraries(hello_main PRIVATE hello)

enable_testing()
find_package(GTest CONFIG REQUIRED)      # 或 FetchContent
add_executable(test_hello tests/test_hello.cpp)
target_link_libraries(test_hello PRIVATE hello GTest::gtest_main)
add_test(NAME test_hello COMMAND test_hello)
```

CUDA 项目写法（P4/暑假用，阶段 0 只需心里有数）：

```cmake
project(kernels LANGUAGES CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES 120)   # RTX 5060 = Blackwell sm_120
add_library(kernels src/gemm.cu)
target_compile_options(kernels PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math>)
```

**第 1 周验收**：能不看教程从零建出这个工程，`cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build` 全绿，能下断点，能开 `-fsanitize=address,undefined` 跑测试。

### 2.4 阅读（阶段 0 只读这些）

- learncpp.com：从 Ch.1 读到「类」部分，当教材用。
- cppreference.com：当字典查，别通读。
- 《Effective Modern C++》条款 1–17（类型推导、auto、decltype、const）。

**阶段 0 验收**：上面 10 个练习全部自己写出来并 commit；能解释「为什么 `unique_ptr` 不能拷贝只能移动」；能跑通 vector add kernel。

---

## 3. 项目详解

### P1 · jq-lite：从零写 JSON 解析器 + 查询命令行工具

**做什么**
- 库：按 RFC 8259 解析/序列化 JSON。DOM 支持 null/bool/number/string/array/object；解析错误带**行列号**；UTF-8 校验；数字用 `std::from_chars` 解析。
- CLI：`jqlite '.user.name' data.json` 路径查询；`-c` 紧凑输出；`-p` 缩进；支持 stdin；后续可加 JSON Pointer / JSON Patch。
- **AI 实战延伸**：用 jqlite 解析 HuggingFace 的 `tokenizer.json`（词表 + merges），打印词表大小、查某个 token 的 id —— 这就是 P5 MiniInfer 的 tokenizer 数据来源，提前把格式摸清。
- 进阶：SAX 流式解析（大文件不全量建树）、arena 分配器。

**为什么做 / 有什么用**
- JSON 是配置、数据交换、后端接口、模型元数据的通用格式（`config.json` / `tokenizer.json` / `generation_config.json` 全是 JSON），几乎每个 C++ 项目都会碰到。自己写一遍，以后用 nlohmann/json / rapidjson 时知道底层发生了什么。
- 它天然覆盖「对象建模 + 资源管理 + 错误处理 + 性能」四个主题，是最合适的第一个项目。
- 产出不是玩具：`jqlite` 可以直接当日常命令行工具用（格式化、查询），也是 P3/P5 的配置与 tokenizer 解析库。

**学到什么**
- OOP：**两种建模方式对比**——类继承体系（`JsonValue` 基类 + 子类）vs `std::variant`，理解各自取舍。
- 资源：`unique_ptr` 树、递归析构、异常安全（强/基本保证）。
- move/copy：解析时大量移动字符串与数组，配合 `string_view` 做零拷贝。
- 接口设计：parser → DOM 的分层；错误用 `std::expected<Json, ParseError>` 还是异常，自己各写一版再选。
- 测试：黄金文件、边界用例（深嵌套、非法 UTF-8、数字溢出、`\uD800`）、libFuzzer 模糊测试。
- 性能：Google Benchmark 对比 nlohmann/json，读/写/查询三个维度。

**里程碑（10 周，约 80–120 小时）**
- W1 tokenizer（词法）→ W2 parser（递归下降）→ W3 DOM + serializer → W4 CLI 查询
- W5 测试 + fuzz → W6 benchmark + 与 nlohmann 对比 → W7–8 优化（分配器、字符串驻留）
- W9 tokenizer.json 实战 + README + 架构图 + CI → W10 复盘博客

**验收 / 简历亮点**
- [ ] 通过 JSONTestSuite 95% 以上用例
- [ ] 解析吞吐 ≥ 100 MB/s，README 里有和 nlohmann/json 的对比表
- [ ] 核心路径单测覆盖，CI 绿，fuzz 跑 24 小时无崩溃
- [ ] 能解析真实 `tokenizer.json` 并输出词表统计
- [ ] 简历句：「从零实现 JSON 解析库与查询 CLI，完整支持 RFC 8259 与行列号错误定位；读吞吐 XXX MB/s，较 nlohmann/json 快 X%；libFuzzer 持续 24h 无崩溃；用于解析 tokenizer 配置。」

**难度 ★★☆ ｜ 扩展**：JSON Schema 校验、SAX 流式、SIMD 加速。

---

### P2 · WebServer：epoll 多线程 HTTP/1.1 服务器 + 推理服务接口

> **基模方向调整：压缩为 8 周**。去掉协程版与部分进阶内容，只保留「epoll + Reactor + 线程池 + 异步日志 + 静态文件 + `/predict` 动态 batching」主线——它只是 P5 的服务底座，不是本项目重点。

**做什么**
- 网络层：非阻塞 socket + epoll（ET）+ 主从 Reactor（主线程 accept，从线程处理连接读写）。
- HTTP 层：请求行/头部状态机解析、Content-Length、keep-alive、chunked（可选）、静态文件服务（sendfile/mmap）、简单路由。
- 基础设施：固定大小线程池（任务队列 + 条件变量）、异步日志（多生产者单消费者 + 双缓冲）、连接超时定时器（小根堆或时间轮）。
- **AI 收尾（新增，2 周）**：加一个 `POST /predict` 接口，后端接一个「假模型」或后来 P3 的 MLP，做**动态 batching**（攒 5ms 请求成一批）与并发限流；P5 会把它换成真的 LLM 推理引擎。
- 收尾：优雅关闭、`wrk` 压测报告、部署到云服务器或树莓派。
- 进阶：C++20 协程版本（`co_await` 读/写）替换回调风格，对比两种写法。

**为什么做 / 有什么用**
- 这是后端方向最经典、面试问得最细的项目：QPS、延迟、连接数、惊群、内存占用都能展开。
- **对 AI 方向的意义**：所有推理服务（Triton、vLLM 的 server、TGI）都是「HTTP/gRPC 前端 + 调度 + 批处理 + 后端模型」，你现在写的正是它的骨架；P5 直接复用。
- 能真正跑起来、能压测、能部署——简历上可以放数据，面试官可以访问。
- 同时补上 Linux 系统编程（fd、非阻塞 IO、信号、线程）和并发编程这两块 OI 完全没碰的领域。

**学到什么**
- OOP 接口设计：`HttpHandler` / `Connection` / `EventLoop` 的抽象边界。
- RAII 封装：socket fd、epoll 注册/注销、线程 join——**每个资源都绑定生命周期**。
- 并发：`thread`/`mutex`/`condition_variable`/`atomic`、内存序入门、死锁与数据竞争、TSan 找问题。
- 网络：TCP 粘包/半包、IO 多路复用、ET vs LT、心跳与超时。
- 工程：集成测试（用 curl 脚本打真实请求）、压测方法论、火焰图定位热点。
- 服务化：请求队列、batching 延迟与吞吐的权衡。

**里程碑（12 周）**
- W1–2 socket 封装 + epoll echo server → W3 HTTP 解析器 → W4 静态文件 + 路由
- W5–6 线程池 + 接入 reactor → W7–8 异步日志 + 定时器 → W9–10 `/predict` + 动态 batching
- W11 压测 + 调优 + 部署 → W12 文档 + 复盘（协程版可选）

**验收 / 简历亮点**
- [ ] 单机 4 核 `wrk -t4 -c1000 -d30s` 有 QPS/P99 数据，调优前后对比
- [ ] ASan/TSan 下长时间压测无报错
- [ ] `/predict` 在并发压测下 batching 生效（有 batch size 分布日志）
- [ ] 真实部署（截图 + curl 演示）
- [ ] 简历句：「基于 epoll ET + 主从 Reactor 实现多线程 HTTP 服务器，含线程池、异步日志与超时管理；实现推理服务的动态 batching 接口；wrk 压测 4 核 QPS ≥ XXk、P99 ≤ XX ms；ASan/TSan 无告警。」

**难度 ★★★ ｜ 参考**：陈硕《Linux 多线程服务器编程》+ muduo 源码（中文注释，强烈推荐读）、cpp-httplib。

---

### P3 · MiniTensor：张量库 + 自动微分 + 训练 MLP

**做什么**
- **Tensor**：`Tensor` 类（`dtype=float32`、shape、stride、数据存储 `std::vector<float>` 或自管内存）、广播、reshape/transpose/slice、与 Python 一样的基本运算。
- **算子**：`+ - * /`、matmul、exp/log、sum/mean/max（含 axis）、ReLU/Sigmoid/Tanh、Softmax、交叉熵；每个算子同时实现 CPU 版（先正确，后 SIMD/多线程优化）。
- **自动微分**：动态计算图（tape/节点 + 反向传播），实现 `backward()`；支持广播的反向（sum 到原 shape）；梯度检查（数值梯度 vs 解析梯度）。
- **训练闭环**：用 P1 的 jqlite 读配置；实现 SGD/Adam、mini-batch、训练 MLP 在 MNIST/Fashion-MNIST 上到 97%+ 准确率；存/加载权重（二进制格式 + 版本号）。
- **手写小 GPT（新增，基模核心）**：用自己写的 `Tensor`/autograd 实现 GPT/LLaMA 的最小版本（multi-head attention、RMSNorm、RoPE、SwiGLU、BPE tokenizer），在租卡（AutoDL 4090 几小时）或本机 8GB 上训练 30–60M 参数模型，语料用 TinyStories 或中文小语料；产出 loss 曲线、GPU 吞吐（tokens/s）和生成样例。**这是整个路线里最接近「基模」的一步。**
- **scaling 小实验（可选）**：3 个规模（5M / 10M / 20M）各训固定步数，画 loss 随参数量/数据量的曲线，验证 Chinchilla 的结论。
- **对照**：与 PyTorch 同结构模型对拍（初始化权重相同，前向/反向/一步更新后的参数误差 < 1e-5）。
- 进阶：`float16`/`bfloat16` 模拟、算子融合（ReLU+matmul）、多线程数据加载。

**为什么做 / 有什么用**
- AI infra 的**入口项目**：不自己写过 autograd，就永远看不懂 PyTorch 的 `Tensor` / `backward()` / `view` / 广播规则，也就写不好算子（P4）和推理引擎（P5）。
- 这是「C++ 面向对象 + 模板 + 内存管理 + 数值计算 + 数学」的集大成练习，比刷题更能证明你能写 AI 系统代码。
- 产物能继续长大：P4 把算子换成 CUDA kernel，P5 从中抽 Transformer 组件，P6 用它做数据并行实验。
- 简历上可以写「手写 autograd 框架」——面试 AI infra 岗时非常能聊。

**学到什么**
- 数值数组的内存布局：shape/stride、行主序、广播的 stride 技巧、零拷贝 view。
- 计算图与反向传播：拓扑排序、引用计数/所有权、tape vs 动态图。
- 模板与类型擦除：`Tensor` 如何同时支持 float/double/half，或者先固定 float32 保持简单。
- 数值稳定性：softmax 减最大值、log-sum-exp、交叉熵避免 log(0)、梯度检查。
- 性能：算子级 benchmark（GFLOPS）、内存分配次数、与 PyTorch（CPU）对比。
- 工程：二进制权重格式设计、版本兼容、测试金字塔（单算子 → 梯度 → 端到端训练）。

**里程碑（14 周）**
- W1–2 Tensor + 内存布局 + 基础算术 → W3 matmul（朴素 → 分块 → 多线程）
- W4–5 常用算子 + softmax/交叉熵 → W6 计算图 + backward
- W7–8 梯度检查 + 优化器（SGD/Adam）→ W9–10 训练 MLP 到 97%
- W11–12 权重存取 + 推理模式（no_grad）→ W13 与 PyTorch 对拍 + 性能数据
- W14 文档 + 架构图 + 复盘

**验收 / 简历亮点**
- [ ] `ctest` 全绿：算子测试 + 梯度检查（相对误差 < 1e-5）+ 训练收敛测试
- [ ] MNIST MLP 测试集准确率 ≥ 97%，训练曲线进 README
- [ ] 与 PyTorch 对拍：同权重下前向/反向误差 < 1e-5（有脚本）
- [ ] matmul 达到 CPU 单核 XX GFLOPS、8 线程 XX GFLOPS（对比 naive 的加速比）
- [ ] 简历句：「从零实现动态图自动微分张量库 MiniTensor，支持广播、matmul、Softmax/交叉熵与 Adam；梯度检查误差 < 1e-5，MNIST MLP 达 97.X%；matmul 多线程优化 XX× 加速；与 PyTorch 对拍验证正确性。」

**难度 ★★★★ ｜ 参考**：micrograd（Karpathy，200 行 autograd）、tinygrad、PyTorch `Tensor` 文档、《深度学习》花书第 6/8 章。

---

### P4 · CUDA 算子库与性能工程

**做什么**
- 环境：CUDA Toolkit 12.8+（Blackwell `sm_120`）、Nsight Compute（`ncu`）、Nsight Systems（`nsys`）、cuBLAS 作为对照。
- **Kernel 阶梯训练**（每个都写 naive → 优化版，记录带宽/算力利用率）：
  - vector add / reduction（warp shuffle、避免 bank conflict、合并访存）
  - **matmul**：naive → shared memory tiling → register blocking → 双缓冲（cp.async 可选）→ 分块大小调参
  - elementwise：exp/log/GELU/ReLU（结合 SIMD 访存、`__ldg`、向量化 float4）
  - softmax / layernorm / RMSNorm（行内 reduction + 数值稳定）
  - **训练侧 kernel（新增，基模最常用）**：Adam/AdamW 单步融合更新（fp32 master weights + fp16/bf16 计算）、梯度累加、bias + activation 融合、cross-entropy 融合、attention backward（可选）
  - **fused attention**（FlashAttention 简化版：tiling + online softmax，不要求超越原版）
  - 量化算子：int8 反量化 + matmul（Q8_0 风格），可选 int4
- **工程化**：算子库 API 设计（`Tensor` 复用 P3）、错误检查宏（`CUDA_CHECK`）、ASan/计算校验、基准测试（`cudaEvent` 计时，warmup，多次取中位数）。
- **性能分析**：用 `ncu` 看 roofline、occupancy、memory throughput、stall reasons；写进 README 的优化故事（这张图是简历/面试的核心素材）。
- 可选：CUDA Graph、streams 并发、半精度（`__half`/`bfloat16`）、对比 PyTorch 同算子延迟。

**为什么做 / 有什么用**
- AI infra 岗位**最直接的硬技能证明**：能写对、能写快、能用 profiler 讲清楚为什么快。
- 面试高频：shared memory 怎么用、bank conflict 怎么避免、occupancy 是什么、roofline 怎么读、online softmax 为什么数值稳定——全部在这个项目里踩一遍。
- 和 P3 配套：P3 是「框架骨架」，P4 是「底层发动机」，两个项目一起讲就是一个完整叙事。
- 8 GB 显存完全够：这些 kernel 的测试矩阵规模不需要大显存。

**学到什么**
- GPU 执行模型：SM/warp/thread block、寄存器与共享内存、occupancy、访存合并。
- 性能工程方法论：先测量再优化、roofline 判断瓶颈、逐层优化并保留每一步数据。
- 数值正确性：GPU 浮点归约的累积误差、`--use_fast_math` 的影响、与 CPU 参考实现对比的容差。
- CUDA 工程：CMake 集成、`sm_120` 架构编译、多文件 .cu 组织、错误处理。
- Profiling 工具：ncu/nsys 的指标解读。

**里程碑（14 周）**
- W1 环境 + vector add / reduction → W2–4 matmul 四级优化（每级一组数据）
- W5–6 elementwise + softmax → W7–8 layernorm/RMSNorm → W9–10 fused attention
- W11 量化 kernel（int8/int4 反量化 matmul）→ W12 ncu 全面分析 + 调参
- W13 与 cuBLAS/PyTorch 对比 + 测试 → W14 文档 + 博客

**验收 / 简历亮点**
- [ ] 所有 kernel 与 CPU 参考实现对比误差在容差内（相对误差 < 1e-4）
- [ ] matmul 达到显存带宽/峰值算力的 XX%（有 roofline 图），相对 naive kernel 加速 XX×，与 cuBLAS 差距 < X×
- [ ] attention kernel 在 seq_len=512、head_dim=64 下比 PyTorch eager 快 X×
- [ ] README 含 ncu 截图 + 逐级优化数据表
- [ ] 简历句：「实现 CUDA 算子库（matmul/softmax/layernorm/fused attention/int8 量化），matmul 经 4 级优化达到峰值算力 XX%、相对 naive 加速 XX×；用 Nsight Compute 做 roofline 分析并逐项调优；与 cuBLAS/PyTorch 对比验证。」

**难度 ★★★★ ｜ 参考**：《Programming Massively Parallel Processors》（PMPP，CUDA 圣经）、CUDA C++ Programming Guide、FlashAttention 论文、NVIDIA 官方 blog（matmul 优化系列）。

---

### P5 · MiniInfer：LLM 推理引擎

**做什么**
- **模型加载**：支持小型 LLaMA 架构模型（TinyLlama-1.1B / Qwen2.5-0.5B/1.5B），权重格式自定义（或简化 GGUF）；含 embedding、RMSNorm、RoPE、GQA attention、SwiGLU MLP。
- **Tokenizer**：BPE 解码（复用 P1 解析的 `tokenizer.json`），支持中文/英文/emoji 的基本往返。
- **推理执行**：prefill + decode 两阶段，KV cache、RoPE 位置编码、采样（greedy / temperature / top-k / top-p）、流式输出。
- **性能**：
  - 算子后端先用 P3/P4 的 kernel（CUDA matmul + softmax + layernorm）；FP16 推理；
  - 量化：Q8_0 权重（int8），对比 FP16 的显存/速度；
  - KV cache 优化：prealloc + paged 管理（可选）；
  - **continuous batching**：多请求并发调度，吞吐（tokens/s）对比单请求。
- **服务化**：用 P2 的 HTTP 服务器暴露 `/v1/completions`（OpenAI 兼容风格），支持流式 SSE。
- **对标**：与 `llama.cpp` 相同模型、相同硬件对比 tokens/s 与首 token 延迟；写清差距原因。
- 进阶：投机解码（draft model）、int4 量化（Q4_0）、投机采样、CUDA Graph。

**为什么做 / 有什么用**
- **这是 AI infra 方向最值钱的学生项目**：它把 P1（tokenizer JSON）、P2（HTTP/调度）、P3（张量/数值）、P4（算子）全部串起来，是一条完整的技术故事线。
- 面试可展开 30 分钟：KV cache 省了什么、prefill 和 decode 的计算特征差异（算力密集 vs 带宽密集）、continuous batching 怎么调度、量化怎么影响精度、为什么是 memory-bound。
- 8 GB 显存足够：0.5B/1.1B 模型 FP16 约占 1–2.2 GB，Q8 减半，能跑多并发小 batch。
- 简历上写「手写 LLM 推理引擎」的本科生极少，这是强差异化。

**学到什么**
- Transformer 推理的工程细节：RoPE、GQA、KV cache 布局、attention mask、sampling。
- 内存与带宽：decode 阶段是 memory-bound，优化重点从 FLOPS 转向「少读字节」。
- 调度：请求队列、batch 组装、KV cache 分页/复用、尾延迟控制。
- 量化：int8 对称量化、per-channel scale、反量化 kernel、精度评估（perplexity/生成质量）。
- 系统级 benchmark：tokens/s、TTFT（首 token 延迟）、显存占用、并发曲线。

**里程碑（14 周）**
- W1 模型结构 + 权重加载（先跑通随机权重）→ W2 RoPE/RMSNorm/GQA 单层前向
- W3 全模型前向 + 与 PyTorch 对拍 → W4 tokenizer + 文本生成（greedy）
- W5–6 KV cache + 采样 → W7–8 接 P4 CUDA kernel + FP16
- W9–10 量化 Q8 + 精度/速度对比 → W11–12 continuous batching
- W13 P2 HTTP 服务化 + SSE 流式 → W14 与 llama.cpp 对比 + 文档

**验收 / 简历亮点**
- [ ] 能生成连贯文本（示例进 README），与 PyTorch 同模型 logits 误差 < 1e-3
- [ ] 有 tokens/s、TTFT、显存占用、并发数曲线；与 llama.cpp 对比表
- [ ] Q8 量化后精度损失可量化（如 50 条 prompt 的人工/困惑度对比）
- [ ] HTTP 服务可被 curl 调用并流式返回
- [ ] 简历句：「从零实现 LLM 推理引擎 MiniInfer（LLaMA 架构），含 BPE tokenizer、KV cache、GQA、RoPE、int8 量化与 continuous batching；在 RTX 5060 上 Qwen2.5-0.5B 达到 XX tokens/s、TTFT XX ms；提供 OpenAI 风格流式 HTTP 接口。」

**难度 ★★★★★ ｜ 参考**：llama.cpp（重点读 `llama.cpp`/`ggml` 结构）、vLLM 论文（PagedAttention）、nanoGPT（Karpathy）、HuggingFace `transformers` 的 LLaMA 实现。

---

### P6 · 分布式训练 / 推理集群（或 RaftKV 备选）

**做什么（默认路线：分布式训练）**
- 基础：多进程训练（单机 2–4 进程模拟多卡/多机）+ TCP/gRPC 通信层（自研或 MPI/NCCL 概念模拟）。
- 并行：数据并行 + 梯度 allreduce（ring allreduce 自己实现），梯度累积，学习率缩放。
- 工程：checkpoint 保存/恢复、训练中断续跑、日志与指标、超参配置（复用 P1 的 jqlite）。
- 用 P3 的 MiniTensor 或 PyTorch 做小模型（MLP / 小 Transformer）在 MNIST/CIFAR 上验证：单卡 vs 多进程吞吐、收敛一致性。
- 可选进阶：ZeRO-1/2 式优化器状态分片、梯度压缩（fp16/量化通信）、弹性扩缩。
- **基模训练进阶（新增）**：ZeRO-1/2 参数与优化器状态分片、张量并行（列/行切分 + allreduce）、流水并行（micro-batch 调度）在 2–4 进程小模型上的最小实现；用 loss 曲线与 HellaSwag 小样本对比验证正确性。
- **备选路线（若想回到系统方向）**：RaftKV（原 P6，Raft 选举/日志复制/线性一致读），或「分布式推理服务」：多副本 + 负载均衡 + KV cache 感知路由（更有 AI 味，推荐）。

**为什么做 / 有什么用**
- 训练系统是 AI infra 的另一个高地（字节、阿里、华为的「训练框架/平台」岗），能讲清 allreduce 与并行策略的人很少。
- 单机模拟分布式同样能展示核心能力：通信、同步、容错、可观测性；面试不问你有几张卡，问你怎么设计。
- 作为最后一个项目，它天然把前面所有项目串成「一个 AI 系统的全栈」。

**学到什么**
- 分布式核心：集合通信、ring allreduce、参数服务器 vs 数据并行、一致性/收敛。
- 并发与容错：进程/线程模型、故障检测、checkpoint、重启恢复。
- 性能：通信开销占比、计算/通信重叠、带宽瓶颈定位。
- 可观测性：指标、日志、调试工具。

**里程碑（14 周）**
- W1–2 多进程训练骨架 + 数据切分 → W3–4 ring allreduce 实现 + 单测
- W5–6 训练一致性验证（单卡 vs 2/4 进程 loss 对齐）→ W7–8 checkpoint/恢复
- W9–10 性能分析（通信占比、scaling efficiency）→ W11–12 优化（重叠/压缩）
- W13–14 文档 + 与 PyTorch DDP 对比

**验收 / 简历亮点**
- [ ] 2/4 进程数据并行训练收敛曲线与单进程对齐（loss 差 < 1e-3）
- [ ] ring allreduce 带宽测试 + 单测（数值正确、任意进程数）
- [ ] 中途 kill 后从 checkpoint 恢复继续训练，曲线连续
- [ ] 有 scaling efficiency 数据（4 进程相对单进程）
- [ ] 简历句：「实现数据并行训练系统，自研 ring allreduce 集合通信与梯度累积；4 进程相对单进程 scaling efficiency XX%；支持 checkpoint 断点续训；与 PyTorch DDP 对比验证收敛一致性。」

**难度 ★★★★★ ｜ 参考**：PyTorch DDP 文档、Horovod、Ring Allreduce 论文（Baidu）、ZeRO 论文、MIT 6.824（若走 RaftKV 备选）。

---

## 4. 暑假小项目（2027.7–8，每个 1–3 周，任选 2–3 个）

| 小项目 | 做什么 | 用途 |
|---|---|---|
| CUDA matmul 阶梯 | naive → shared mem → register blocking，记录每级数据 | P4 的热身，开学前就有 GPU 手感 |
| pybind11 算子扩展 | 把 C++/CUDA 算子包成 Python 模块，`pip install` 可用 | AI 工程师日常，简历上「会写 Python 扩展」很实用 |
| **nanoGPT 复现** | 用 PyTorch 复现 Karpathy 的 nanoGPT，跑通训练 + 采样，逐模块理解 | P3「手写小 GPT」的精神预演，**基模方向必修** |
| **数据 pipeline** | 下载/整理 1–5 GB 中文语料，做清洗、去重、tokenize（BPE）、打包成训练样本 | 基模训练 70% 工作量在数据，面试高频 |
| **CS336 作业** | Stanford CS336（language modeling from scratch）前 3–4 次作业：tokenizer / Transformer / 训练 | 基模方向最对口的课程，做完 ≈ 半个 P3 |
| **租卡实验** | AutoDL 4090/A100 按小时租，训 30–60M 模型，记录吞吐、显存、loss | 证明你真的跑过训练，不只是写代码 |
| 线程池库 | 固定/动态线程池 + 任务队列 + future | P2/P3 数据加载与并行基础 |
| SIMD CPU 算子 | AVX2 实现向量加/dot/softmax，对比编译器自动向量化 | 没有 GPU 也能讲性能，面试常问 |
| micrograd C++ 版 | 200 行自动微分（标量） | 正式上 P3 前先走通 autograd 原理 |
| 无锁队列 | SPSC / MPMC ring buffer | P6 通信层基础 |

---

## 5. 贯穿全程的工程规范

**编译选项（Debug）**

```
-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror
-fsanitize=address,undefined          # 内存/未定义行为（host 代码）
-fsanitize=thread                     # 并发项目
-O0 -g
```

**CUDA 项目额外要求**
- `CMAKE_CUDA_ARCHITECTURES 120`（RTX 5060 是 Blackwell `sm_120`，必须 CUDA 12.8+；装错版本会报 `no kernel image is available`）；
- 每个 kernel 必须附 CPU 参考实现 + 容差测试（相对误差 < 1e-4，归约类放宽）；
- 性能数据用 `cudaEvent` 计时，固定 warmup 次数，取中位数，报告「相对 naive / 相对 cuBLAS」两组倍数；
- `ncu`/`nsys` 至少分析 3 个 kernel，截图进 README；
- 显存分配用 RAII 封装（`DeviceBuffer` 类），禁止裸 `cudaMalloc` 散落各处——这是 OI 选手最容易犯的错。

**CMake 规矩**：target-based（`target_link_libraries` / `target_include_directories`）；不用 `file(GLOB)`；`CMAKE_EXPORT_COMPILE_COMMANDS=ON` 给 clangd；依赖优先 `FetchContent`，系统库用 `find_package`。

**Python 规范（实验脚本与工具，§13.3 配套）**：依赖用 `uv`（或 venv + pip-tools）管理，`pyproject.toml` 锁定版本；`ruff` 做格式化与 lint，`mypy` 做类型检查（工具/数据代码必须过，训练脚本可放宽）；pytest 写单测（数据集切分、tokenizer、指标函数）；每个实验必须记录 config + seed + 结果（CSV 或 wandb）；所有训练脚本提供 `--dry-run` / `--max-steps`，能在 1 分钟内跑完小规模验证。

**测试**：GoogleTest + `ctest`；每个项目都要有「单元测试 + 集成测试」；关键项目加 fuzz / 对拍 / 故障注入；数值计算项目必须有「与参考实现对拍」的测试。

**性能**：Google Benchmark 出数据；Linux 用 `perf` + 火焰图；GPU 用 `ncu` + roofline；每个项目 README 必须有「优化前后」对比表。

**Git**：`main` 保护 + feature 分支 + PR（自己 review 自己）；Conventional Commits（`feat:` `fix:` `refactor:` `test:` `docs:`）；每个里程碑打 tag。

**CI**：GitHub Actions，ubuntu + windows 双平台矩阵，跑 `cmake` + `ctest` + sanitizer job；GPU 项目 CI 只跑 CPU 参考实现与编译检查（GitHub 免费 runner 没有 GPU），性能数据在本机跑并写进 README。

**文档**：README 固定结构——一句话简介、GIF/截图、架构图（mermaid）、Quick Start、Benchmark、设计取舍、Roadmap、License(MIT)；复杂项目加 `docs/design.md`。

**代码风格**：clang-format（Google 或 LLVM 风格）；clang-tidy 开 `modernize-*`、`bugprone-*`、`performance-*`、`cppcoreguidelines-*`。

---

## 6. 环境现状与待办（2026-10-23 实测）

**已装 / 已验证**
- WSL2 Ubuntu 24.04（`Ubuntu-24.04`，用户 `dym`，NOPASSWD sudo）
- g++ 13.3.0、clang 18.1.3、CMake 3.28.3、Ninja 1.11.1、git 2.43.0、clangd、gdb/lldb
- VS Code 1.142 + Remote-WSL + C/C++/CMake Tools/clangd 扩展
- GPU：RTX 5060 Laptop 8 GB，驱动 592.01，WSL 内 `nvidia-smi` 正常（`/dev/dxg` 存在）
- GoogleTest 1.14（`libgtest-dev`），ASan/UBSan 可用

**待装（按需，不用一次装完）**
- CUDA Toolkit 12.8+（阶段 0 周末装：`sudo apt-get install -y cuda-toolkit-12-8` 或 NVIDIA 官方 runfile）
- Nsight Compute / Nsight Systems（`ncu`、`nsys`）
- pybind11（暑假 2027）、PyTorch（CPU 版即可，作对照基线；GPU 版注意和驱动匹配）
- `nvidia-smi` 里显存 8151 MiB 中系统/WSL 占一部分，实际可用约 6.5–7 GB，项目按 ≤6 GB 设计

**省显存设置**（`%UserProfile%\.wslconfig`）：
```
[wsl2]
memory=8GB
processors=6
```

---

## 7. 学习资源（精选，不要贪多）

**书**
- 《C++ Primer》第 5 版——当参考，别通读
- 《Effective Modern C++》——必读，分主题读
- 《C++ Concurrency in Action》——P2 期间读前 4 章
- 《Linux 多线程服务器编程》（陈硕）——P2 配套
- **《Programming Massively Parallel Processors》(PMPP)** —— P4/CUDA 圣经
- 《深入理解计算机系统》（CSAPP）——贯穿全程
- 《深度学习》（花书）第 6/8 章 + 《动手学深度学习》(d2l.ai)——P3 数学/训练配套
- 《数据密集型应用系统设计》（DDIA）——P6/后端备选配套

**网站 / 视频**
- cppreference.com（字典）、learncpp.com（教材）、C++ Core Guidelines
- CUDA C++ Programming Guide、NVIDIA 官方 blog（matmul 优化系列）
- PyTorch 源码的 `Tensor`/`autograd` 文档、Karpathy 的 micrograd/nanoGPT 视频
- **CS336: Language Modeling from Scratch**（Stanford，基模方向最对口）、CS224N（NLP 基础）
- Karpathy《Let's build GPT: from scratch, in code》视频（实现一遍 Attention + GPT）
- CppCon 的 Back to Basics 系列（B 站有搬运）、Jason Turner 的 C++ Weekly

**论文（P4/P5 期间读，每篇写 1 页笔记）**

基模·架构与预训练：
- Attention Is All You Need（Transformer 原论文）
- GPT-2 / GPT-3（scaling 与 in-context learning）
- LLaMA 1 / 2（工程化架构：RMSNorm、RoPE、SwiGLU、GQA）
- Chinchilla（scaling laws，数据/参数配比）
- Switch Transformer 或 Mixtral（MoE）

基模·系统与推理：
- FlashAttention（I/O 感知的 attention）
- vLLM / PagedAttention（KV cache 分页）
- ZeRO（显存优化）、Megatron-LM（张量/流水并行）
- Ring Allreduce（百度）
- 量化：GPTQ / AWQ / llama.cpp 的 Q4_0 格式说明

基模·后训练（大三上读）：
- InstructGPT / RLHF、DPO（直接偏好优化）

**值得读源码**
- nlohmann/json（P1 对照）、fmtlib/fmt（现代 API 设计）、muduo（P2）
- micrograd / tinygrad（P3 自动微分）、PyTorch `ATen` 的算子组织
- llama.cpp 的 `ggml`（P4/P5）、cuBLAS 用法示例
- googletest（测试写法）

**比赛 / 开源**
- 开源之夏 OSPP（每年 5–9 月，大一大二可参加）：优先选 AI infra / 推理 / 编译器 / 数据库方向项目
- 华为软件精英挑战赛、昇腾/华为 AI 相关比赛（大二下–大三）
- 数据库大赛（OceanBase/PolarDB，大二–大三，若想做存储方向）
- GSoC（需 18+，大二后可尝试）
- 有 OI 底子的话，ICPC/CCPC 可打但要控制时间投入，别挤占项目

---

## 8. 里程碑检查表

- [ ] 2026.11 中旬：阶段 0 完成，CMake 模板工程跑通，10 个练习 commit，CUDA vector add 跑通
- [ ] 2027.1：P1 完成，GitHub 第一个仓库有 CI badge + benchmark + 博客 + tokenizer.json 解析 demo
- [ ] 2027.6：P2 完成（8 周版），有压测数据 + `/predict` batching + 部署
- [ ] 2027.8：暑假：**nanoGPT 复现跑通 + 租卡训练一个 30–60M 小模型（有 loss 曲线/生成样例）+ pybind11 扩展 + 至少 1 个开源 PR**
- [ ] 2028.1：P3 完成，MNIST 97%+，**手写小 GPT 训练出连贯文本**，与 PyTorch 对拍报告
- [ ] 2028.6：P4 完成，ncu roofline 图 + 与 cuBLAS 对比 + **Adam/attention kernel**
- [ ] 2028.8：简历定稿（P1/P3/P4/P5 主线 + 小 GPT 训练经历），开始准备 2029 暑期实习与科研夏令营/保研
- [ ] 2029.1：P5 完成，与 llama.cpp 对比数据 + 流式 HTTP 接口
- [ ] 2029.6：P6 完成（DDP + ring allreduce + ZeRO/张量并行最小实现）+ 暑期实习 offer

---

## 9. 我们的协作方式

1. **项目开始**：我先给需求文档 + 接口草案 + 第一周任务清单；你看完提问，确认后开工。
2. **你写代码**：卡住先自己查 20–30 分钟，再问我；我讲原理、给最小示例，**不直接给完整实现**。
3. **里程碑 review**：每个里程碑结束，我看你的 `git diff`，重点讲设计、现代 C++ 写法、性能与测试哪里能改。
4. **节奏**：每周 8–10 小时（2–3 次 × 2–3 小时）；每两周一个可运行 demo；每个项目结束写复盘博客。
5. **复盘模板**（每个项目完成后填）：做了什么 / 放弃或没做完什么 / 性能数据 / 最大的技术困难与解法 / 如果重写会怎么改 / 面试可能追问的 3 个问题。

---

## 10. 课程与项目对齐（顺手把课内学分也用上）

| 学期 | 专业课（预计） | 对齐项目 |
|---|---|---|
| 大一上 | 程序设计基础、高数 | 阶段 0 + P1 |
| 大一下 | 数据结构、线性代数、离散数学 | P2（并发）+ 线代支撑 P3/P4 |
| 大二上 | 计算机组成原理、操作系统、概率论 | P3（内存/数值/并发） |
| 大二下 | 计算机网络、机器学习、数据库（如有） | P4（CUDA/体系结构）+ P2 深化 |
| 大三上 | 深度学习、编译原理（如有）、软件工程 | P5（推理引擎）+ **CS336 自学 + 后训练论文** |
| 大三下 | 分布式系统（如有）/ 选修 / 实习 | P6 + 实习 / 科研 |

> 基模专项自学（不占学分）：CS336 全 10 讲 + CS224N 前半 + 每周论文笔记 + 每 2–3 个月租卡实验。研究线额外：线代/概率/优化教材过一遍，大二下开始联系导师进组。

---

## 11. 方向分支（到 2028 年再定，现在不用焦虑）

| 方向 | 必做 | 加分 | 建议 |
|---|---|---|---|
| **AI 基模 · 工程线（当前主线）** | P3（含小 GPT）、P4、P5、P6 | 开源 PR（llama.cpp / vLLM / Megatron-LM）+ 租卡训练实验 + CS336 | 训练系统 / 推理 / 算子岗；本科靠项目 + 实习入口，与 C++ 路线完全对齐 |
| **AI 基模 · 研究线** | P3 手写小 GPT + 线代/概率/优化 + 论文复现 2 篇 | 进组科研、Kaggle/复现报告、GPA、英语 | 目标读博/科研岗；C++ 只作工具，重心转实验设计与写作；大二下开始联系导师 |
| AI 算法 / 应用研究 | P1 + P3 + PyTorch 训练项目 | 论文复现、Kaggle | C++ 作为工具链，重心转 Python/实验；P5 可作为加分 |
| 后端 / 基础架构 | P2（+ 选项：MiniKV） | P6 RaftKV | 岗位最多、最稳；GPA/项目均衡即可 |
| 数据库内核（备选） | MiniKV（原 P3 的 LSM-Tree 存储引擎） | 分布式 KV | 需要系统课扎实，和 AI infra 二选一深化 |
| 图形 / 图形学（备选） | RayTracer（原 P4） | 光栅化渲染器 | 需要图形学课，和 AI 关系不大 |

> 想把原计划的存储/图形/编译器项目捡回来：MiniKV、RayTracer、MiniLang 的完整需求都还在 `ROADMAP-archive.md` 里（需要时我恢复给你）。

---

## 12. AI 系统技能清单（每完成一项打勾，面试前逐条自查）

- [ ] 能手写 CUDA kernel 并解释 warp / SM / occupancy / 访存合并
- [ ] 能读 ncu 的 roofline 图和 stall reason，判断 kernel 是算力还是带宽瓶颈
- [ ] 能手写 matmul 的 shared memory tiling，并说清每级优化带来多少收益
- [ ] 理解 Transformer 推理：prefill vs decode、KV cache、RoPE、GQA、attention mask
- [ ] 能做 int8 对称量化与反量化，并评估精度损失
- [ ] 懂 continuous batching 的调度目标与尾延迟权衡
- [ ] 能手写 autograd（计算图 + 反向传播 + 梯度检查）
- [ ] 理解数据并行 / ring allreduce / 梯度累积 / checkpoint
- [ ] 能用 pybind11 把 C++/CUDA 代码暴露给 Python
- [ ] 能读并复现一篇 AI 系统论文的核心 idea（至少 FlashAttention 或 PagedAttention）

基模专项（工程线 + 研究线都要）：
- [ ] 能从零实现 Transformer（attention/RMSNorm/RoPE/SwiGLU）并训练出会说话的 30–60M 模型，有 loss 曲线进 README
- [ ] 懂预训练数据 pipeline：清洗、去重、BPE tokenize、打包、shuffle，能说出每步对 loss 的影响
- [ ] 理解 scaling laws（Chinchilla），能设计 3 个规模的小实验验证趋势
- [ ] 懂后训练：SFT、DPO/RLHF 的目标函数与最小实现，知道和预训练的区别
- [ ] 能解释 RoPE / RMSNorm / GQA / MoE 各自解决什么问题、代价是什么
- [ ] 至少精读并复现过 1 篇基模论文（架构类或后训练类），有复现报告
- [ ] 能说清「预训练 / SFT / RLHF / 推理」四段各自的算力、数据、目标与评测方式

---

## 13. 多语言与工程工具技能矩阵（C++ 之外）· 2026-10-29 增补

**为什么加**：基模岗位的真实技术栈不是「只会 C++」。日常实验 90% 是 Python，10% 是 kernel/系统（CUDA/C++），再叠加 shell、SQL、Docker、HuggingFace 生态。只会 C++ 会被评价「系统底子好但做不了实验」，只会 Python 会被评价「调包侠」。目标是**两头都能干活**。

### 13.1 语言分工（这张表是核心）

| 语言/工具 | 在岗位里的角色 | 目标水平 | 对应阶段 |
|---|---|---|---|
| C++20 | 训练/推理系统、算子框架、性能关键路径 | 主线（已在做） | P1–P6 |
| CUDA / Triton | kernel 实现与调优 | 能写能调 | P4、暑假 2027 |
| **Python** | 实验、训练脚本、数据处理、评测、工具链 | **工程级**（不止会语法） | 贯穿全程，见 13.2 |
| Shell / Linux | 环境、进程、日志、ssh/tmux、监控 | 熟练 | 全程 |
| SQL | 训练数据统计与采样分析 | 会用 | 大二下起 |
| Docker | 环境复现、CI、算子容器 | 会用 | P4 前 |
| Git / CI | 版本、协作、自动测试 | 熟练 | 全程 |
| （可选）Rust / Mojo | 高性能/新语言尝鲜 | 了解 | 大三 |

### 13.2 Python 五级能力（面试前逐条自查）

- **L1 语言本身**：dataclass、typing（Optional/Union/Sequence/Protocol）、生成器、上下文管理器、装饰器、异常链、async/await 基础、pathlib、argparse。
- **L2 工程化**：uv/venv、pyproject.toml、pytest（fixture/parametrize/mock）、ruff、mypy、logging、配置管理、CLI 工具。
- **L3 数值与数据**：numpy（广播、向量化、内存布局）、PyTorch（Tensor/autograd/Dataset/DataLoader/optimizer/checkpoint）、tokenizers/datasets、pandas、流式读大 JSONL。
- **L4 性能与调试**：cProfile/py-spy、torch.profiler、nvidia-smi、OOM 排查、multiprocessing、num_workers。
- **L5 生态与可复现实验**：HF transformers（加载/生成）、wandb/tensorboard、torchrun/DDP、vLLM 客户端；每个实验都有 config + seed + 结果表。

### 13.3 Python 训练安排（不占主线项目时间）

| 时间 | 内容 | 产出 |
|---|---|---|
| 2026.11–12（与 P1 并行，每周 2–3h） | L1+L2：Python 重写几道 OI 题；搭 ruff+mypy+pytest 模板仓库；写 3 个 CLI 小工具 | `python-lab` 仓库（10 练习 + CI） |
| 2027.3–6（与 P2 并行，每周 2h） | L3：numpy 向量化练习、JSON→统计报表脚本、pytest 覆盖 | 练习 + 小报告 |
| 2027.7–8（暑假主任务） | L3+L4+L5：nanoGPT 复现、数据 pipeline、pybind11、wandb | 训练出小模型 + 实验报告 |
| 2027.9–2028.1（与 P3 并行） | 用 PyTorch 写同结构模型做 P3 对拍基线；训练脚本（config/checkpoint） | 对拍报告 + 脚本 |
| 2028.3–6（与 P4 并行） | torch.profiler + ncu 对比同一算子；Python 调用 CUDA 的 benchmark harness | benchmark 报告 |
| 2028.9–2029.1（与 P5 并行） | vLLM 客户端 vs 自己的引擎做同 prompt 评测 | 对比表 |
| 2029.3–6（与 P6 并行） | torchrun/DDP 训练对照 | 实验报告 |

**验收**：不看 IDE 能写完 300 行训练脚本；能独立走「数据→训练→评测→记录→复现」；能改 HF 官方 example；`ruff + mypy` 全绿。

### 13.4 语言之外、真正卡人的三件事

1. **读论文 → 实现**：每周 1 篇，写 1 页笔记（问题/方法/代价/能否复现）。
2. **实验设计**：控制变量、ablation、多 seed、报告均值±方差、结果表格化。
3. **调试 GPU**：OOM、NaN loss、loss 不降、吞吐只有预期 1/10 的排查套路（nvidia-smi → 缩小规模单步 → 与 PyTorch 逐层对比）。

### 13.5 一句话原则

> **C++ 让你写得快，Python 让你做得对，CUDA 让你跑得动。** 现阶段比例大致 C++ 6 : Python 3 : CUDA 1；大二下之后 C++ 与 Python 各半；实习前 CUDA 占比上升。

---

*本文随进展更新；每次完成项目后回到这里回填数据与复盘。2026-10-23 按 AI 系统方向调整；2026-10-29 按 AI 基模方向再调整（双轨、P2 压缩、P3 加小 GPT、P4 加训练 kernel、暑假 2027 改 nanoGPT+租卡、P6 聚焦分布式训练、新增 §13 多语言技能矩阵）。*
