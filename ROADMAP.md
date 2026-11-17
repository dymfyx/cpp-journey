# 我的现代 C++ 三年计划（大一上 → 大三下）

> 我，中山大学 2026 级人工智能专业。高中有信息学竞赛背景，算法和调试还行，但没写过工程代码：不会面向对象，不懂 CMake，没写过测试，没碰过 Linux 和 GPU。
>
> 这份文档是我自己排的三年路线，写给未来的我看。最初在 2026-10-20 定下来，10-23 按 AI 系统方向改过一版，10-29 决定往「AI 基模」（预训练 / 训练系统 / 后训练 / 推理）方向走，又改了一版。每做完一个项目我会回来补数据、写复盘，计划本身也会随进展滚动调整。

## 关于我现在的条件

- **有**：算法、数据结构、调试、读别人代码的能力，C 语法，STL 常用容器。
- **缺**：对象模型、RAII、move 语义、智能指针、模板、异常处理、并发、CMake/测试/CI、Linux 工程环境、CUDA。
- **硬件**：笔记本一块 RTX 5060 Laptop，8 GB 显存，WSL2 里 `nvidia-smi` 能直通。
  - 8 GB 能干什么我心里有数：0.5B–3B 的量化模型能推理，30M–124M 的小模型能训练，写 kernel 完全够用；7B 全参训练不用想。真要做基模实验，按小时租云 GPU（AutoDL 之类，一天几十块）。
- **时间**：每周 8–10 小时，考试周会掉到 3–4 小时。所以计划按「学期」排，不按周排，留出缓冲。

## 0. 我想达到什么

1. 手上有 3–4 个能讲 30 分钟的项目。基模方向最值钱的是「**从零训过一个小模型 + 自己写过训练/推理系统**」，所以 P3 的自动微分 + 手写小 GPT、P4 的 CUDA 算子、P5 的推理引擎、P6 的分布式训练是重点，P1/P2 是底座。
2. 能独立走完一条链路：需求 → 接口 → 实现 → 测试 → 性能分析（含 GPU profiler）→ 文档。
3. 工具链用熟：CMake / Ninja / clangd / gdb / sanitizers / GoogleTest / Google Benchmark / CUDA Toolkit / Nsight / PyTorch / GitHub Actions。
4. 数学和论文别落下：线代、概率、优化要扎实，至少精读并复现 2 篇基模论文（比如 FlashAttention、Chinchilla/DPO 二选一）。
5. 大三上的寒假（2029.1–2）拿这份简历去投 2029 暑期实习（基模训练系统 / 推理 / 算子方向），同时准备科研夏令营和保研材料。

### 我给自己定的三条规矩

1. 不写 `#include <bits/stdc++.h>`，不写 800 行单文件，不拿全局变量当状态。工程代码就是接口 + 实现 + 测试 + 构建。
2. 先想清楚资源的生命周期（谁分配、谁释放、失败了怎么办），再想算法。我现在觉得这就是 OOP 和 RAII 的全部意义。
3. 性能必须能度量。每个项目都要有 benchmark 和「优化前 / 优化后」两张数字。GPU 项目看的是 tokens/s、FLOPS、显存带宽利用率、相对 PyTorch/cuBLAS 的倍数，不是「感觉快了」。

## 1. 整体路线

| 阶段 | 时间 | 做什么 | 难度 | 技术 | 我图它什么 |
|---|---|---|---|---|---|
| 阶段 0 | 2026.10–11（4 周） | 工具链 + 现代 C++ + GPU 环境 | ★ | C++17/20、CUDA 环境 | 地基 |
| P1 | 2026.11–2027.1（10 周） | jq-lite：JSON 解析库 + 查询 CLI | ★★ | C++17/20 | 通用 C++ 基本功，顺带解析 tokenizer.json |
| P2 | 2027.3–6（8 周，压缩） | WebServer：epoll 多线程 HTTP + 推理服务接口 | ★★★ | C++20 | serving 底座，P5 直接用 |
| 暑假 2027 | 2027.7–8 | nanoGPT 复现 + 租卡训小模型 + pybind11 | ★★★ | PyTorch、CUDA 12.8+ | 第一个「真训过模型」的作品 |
| P3 | 2027.9–2028.1（14 周） | MiniTensor：张量 + autograd + 手写小 GPT 并训练 | ★★★★ | C++20 | 基模训练入门王牌 |
| P4 | 2028.3–6（14 周） | CUDA 算子库：matmul / attention / layernorm / Adam / 量化 | ★★★★ | CUDA、C++20 | 算子硬技能 |
| 暑假 2028 | 2028.7–8 | 实习 / 论文复现（FlashAttention、DPO）+ 租卡实验 | — | — | 简历定稿 |
| P5 | 2028.9–2029.1（14 周） | MiniInfer：LLM 推理引擎（KV cache / 量化 / batching） | ★★★★★ | C++20 + CUDA | 我最值钱的项目 |
| P6 | 2029.3–6（14 周） | 分布式训练（DDP → ZeRO / 张量并行 → 小模型多卡） | ★★★★★ | C++20 + NCCL | 基模系统核心加分 |

**如果时间不够，我就保这些**：P1 + P3（含小 GPT）+ P5 做深；P4 至少完成 matmul + softmax/layernorm + attention 三个 kernel；P2 压到 6–8 周；P6 只做 DDP + ring allreduce。宁可 3 个讲得透，不要 6 个都半成品。

**贯穿全程、每周固定 2–3 小时（不占项目时间）**：CS336 / CS224N 自学、每周一篇论文笔记、每 2–3 个月一次租卡实验、Python 工程训练（见 §13）。这些决定我「像不像做基模的人」，项目只是载体。

**和上一版的区别**：上一版是泛 AI infra，这一版转向基模——P2 压成服务底座；P3 加了「手写小 GPT + 训练」；P4 加了训练侧 kernel（Adam 融合）和 attention；暑假 2027 从「CUDA 入门」改成「nanoGPT 复现 + 租卡训练」；P6 聚焦训练系统。原来想做的存储/图形/编译器项目都在 `ROADMAP-archive.md` 里存着。

## 2. 阶段 0：工具链和现代 C++（4 周，已完成）

直接上项目会卡在「编译不过」「不会组织代码」「看不懂报错」。所以我先花 4 周把地基打好。

### 2.1 环境（2026-10-20 起陆续配好）

| 项 | 状态 |
|---|---|
| 主环境 | ✅ WSL2 + Ubuntu 24.04 + VS Code Remote-WSL |
| 编译器 | ✅ gcc 13.3（主用）/ clang 18.1.3 |
| 构建 | ✅ CMake 3.28 + Ninja 1.11 |
| 调试 | ✅ gdb + VS Code 调试面板 |
| 语言服务 | ✅ clangd（配 `compile_commands.json`） |
| GPU | ✅ RTX 5060 Laptop 8 GB，WSL 内 `nvidia-smi` 可见 |
| CUDA Toolkit | ⬜ 待装（Blackwell 要 CUDA 12.8+，准备 13.x） |
| pybind11 / PyTorch | ⬜ 暑假 2027 前装 |

选 WSL2 不选纯 Windows，是因为 MinGW 没有 sanitizers，P2 要 epoll，perf/ncu 只在 Linux 上顺，而且以后实习环境也是 Linux。

### 2.2 语言清单（每个主题配一个练习）

| 主题 | 要搞懂的点 | 练习 | 状态 |
|---|---|---|---|
| 类 / 构造 / 析构 / 封装 | 不变量、`const` 成员函数、初始化列表、`explicit` | RAII `File` 类 | ✅ p0-01 |
| 拷贝语义 | 拷贝构造、拷贝赋值、自赋值、异常安全 | `Buffer` 类 | ✅ p0-02 |
| 移动语义 | 右值引用、`std::move`、`noexcept`、rule of 0/5 | 给 `Buffer` 加移动，放进 `vector` | ✅ 已做 |
| 智能指针 | `unique_ptr` / `shared_ptr` / `weak_ptr`、循环引用 | 用 `unique_ptr` 写链表和二叉树 | ⬜ |
| 模板 | 函数/类模板、特化、C++20 concepts | `Matrix<T>`（P3 张量雏形） | ⬜ |
| STL | 迭代器、`<algorithm>`、ranges 初探 | 用算法重写 3 道 OI 题 | ⬜ |
| 错误处理 | 异常 vs `expected` / `optional`、`noexcept` | 把返回 `-1` 的接口改成 `expected` | ⬜ |
| 现代工具箱 | `optional` / `variant` / `string_view` / `span` / `format` | 用 `variant` 写状态机 | ⬜ |
| 编译与链接 | 头文件、ODR、`inline`、`static`、声明与定义分离 | 把单文件拆成 3 个 .cpp + 头文件 | ⬜ |
| GPU 最小闭环 | `nvidia-smi`、`nvcc`、kernel 语法、grid/block、统一内存 | 写 vector add kernel，和 CPU 版比 100 万向量 | ⬜ 周末做 |

### 2.3 我的 CMake 模板

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

CUDA 项目（P4 / 暑假用，阶段 0 先知道怎么写）：

```cmake
project(kernels LANGUAGES CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES 120)   # RTX 5060 = Blackwell sm_120
add_library(kernels src/gemm.cu)
target_compile_options(kernels PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math>)
```

**验收**：不看教程能建出这个工程，`cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build` 全绿，能下断点，能开 `-fsanitize=address,undefined` 跑测试。

### 2.4 阶段 0 读什么

- learncpp.com：从 Ch.1 读到「类」部分，当教材。
- cppreference.com：当字典查，别通读。
- 《Effective Modern C++》条款 1–17（类型推导、auto、decltype、const）。

**验收**：上面 10 个练习自己写出来并 commit；能解释「为什么 `unique_ptr` 不能拷贝只能移动」；能跑通 vector add kernel。

## 3. 项目

### P1 · jq-lite：从零写 JSON 库 + 查询 CLI

**状态：✅ 完成（2026-11-16，10 周实际压缩到 4 周多）**

**做了什么**

- 库：RFC 8259 解析/序列化。DOM 是 `std::variant`（null/bool/number/string/array/object），解析错误带行列号，`\uXXXX` 转 UTF-8，数字用 `std::from_chars` + 严格文法，序列化用 `std::to_chars`。
- CLI：`jq_lite [-c] [-r] <路径> [文件]`，jq 风格路径 `.a.b[0]`，支持 stdin，退出码 0/1/2。
- 扩展：RFC 6901 JSON Pointer（`/a~1b/~0key/1`）、`ParseOptions{max_depth}`（默认 512 层）、JSONTestSuite 官方测试集接入。
- 工程：CMake `install()` + `export` + `find_package(jq-lite)`、GoogleTest、fuzz 驱动、Google Benchmark、与 nlohmann/json 对拍、GitHub Actions 双配置 CI、`docs/design.md`。

**实测数据（Release，1 MB 文档，10 轮 median）**

| 项 | 我的实现 | nlohmann/json |
|---|---|---|
| 解析吞吐 | 133.9 MiB/s | 63.9 MiB/s（快 2.1×） |
| 紧凑序列化 | 245.8 MiB/s | 240.2 MiB/s |
| 缩进序列化 | 162.5 MiB/s | 173.4 MiB/s |
| 单测 | 68 个（全部通过） | — |
| JSONTestSuite | y_ 95/95、n_ 188/188 | — |
| fuzz | 10 万次随机/破坏输入零崩溃 | — |

数字只做同场对比：笔记本睿频/降频会让同一份代码跨会话差 30–40%。`to_chars` 那次优化把紧凑序列化从 162 提到 246 MiB/s，是我第一次「用 profiler + 原理」而不是「凭感觉」做优化。

**为什么做、有什么用**

- JSON 到处都是：`config.json`、`tokenizer.json`、`generation_config.json`、API 返回体。自己写一遍，以后用 nlohmann/rapidjson 时知道底层在干什么。
- 它一次覆盖「对象建模 + 资源管理 + 错误处理 + 性能」四个主题，最适合当第一个项目。
- 产出的 `jq_lite` 现在真的在当我自己的命令行工具用（格式化、取字段），P3/P5 的配置和 tokenizer 解析也靠它。

**学到什么**

- 同一份 DOM，对比了「继承 + 虚函数」和 `std::variant` 两种建模，最后选了 variant：`sizeof(Value) = 56`，值语义、可拷贝可比较、无虚表。
- 递归下降、错误行列号、UTF-8 编码、`\u` 代理对、数字严格文法。
- 资源与异常：构造失败如何不泄漏、`noexcept` 纪律、栈展开时的析构顺序。
- 测试金字塔：单测 → fuzz → 官方测试集 → 与参考实现对拍，四层各抓不同问题。
- 工程化：CMake install/export、`find_package`、CI 上跑 sanitizer。

**踩过的坑（都写进 README 了）**

- `%.17g` 保往返但输出太长，换成 `std::to_chars` 后序列化快 50%+；
- 深度限制必须用 RAII 守卫在构造时回滚，否则计数错乱；
- 删掉看起来「冗余」的边界检查，单点删不报错（被别的检查接住了），两个一起删才被 ASan 抓到越界——这个实验后来成了 design.md 的素材；
- Debug+ASan 编出来的静态库给别人链接会缺 `__asan_*` 符号，install 必须用 Release。

**重看会怎么改**

- 数字格式化可以更进一步用 Dragonbox 类算法；
- 对象用 `std::map` 保序但慢，可以换「有序 vector + 查找」；
- 大文件应该走 SAX 流式，现在只支持全量建树。

### P2 · WebServer：epoll 多线程 HTTP/1.1 + 推理服务接口

> 基模方向下这个项目压到 8 周：去掉协程版和部分进阶，只留「epoll + Reactor + 线程池 + 异步日志 + 静态文件 + `/predict` 动态 batching」主线。它只是 P5 的服务底座，不是重点。

**做什么**

- 网络层：非阻塞 socket + epoll（ET）+ 主从 Reactor（主线程 accept，从线程处理读写）。
- HTTP 层：请求行/头部状态机、Content-Length、keep-alive、chunked（可选）、静态文件（sendfile/mmap）、简单路由。
- 基础设施：固定大小线程池（任务队列 + 条件变量）、异步日志（多生产者单消费者 + 双缓冲）、连接超时定时器（小根堆或时间轮）。
- AI 收尾（2 周）：`POST /predict`，后端先接「假模型」或 P3 的 MLP，做**动态 batching**（攒 5ms 成一批）和并发限流；P5 换成真 LLM。
- 收尾：优雅关闭、`wrk` 压测报告、部署到云服务器或树莓派。

**为什么做、有什么用**

- 后端方向最经典、面试问得最细的项目：QPS、P99、连接数、惊群、内存都能展开。
- 对 AI 方向的意义：所有推理服务（Triton、vLLM server、TGI）都是「HTTP/gRPC 前端 + 调度 + 批处理 + 后端模型」，我现在写的就是这个骨架。
- 能真跑、能压测、能部署，简历上放数据、面试官能访问。
- 顺带补上 Linux 系统编程（fd、非阻塞 IO、信号、线程）和并发，这两块 OI 完全没碰。

**学到什么**

- 接口设计：`HttpHandler` / `Connection` / `EventLoop` 的边界。
- RAII 封装：socket fd、epoll 注册/注销、线程 join，每个资源绑生命周期。
- 并发：`thread`/`mutex`/`condition_variable`/`atomic`、内存序入门、死锁与数据竞争、TSan。
- 网络：TCP 粘包/半包、IO 多路复用、ET vs LT、心跳超时。
- 工程：curl 集成测试、压测方法论、火焰图定位热点。
- 服务化：请求队列、batching 延迟与吞吐的权衡。

**里程碑（8 周）**

- W1–2 socket 封装 + epoll echo → W3 HTTP 解析器 → W4 静态文件 + 路由
- W5–6 线程池 + reactor → W7 `/predict` + 动态 batching → W8 压测 + 文档

**验收**

- [ ] 单机 4 核 `wrk -t4 -c1000 -d30s` 有 QPS/P99，调优前后对比
- [ ] ASan/TSan 下长时间压测无报错
- [ ] `/predict` 并发下 batching 生效（有 batch size 分布日志）
- [ ] 真部署（截图 + curl 演示）
- [ ] 简历句：「基于 epoll ET + 主从 Reactor 实现多线程 HTTP 服务器，含线程池、异步日志与超时管理；实现推理服务动态 batching 接口；wrk 4 核 QPS ≥ XXk、P99 ≤ XX ms；ASan/TSan 无告警。」

**难度 ★★★ ｜ 参考**：陈硕《Linux 多线程服务器编程》、muduo 源码、cpp-httplib。

### P3 · MiniTensor：张量 + 自动微分 + 手写小 GPT

**做什么**

- **Tensor**：`Tensor` 类（float32、shape、stride、自管内存）、广播、reshape/transpose/slice。
- **算子**：`+ - * /`、matmul、exp/log、sum/mean/max（含 axis）、ReLU/Sigmoid/Tanh、Softmax、交叉熵；先写对的 CPU 版，再 SIMD/多线程优化。
- **自动微分**：动态计算图 + `backward()`，支持广播的反向（sum 回原 shape），梯度检查（数值 vs 解析）。
- **训练闭环**：用 P1 读配置，SGD/Adam、mini-batch，MLP 在 MNIST 上到 97%+，权重二进制存取（带版本号）。
- **手写小 GPT**：用自己写的 Tensor/autograd 实现 GPT 最小版（multi-head attention、RMSNorm、RoPE、SwiGLU、BPE tokenizer），在租卡或本机 8GB 上训 30–60M 模型，语料用 TinyStories 或中文小语料；产出 loss 曲线、tokens/s、生成样例。**这是整条路线里最接近「基模」的一步。**
- **scaling 小实验（可选）**：5M / 10M / 20M 三个规模各训固定步数，画 loss 曲线验证 Chinchilla 结论。
- **对拍**：与 PyTorch 同结构模型比，初始化权重相同，前向/反向/一步更新误差 < 1e-5。

**为什么做、有什么用**

- 不自己写过 autograd，就永远看不懂 PyTorch 的 `Tensor`/`backward()`/广播规则，也写不好算子（P4）和推理引擎（P5）。
- 是「C++ OOP + 模板 + 内存 + 数值 + 数学」的集大成练习，比刷题更能证明我能写 AI 系统代码。
- 产物能继续长大：P4 把算子换 CUDA，P5 抽 Transformer 组件，P6 拿它做数据并行实验。

**学到什么**

- 内存布局：shape/stride、行主序、广播的 stride 技巧、零拷贝 view。
- 计算图：拓扑排序、所有权、tape vs 动态图。
- 类型擦除：一个 Tensor 同时支持 float/double/half，或者先固定 float32 保持简单。
- 数值稳定性：softmax 减最大值、log-sum-exp、交叉熵防 log(0)、梯度检查。
- 性能：算子 GFLOPS、分配次数、与 PyTorch（CPU）对比。
- 工程：二进制权重格式与版本兼容、测试金字塔（单算子 → 梯度 → 端到端训练）。

**里程碑（14 周）**

- W1–2 Tensor + 布局 + 基础算术 → W3 matmul（朴素 → 分块 → 多线程）
- W4–5 常用算子 + softmax/交叉熵 → W6 计算图 + backward
- W7–8 梯度检查 + 优化器 → W9–10 训练 MLP 到 97%
- W11–12 权重存取 + 推理模式（no_grad）→ W13 与 PyTorch 对拍 + 性能
- W14 文档 + 架构图 + 复盘

**验收**

- [ ] ctest 全绿：算子 + 梯度检查（相对误差 < 1e-5）+ 收敛测试
- [ ] MNIST MLP ≥ 97%，训练曲线进 README
- [ ] 与 PyTorch 对拍误差 < 1e-5（有脚本）
- [ ] matmul 单核/8 线程 GFLOPS + 相对 naive 加速比
- [ ] 简历句：「从零实现动态图自动微分张量库 MiniTensor，支持广播、matmul、Softmax/交叉熵与 Adam；梯度检查误差 < 1e-5，MNIST MLP 达 97.X%；matmul 多线程 XX× 加速；与 PyTorch 对拍验证。」

**难度 ★★★★ ｜ 参考**：micrograd、tinygrad、PyTorch Tensor 文档、《深度学习》花书第 6/8 章。

### P4 · CUDA 算子库

**做什么**

- 环境：CUDA Toolkit 12.8+（`sm_120`）、`ncu`、`nsys`、cuBLAS 对照。
- Kernel 阶梯（每个都写 naive → 优化版，记录带宽/算力利用率）：
  - vector add / reduction（warp shuffle、bank conflict、合并访存）
  - matmul：naive → shared memory tiling → register blocking → 双缓冲 → 调参
  - elementwise：exp/log/GELU/ReLU（`__ldg`、float4 向量化）
  - softmax / layernorm / RMSNorm（行内 reduction + 数值稳定）
  - 训练侧 kernel：Adam/AdamW 融合更新、梯度累加、bias+activation 融合、cross-entropy 融合
  - fused attention（FlashAttention 简化版：tiling + online softmax）
  - 量化：int8 反量化 + matmul（Q8_0 风格），可选 int4
- 工程化：算子库 API（复用 P3 的 Tensor）、`CUDA_CHECK` 宏、数值校验、`cudaEvent` 计时（warmup + 中位数）。
- 性能分析：`ncu` 看 roofline / occupancy / memory throughput / stall reason，写进 README。

**为什么做、有什么用**

- AI infra 最直接的硬技能证明：写对、写快、能用 profiler 讲清为什么快。
- 面试高频都在这里踩一遍：shared memory、bank conflict、occupancy、roofline、online softmax 为什么稳。
- 和 P3 配套：P3 是框架骨架，P4 是底层发动机，两个一起讲是完整叙事。
- 8 GB 显存完全够，测试矩阵不需要大显存。

**学到什么**

- GPU 执行模型：SM/warp/block、寄存器与共享内存、occupancy、访存合并。
- 性能工程：先测量再优化、roofline 判断瓶颈、逐级优化保留每一步数据。
- 数值正确性：GPU 浮点归约误差、`--use_fast_math` 的影响、与 CPU 参考的容差。
- CUDA 工程：CMake 集成、`sm_120` 编译、多文件 .cu 组织、错误处理。
- Profiling：ncu/nsys 指标解读。

**里程碑（14 周）**

- W1 环境 + vector add/reduction → W2–4 matmul 四级优化（每级一组数据）
- W5–6 elementwise + softmax → W7–8 layernorm/RMSNorm → W9–10 fused attention
- W11 量化 kernel → W12 ncu 全面分析 + 调参
- W13 与 cuBLAS/PyTorch 对比 → W14 文档 + 博客

**验收**

- [ ] 所有 kernel 与 CPU 参考误差在容差内（< 1e-4）
- [ ] matmul 达到峰值算力/带宽的 XX%（有 roofline 图），相对 naive 加速 XX×，与 cuBLAS 差距 < X×
- [ ] attention 在 seq_len=512、head_dim=64 下比 PyTorch eager 快 X×
- [ ] README 含 ncu 截图 + 逐级优化表
- [ ] 简历句：「实现 CUDA 算子库（matmul/softmax/layernorm/fused attention/int8 量化），matmul 经 4 级优化达到峰值算力 XX%、相对 naive 加速 XX×；用 Nsight Compute 做 roofline 分析并调优；与 cuBLAS/PyTorch 对比验证。」

**难度 ★★★★ ｜ 参考**：PMPP（CUDA 圣经）、CUDA C++ Programming Guide、FlashAttention 论文、NVIDIA 官方 matmul 优化 blog。

### P5 · MiniInfer：LLM 推理引擎

**做什么**

- 模型加载：小 LLaMA 架构（TinyLlama-1.1B / Qwen2.5-0.5B/1.5B），自定义权重格式（或简化 GGUF）；embedding、RMSNorm、RoPE、GQA、SwiGLU。
- Tokenizer：BPE 解码（复用 P1 解析的 `tokenizer.json`），中英文/emoji 基本往返。
- 推理：prefill + decode 两阶段、KV cache、采样（greedy / temperature / top-k / top-p）、流式输出。
- 性能：算子用 P3/P4 的 kernel（CUDA matmul + softmax + layernorm），FP16；Q8_0 量化对比；KV cache prealloc + paged（可选）；continuous batching。
- 服务化：用 P2 的 HTTP 服务器暴露 `/v1/completions`（OpenAI 兼容风格），支持 SSE 流式。
- 对标：与 llama.cpp 同模型同硬件比 tokens/s 与首 token 延迟，写清差距原因。

**为什么做、有什么用**

- AI infra 方向最值钱的学生项目：P1（tokenizer JSON）、P2（HTTP/调度）、P3（张量/数值）、P4（算子）全串起来。
- 面试能聊 30 分钟：KV cache 省了什么、prefill 和 decode 的计算特征差异、continuous batching 怎么调度、量化怎么影响精度、为什么 decode 是 memory-bound。
- 8 GB 显存够：0.5B/1.1B FP16 占 1–2.2 GB，Q8 减半，能跑多并发小 batch。

**学到什么**

- Transformer 推理工程细节：RoPE、GQA、KV cache 布局、attention mask、sampling。
- 内存与带宽：decode 是 memory-bound，优化重点从 FLOPS 转到「少读字节」。
- 调度：请求队列、batch 组装、KV cache 分页/复用、尾延迟控制。
- 量化：int8 对称量化、per-channel scale、反量化 kernel、精度评估。
- 系统级 benchmark：tokens/s、TTFT、显存、并发曲线。

**里程碑（14 周）**

- W1 模型结构 + 权重加载（随机权重先跑通）→ W2 RoPE/RMSNorm/GQA 单层前向
- W3 全模型前向 + 与 PyTorch 对拍 → W4 tokenizer + greedy 生成
- W5–6 KV cache + 采样 → W7–8 接 CUDA kernel + FP16
- W9–10 Q8 量化 + 精度/速度对比 → W11–12 continuous batching
- W13 P2 HTTP 服务化 + SSE → W14 与 llama.cpp 对比 + 文档

**验收**

- [ ] 能生成连贯文本（示例进 README），与 PyTorch logits 误差 < 1e-3
- [ ] tokens/s、TTFT、显存、并发曲线；与 llama.cpp 对比表
- [ ] Q8 量化精度损失可量化（50 条 prompt 困惑度/人工对比）
- [ ] HTTP 服务可 curl 调用并流式返回
- [ ] 简历句：「从零实现 LLM 推理引擎 MiniInfer（LLaMA 架构），含 BPE tokenizer、KV cache、GQA、RoPE、int8 量化与 continuous batching；RTX 5060 上 Qwen2.5-0.5B 达 XX tokens/s、TTFT XX ms；提供 OpenAI 风格流式 HTTP 接口。」

**难度 ★★★★★ ｜ 参考**：llama.cpp / ggml、vLLM（PagedAttention）、nanoGPT、HF transformers 的 LLaMA 实现。

### P6 · 分布式训练（备选：分布式推理 / RaftKV）

**做什么（默认：分布式训练）**

- 多进程训练（单机 2–4 进程模拟多卡/多机）+ TCP/gRPC 通信层。
- 数据并行 + 梯度 allreduce（自己实现 ring allreduce）、梯度累积、学习率缩放。
- checkpoint 保存/恢复、训练中断续跑、日志与指标、配置（复用 P1）。
- 用 P3 的 MiniTensor 或 PyTorch 在小模型上验证：单卡 vs 多进程吞吐、收敛一致性。
- 进阶：ZeRO-1/2 参数与优化器状态分片、张量并行（列/行切分 + allreduce）、流水并行（micro-batch 调度）在 2–4 进程上的最小实现。
- **备选**：多副本分布式推理（负载均衡 + KV cache 感知路由），更有 AI 味；或者 RaftKV（如果想回系统方向）。

**为什么做、有什么用**

- 训练系统是 AI infra 的另一个高地（训练框架/平台岗），能讲清 allreduce 与并行策略的人很少。
- 单机模拟也能展示核心能力：通信、同步、容错、可观测性。面试不问我有几张卡，问怎么设计。
- 作为最后一个项目，把前面所有项目串成「一个 AI 系统的全栈」。

**学到什么**

- 分布式核心：集合通信、ring allreduce、参数服务器 vs 数据并行、一致性/收敛。
- 并发与容错：进程/线程模型、故障检测、checkpoint、重启恢复。
- 性能：通信占比、计算/通信重叠、带宽瓶颈定位。
- 可观测性：指标、日志、调试。

**里程碑（14 周）**

- W1–2 多进程训练骨架 + 数据切分 → W3–4 ring allreduce + 单测
- W5–6 一致性验证（单卡 vs 2/4 进程 loss 对齐）→ W7–8 checkpoint/恢复
- W9–10 性能分析（通信占比、scaling efficiency）→ W11–12 优化（重叠/压缩）
- W13–14 文档 + 与 PyTorch DDP 对比

**验收**

- [ ] 2/4 进程 loss 曲线与单进程对齐（差 < 1e-3）
- [ ] ring allreduce 带宽测试 + 单测（数值正确、任意进程数）
- [ ] kill 后从 checkpoint 恢复继续训练，曲线连续
- [ ] scaling efficiency 数据
- [ ] 简历句：「实现数据并行训练系统，自研 ring allreduce 集合通信与梯度累积；4 进程相对单进程 scaling efficiency XX%；支持 checkpoint 断点续训；与 PyTorch DDP 对比验证收敛一致性。」

**难度 ★★★★★ ｜ 参考**：PyTorch DDP、Horovod、百度 Ring Allreduce、ZeRO、MIT 6.824（Raft 备选）。

## 4. 暑假小项目（2027.7–8，每个 1–3 周，挑 2–3 个）

| 小项目 | 做什么 | 用途 |
|---|---|---|
| CUDA matmul 阶梯 | naive → shared mem → register blocking，记录每级数据 | P4 热身，开学前有 GPU 手感 |
| pybind11 算子扩展 | C++/CUDA 算子包成 Python 模块，`pip install` 可用 | AI 工程师日常，简历上很实用 |
| **nanoGPT 复现** | PyTorch 复现 Karpathy 的 nanoGPT，训练 + 采样，逐模块理解 | P3「手写小 GPT」的预演，基模方向必修 |
| **数据 pipeline** | 下载/整理 1–5 GB 中文语料，清洗、去重、BPE tokenize、打包 | 基模训练 70% 工作量在数据，面试高频 |
| **CS336 作业** | Stanford CS336 前 3–4 次作业：tokenizer / Transformer / 训练 | 最对口课程，做完 ≈ 半个 P3 |
| **租卡实验** | AutoDL 4090/A100 按小时租，训 30–60M 模型，记吞吐/显存/loss | 证明我真跑过训练 |
| 线程池库 | 固定/动态线程池 + 任务队列 + future | P2/P3 并行基础 |
| SIMD CPU 算子 | AVX2 向量加/dot/softmax，对比自动向量化 | 没 GPU 也能讲性能 |
| micrograd C++ 版 | 200 行标量自动微分 | 正式上 P3 前走通 autograd |
| 无锁队列 | SPSC / MPMC ring buffer | P6 通信基础 |

## 5. 工程规范（我的习惯清单）

**编译选项（Debug）**

```
-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror
-fsanitize=address,undefined          # 内存/未定义行为
-fsanitize=thread                     # 并发项目
-O0 -g
```

**CUDA 项目额外要求**

- `CMAKE_CUDA_ARCHITECTURES 120`（RTX 5060 是 Blackwell；装错版本会报 `no kernel image is available`）；
- 每个 kernel 附 CPU 参考实现 + 容差测试（相对误差 < 1e-4，归约类放宽）；
- 用 `cudaEvent` 计时，固定 warmup，取中位数，报「相对 naive / 相对 cuBLAS」两组倍数；
- `ncu`/`nsys` 至少分析 3 个 kernel，截图进 README；
- 显存用 RAII 封装（`DeviceBuffer`），禁止裸 `cudaMalloc` 散落各处。

**CMake**：target-based；不用 `file(GLOB)`；`CMAKE_EXPORT_COMPILE_COMMANDS=ON`；依赖优先 `FetchContent`，系统库 `find_package`。

**Python（实验脚本）**：`uv` 管理依赖；`ruff` 格式化与 lint；`mypy` 类型检查；pytest 写单测；每个实验记 config + seed + 结果；训练脚本提供 `--dry-run` / `--max-steps`，1 分钟能跑完小规模验证。

**测试**：GoogleTest + ctest；单元 + 集成；关键项目加 fuzz / 对拍 / 故障注入；数值项目必须有「与参考实现对拍」。

**性能**：Google Benchmark 出数据；Linux 用 `perf` + 火焰图；GPU 用 `ncu` + roofline；README 必须有「优化前后」对比表。

**Git**：`main` + feature 分支 + PR（自己 review 自己）；Conventional Commits；里程碑打 tag。

**CI**：GitHub Actions，ubuntu + windows 矩阵；GPU 项目 CI 只跑 CPU 参考与编译检查，性能数据在本机跑进 README。

**文档**：README 固定结构——一句话简介、截图、架构图（mermaid）、Quick Start、Benchmark、设计取舍、Roadmap、License；复杂项目加 `docs/design.md`。

**代码风格**：clang-format（Google/LLVM）；clang-tidy 开 `modernize-*`、`bugprone-*`、`performance-*`、`cppcoreguidelines-*`。

## 6. 环境现状与待办（2026-10-23 实测，11-16 复核）

**已装 / 已验证**

- WSL2 Ubuntu 24.04（`Ubuntu-24.04`，用户 `dym`，NOPASSWD sudo）
- g++ 13.3.0、clang 18.1.3、CMake 3.28.3、Ninja 1.11.1、git 2.43.0、clangd、gdb/lldb
- VS Code + Remote-WSL + C/C++ / CMake Tools / clangd
- GPU：RTX 5060 Laptop 8 GB，驱动 592.01，WSL 内 `nvidia-smi` 正常
- GoogleTest 1.14、ASan/UBSan、Google Benchmark、nlohmann-json 3.11

**待装（按需，不用一次装完）**

- CUDA Toolkit 12.8+（周末装，`sudo apt-get install -y cuda-toolkit-12-8` 或官方 runfile）
- Nsight Compute / Systems（`ncu`、`nsys`）
- pybind11（暑假 2027）、PyTorch（CPU 版作对照即可）
- `nvidia-smi` 8151 MiB 里系统/WSL 占一部分，实际可用约 6.5–7 GB，按 ≤6 GB 设计

**省显存设置**（`%UserProfile%\.wslconfig`）：

```
[wsl2]
memory=8GB
processors=6
```

## 7. 学习资源（精选，别贪多）

**书**

- 《C++ Primer》第 5 版——当参考，别通读
- 《Effective Modern C++》——必读，分主题读
- 《C++ Concurrency in Action》——P2 期间读前 4 章
- 《Linux 多线程服务器编程》（陈硕）——P2 配套
- PMPP——P4/CUDA 圣经
- 《深入理解计算机系统》——贯穿全程
- 《深度学习》花书第 6/8 章 + 《动手学深度学习》——P3 配套
- 《数据密集型应用系统设计》——P6/后端备选配套

**网站 / 视频**

- cppreference.com（字典）、learncpp.com（教材）、C++ Core Guidelines
- CUDA C++ Programming Guide、NVIDIA 官方 matmul 优化 blog
- PyTorch Tensor/autograd 文档、Karpathy 的 micrograd/nanoGPT 视频
- CS336（Stanford，基模最对口）、CS224N
- CppCon Back to Basics（B 站有搬运）、C++ Weekly

**论文（P4/P5 期间读，每篇 1 页笔记）**

- 架构与预训练：Attention Is All You Need、GPT-2/3、LLaMA 1/2、Chinchilla、Switch Transformer/Mixtral
- 系统与推理：FlashAttention、vLLM/PagedAttention、ZeRO、Megatron-LM、Ring Allreduce、GPTQ/AWQ/llama.cpp Q4_0
- 后训练（大三上读）：InstructGPT/RLHF、DPO

**值得读源码**

- nlohmann/json（P1 对照）、fmt（现代 API 设计）、muduo（P2）
- micrograd / tinygrad（P3）、PyTorch ATen 的算子组织
- llama.cpp 的 ggml（P4/P5）、cuBLAS 示例
- googletest（测试写法）

**比赛 / 开源**

- 开源之夏 OSPP（每年 5–9 月，大一大二可参加）：选 AI infra / 推理 / 编译器方向
- 华为软件精英挑战赛、昇腾相关比赛（大二下–大三）
- 数据库大赛（OceanBase/PolarDB，想做存储方向的话）
- GSoC（18+，大二后）
- 有 OI 底子可以打 ICPC/CCPC，但控制时间，别挤占项目

## 8. 里程碑（打勾用）

- [x] 2026.11 中旬：阶段 0 完成，CMake 模板跑通，练习 commit，**P1 提前完成**（68 测试、JSONTestSuite 95/95+188/188、fuzz 10 万、CI 绿、已开源）
- [ ] 2026.12–2027.1：P1 收尾 + 写复盘博客 + tokenizer.json 实战（也可直接进 P2）
- [ ] 2027.6：P2 完成（8 周版），压测数据 + `/predict` batching + 部署
- [ ] 2027.8：nanoGPT 复现 + 租卡训 30–60M 模型（loss 曲线/生成样例）+ pybind11 + 至少 1 个开源 PR
- [ ] 2028.1：P3 完成，MNIST 97%+，手写小 GPT 训出连贯文本，与 PyTorch 对拍报告
- [ ] 2028.6：P4 完成，ncu roofline 图 + cuBLAS 对比 + Adam/attention kernel
- [ ] 2028.8：简历定稿（P1/P3/P4/P5 + 小 GPT 训练经历），准备 2029 暑期实习与保研
- [ ] 2029.1：P5 完成，与 llama.cpp 对比 + 流式 HTTP
- [ ] 2029.6：P6 完成（DDP + ring allreduce + ZeRO/张量并行最小实现）+ 实习 offer

## 9. 我打算怎么干

1. **每个项目开始**：先要需求 + 接口草案 + 第一周任务，自己看完提问，确认再动手。
2. **代码自己写**：卡住先查 20–30 分钟再问；宁可被讲原理和最小示例，也不要直接拿完整实现——直接拿来的代码，面试一问就露馅。
3. **每个里程碑 review**：看 `git diff`，重点聊设计、现代 C++ 写法、性能与测试。
4. **节奏**：每周 8–10 小时（2–3 次，每次 2–3 小时）；每两周要有一个能跑的 demo；每个项目结束写复盘。
5. **复盘模板**：做了什么 / 没做完什么 / 性能数据 / 最大的技术困难与解法 / 重写会怎么改 / 面试可能追问的 3 个问题。

## 10. 课程对齐

| 学期 | 专业课（预计） | 对齐项目 |
|---|---|---|
| 大一上 | 程序设计基础、高数 | 阶段 0 + P1 |
| 大一下 | 数据结构、线性代数、离散数学 | P2 + 线代支撑 P3/P4 |
| 大二上 | 计组、操作系统、概率论 | P3（内存/数值/并发） |
| 大二下 | 计算机网络、机器学习、数据库（如有） | P4 + P2 深化 |
| 大三上 | 深度学习、编译原理（如有）、软工 | P5 + CS336 + 后训练论文 |
| 大三下 | 分布式系统（如有）/ 选修 / 实习 | P6 + 实习 / 科研 |

> 基模专项自学（不占学分）：CS336 全 10 讲 + CS224N 前半 + 每周论文笔记 + 每 2–3 个月租卡实验。研究线额外：线代/概率/优化过一遍，大二下开始联系导师。

## 11. 方向分支（2028 年再定，现在不焦虑）

| 方向 | 必做 | 加分 | 适合谁 |
|---|---|---|---|
| **AI 基模 · 工程线（当前主线）** | P3（含小 GPT）、P4、P5、P6 | 开源 PR（llama.cpp / vLLM / Megatron）+ 租卡实验 + CS336 | 训练系统 / 推理 / 算子岗，本科靠项目 + 实习入口 |
| AI 基模 · 研究线 | P3 手写小 GPT + 线代/概率/优化 + 复现 2 篇论文 | 进组科研、复现报告、GPA、英语 | 读博/科研岗，C++ 是工具，重心在实验与写作 |
| AI 算法 / 应用 | P1 + P3 + PyTorch 训练项目 | 论文复现、Kaggle | 重心在 Python/实验，P5 可作加分 |
| 后端 / 基础架构 | P2（可选 MiniKV） | P6 RaftKV | 岗位最多最稳，项目均衡即可 |
| 数据库内核 | MiniKV（原计划） | 分布式 KV | 系统课扎实，和 AI infra 二选一 |
| 图形 / 图形学 | RayTracer（原计划） | 光栅化渲染器 | 需要图形学课，和 AI 关系不大 |

> 原计划的存储/图形/编译器项目（MiniKV、RayTracer、MiniLang）需求都在 `ROADMAP-archive.md` 里，需要时能捡回来。

## 12. 技能自查清单（面试前逐条过）

- [ ] 能手写 CUDA kernel 并解释 warp / SM / occupancy / 访存合并
- [ ] 能读 ncu roofline 和 stall reason，判断算力还是带宽瓶颈
- [ ] 能手写 matmul shared memory tiling，说清每级优化收益
- [ ] 理解 Transformer 推理：prefill vs decode、KV cache、RoPE、GQA、attention mask
- [ ] 能做 int8 对称量化与反量化，评估精度损失
- [ ] 懂 continuous batching 的调度目标与尾延迟权衡
- [ ] 能手写 autograd（计算图 + 反向传播 + 梯度检查）
- [ ] 理解数据并行 / ring allreduce / 梯度累积 / checkpoint
- [ ] 能用 pybind11 把 C++/CUDA 暴露给 Python
- [ ] 能复现一篇 AI 系统论文的核心 idea（FlashAttention 或 PagedAttention）

基模专项：

- [ ] 从零实现 Transformer 并训出会说话的 30–60M 模型，loss 曲线进 README
- [ ] 懂预训练数据 pipeline：清洗、去重、BPE、打包、shuffle，能说清每步对 loss 的影响
- [ ] 理解 scaling laws，能设计 3 个规模的小实验验证趋势
- [ ] 懂后训练：SFT、DPO/RLHF 的目标函数与最小实现
- [ ] 能解释 RoPE / RMSNorm / GQA / MoE 各自解决什么、代价是什么
- [ ] 精读并复现过 1 篇基模论文，有复现报告
- [ ] 能说清「预训练 / SFT / RLHF / 推理」四段的算力、数据、目标与评测

## 13. C++ 之外：多语言与工程工具（2026-10-29 增补）

**为什么加**：基模岗真实技术栈是「Python 做实验 + CUDA/C++ 做系统」。只会 C++ 会被说「系统底子好但做不了实验」，只会 Python 会被说「调包侠」。我要两头都能干活。

### 13.1 语言分工

| 语言/工具 | 在岗位里的角色 | 目标 | 阶段 |
|---|---|---|---|
| C++20 | 训练/推理系统、算子框架、性能关键路径 | 主线 | P1–P6 |
| CUDA / Triton | kernel 实现与调优 | 能写能调 | P4、暑假 2027 |
| **Python** | 实验、训练脚本、数据处理、评测、工具链 | 工程级 | 贯穿全程，见 13.2 |
| Shell / Linux | 环境、进程、日志、ssh/tmux、监控 | 熟练 | 全程 |
| SQL | 训练数据统计与采样 | 会用 | 大二下起 |
| Docker | 环境复现、CI、算子容器 | 会用 | P4 前 |
| Git / CI | 版本、协作、自动测试 | 熟练 | 全程 |
| （可选）Rust / Mojo | 尝鲜 | 了解 | 大三 |

### 13.2 Python 五级

- **L1 语言**：dataclass、typing、生成器、上下文管理器、装饰器、异常链、async/await、pathlib、argparse。
- **L2 工程化**：uv/venv、pyproject.toml、pytest（fixture/parametrize/mock）、ruff、mypy、logging、CLI。
- **L3 数值与数据**：numpy（广播/向量化/内存布局）、PyTorch（Tensor/autograd/Dataset/DataLoader/optimizer/checkpoint）、tokenizers/datasets、pandas、流式读大 JSONL。
- **L4 性能与调试**：cProfile/py-spy、torch.profiler、nvidia-smi、OOM 排查、multiprocessing、num_workers。
- **L5 生态与可复现**：HF transformers、wandb/tensorboard、torchrun/DDP、vLLM 客户端；每个实验有 config + seed + 结果表。

### 13.3 训练安排（不占主线时间）

| 时间 | 内容 | 产出 |
|---|---|---|
| 2026.11–12（与 P1 并行，每周 2–3h） | L1+L2：Python 重写几道 OI 题；搭 ruff+mypy+pytest 模板；写 3 个 CLI 小工具 | `python-lab` 仓库（10 练习 + CI） |
| 2027.3–6（与 P2 并行，每周 2h） | L3：numpy 向量化、JSON→统计报表、pytest 覆盖 | 练习 + 小报告 |
| 2027.7–8（暑假主任务） | L3+L4+L5：nanoGPT、数据 pipeline、pybind11、wandb | 训出小模型 + 实验报告 |
| 2027.9–2028.1（与 P3 并行） | 用 PyTorch 写同结构模型做对拍基线；训练脚本 | 对拍报告 + 脚本 |
| 2028.3–6（与 P4 并行） | torch.profiler + ncu 对比同一算子；benchmark harness | benchmark 报告 |
| 2028.9–2029.1（与 P5 并行） | vLLM 客户端 vs 自己的引擎同 prompt 评测 | 对比表 |
| 2029.3–6（与 P6 并行） | torchrun/DDP 训练对照 | 实验报告 |

**验收**：不看 IDE 能写完 300 行训练脚本；独立走完「数据→训练→评测→记录→复现」；能改 HF 官方 example；`ruff + mypy` 全绿。

### 13.4 语言之外真正卡人的三件事

1. **读论文 → 实现**：每周 1 篇，1 页笔记（问题/方法/代价/能否复现）。
2. **实验设计**：控制变量、ablation、多 seed、报告均值±方差、结果表格化。
3. **调试 GPU**：OOM、NaN loss、loss 不降、吞吐只有预期 1/10 的排查套路（nvidia-smi → 缩小规模单步 → 与 PyTorch 逐层对比）。

### 13.5 一句话

> C++ 让我写得快，Python 让我做得对，CUDA 让我跑得动。现在比例大约 C++ 6 : Python 3 : CUDA 1，大二下之后 C++ 和 Python 各半，实习前 CUDA 上升。

---

## 14. 从实习 JD 反推的补课清单（2026-11-17 增补）

**为什么加**：我上阿里校招官网看了「研究型实习生」这批岗位（236 个），把 4 个和 C++/系统最相关的 JD 逐条拆了。目的不是现在投（我 2030 年 6 月毕业，大一投不了），而是**用 JD 反推我未来三年该练什么**。

### 14.1 我盯的岗位

| 岗位 | 部门 | 毕业窗口 | 硬要求关键词 | 我现在的可及性 |
|---|---|---|---|---|
| 大模型强化学习基础设施（Code Agent / RL Infra） | Token Foundry（北京/杭州） | 2027.11–2032.10 | 系统编程、网络与 RDMA、分布式系统原理、LLM 训推了解 | ★★★★ 最可及 |
| RL Systems 工程师 | 阿里国际（杭州） | 2027.01–2031.08 | Python/PyTorch、PPO/GRPO、slime/veRL、Ray、Agent 框架 | ★★★ |
| Agentic RL Infra 工程师 | 阿里国际（杭州） | 2027.01–2031.08 | veRL/OpenRLHF/TRL 二次开发、训推同步、MoE Router、NCCL | ★★ |
| 安全 AGI 推理 & 训练实习工程师 | 阿里（杭州） | 2027.01–2031.01 | **研究生**、C++/Python、量化/投机解码、vLLM/SGLang/TRT-LLM | ★ 需读研 |

我的毕业时间 2030-06 落在以上所有窗口内——**资格不是问题，能力是问题**。

### 14.2 六个反复出现的关键词 → 我的补课映射

| 关键词 | 出现在 | 我怎么补 | 什么时候 |
|---|---|---|---|
| Python + PyTorch 工程 | 全部 | §13 的 L1–L3 提前一个月启动；12 月底前跑通 pybind11 demo | 2026.12 起 |
| 分布式 / RDMA / NCCL / Ray | Token Foundry、RL Systems | 先懂概念（书 + 博客），再租 2 卡做一次 NCCL allreduce 实验 | 2027 暑假 |
| RL 训练框架 veRL / slime / OpenRLHF | 三个 Infra 岗 | 读一篇官方文档 + 跑通一个最小 PPO 示例，写 1 页笔记 | 2027.9–2028.1 |
| 推理框架 vLLM / SGLang / TRT-LLM | 安全 AGI、RL Infra 加分 | P5 做完后用 vLLM 跑同一 prompt 对比 metrics | 2028.9–2029.1 |
| CUDA / 算子 / 量化 / 投机解码 | 安全 AGI、Token Foundry | P4 三个 kernel（matmul/softmax/attention）+ int8 量化 | 2028.3–6 |
| Agent 系统：Rollout / 沙箱 / 轨迹 / 评测 | Token Foundry、RL Systems | P2 的 `/predict` + 自己写一个最小 Code Agent 工具链当玩具 | 2028 暑假 |
| 论文 / 开源 / 竞赛 | 加分项 | OSPP 一个 PR、每周 1 篇论文笔记、ICPC 视情况 | 全程 |

### 14.3 差距分档

- **A 已有**（P1 给的底子）：现代 C++、CMake/测试/CI/benchmark、性能测量、RAII/异常、工程规范。
- **B 一年内可补**：Python L3、PyTorch 基础、Linux/Docker、分布式基本概念、CUDA 入门、读框架源码（vLLM/veRL 各一篇）。
- **C 需要 1–2 年**：分布式训练/推理实战、RDMA/NCCL、RL 算法（PPO/GRPO）、Agent harness、论文/开源 PR。

### 14.4 学习内容的具体调整（相对 §1、§13）

1. **Python 提前到 2026.12**（原计划 2027.3）：每周 2–3h，先 L1+L2，12 月底前用 pybind11 把 P1 的 JSON 库包一层给 Python 用——一个下午就能做完，但简历上多一行「C++/Python 混合」。
2. **P2 期间（2027.3–6）加三件事**：Docker 基础（1 周）、本地起一次 vLLM 或 SGLang（1 天）、读 PagedAttention 博客写 1 页笔记。
3. **2027 暑假**：nanoGPT + `torchrun` DDP + 一次 NCCL allreduce 小实验（租 2 卡或申请超算机时）。
4. **P3 期间（2027.9–2028.1）**：把「checkpoint 写入 / 数据加载 / 存储 IO」当成需求写进设计，而不是只写 KV。
5. **P4（2028.3–6）**：CUDA 三个 kernel 必须做，且每个都要有「对标 cuBLAS / FlashAttention」的表格。
6. **P5（2028.9–2029.1）**：KV cache、continuous batching、int8 量化；metrics 和 vLLM 对齐（TTFT/TPOT/吞吐）。
7. **2029.3 投递前 3 个月**：LeetCode 200 题 + 简历定稿 + 3 个项目讲述稿（每个 2 分钟版 + 10 分钟版）。

### 14.5 倒推时间表

| 时间 | 必须完成 | 为什么 |
|---|---|---|
| 2027.6 | P2 完成 + 压测数据 | 简历第一块「系统」 |
| 2027.8 | nanoGPT 复现 + DDP + 1 页实验报告 | 证明「会训」 |
| 2028.1 | P3 完成 + 小 GPT 训练 | 证明「懂模型内部」 |
| 2028.6 | P4 完成 + cuBLAS 对比表 | 证明「会优化」 |
| 2028.8 | 简历定稿 + 项目讲述稿 | 秋招/暑期实习准备 |
| 2028.9–2029.1 | P5 完成 + vLLM 对比 | 证明「会推理系统」 |
| 2029.3 | 投暑期实习（日常实习可 2027 暑假先试） | 2030 届窗口 |
| 2029.4–6 | 面试 + 复盘 | — |

### 14.6 中大可以接触的实验室（2026-11-17 检索，以官网为准）

| 团队 | 负责人 | 方向 | 和我哪部分对口 |
|---|---|---|---|
| **ARChitecture and SYStems Upscaling（ARX-Lab）** | 张献伟 副教授（zhangxw79@mail.sysu.edu.cn） | HPC/AI Infra、体系结构、编译与运行时、可扩展系统 | P2/P4/P5 全部，最对口 |
| **国家超级计算广州中心** | 陈志广（大数据存储/并行文件系统）、杜江溯（并行分布式大模型系统）、黄聃（高性能 AI 系统）、卢宇彤（超算/并行系统软件）、肖侬（存储/系统结构） | 超算 + AI 系统 | P3/P4/P5/P6 |
| **分布式系统 / 网络** | 陈鹏飞（分布式/云/OS）、张晓溪（AI Infra/分布式机器学习/可编程网络）、权国聪（分布式缓存 + RL） | 分布式系统 | P5/P6 |
| **广东省大数据分析与处理重点实验室** | 刘玉葆（数据库）、郑立彬（AI2DB/时空数据） | 数据管理 | P3 |
| **人工智能学院（珠海）** | 魏天骐（类脑计算/强化学习/机器人） | RL、机器人 | Agentic RL 方向 |
| **人工智能研究院** | 沈力（高效深度学习/高效强化学习）等 | RL、机器学习系统 | P5 + RL 框架 |

**搭线方式（按成功率排序）**：

1. **课程**：先上他们的课（编译原理/超算/系统课），课上表现好，期末直接问「老师我能不能进组做点东西」；
2. **邮件**：标题写 `[本科生科研] 姓名-专业-年级`，正文 300 字以内说清三件事（读了你们组哪篇工作、我会什么、我想做什么），附成绩单 + 简历 + GitHub 链接（`cpp-journey` 正好能证明动手能力）；
3. **比赛**：全国大学生计算机系统能力大赛、超算竞赛校队（SYSU-SCC）、大创申报——都需要导师，反过来是接触导师的入口；
4. **大创 / 本科生开放实验项目**：每年 4–5 月申报，先找好导师再组队。

**提醒**：大一上先把 GPA 和 P2 稳住，寒假开始发第一批邮件；实验室不是越早越好，是「有东西可展示」的时候最好进。

---

*这份文档随进展更新，每完成一个项目回来补数据和复盘。2026-10-23 按 AI 系统方向调整；2026-10-29 按 AI 基模方向再调整；2026-11-16 P1 完成，回填实测数据；2026-11-17 按阿里研究型实习生 JD 增补 §14（补课清单、倒推时间表、中大实验室清单）。*
