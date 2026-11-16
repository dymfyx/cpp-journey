# cpp-journey

我是中山大学 2026 级人工智能专业的学生，高中有信息学竞赛背景。这个仓库记录我从 2026 年 10 月开始学现代 C++ 的过程——每个项目都是我自己从零写的，带测试、性能数据和文档，不是跟着教程抄一遍。

长期规划在 [ROADMAP.md](ROADMAP.md)。目前做完的是阶段 0 的两个小练习和第一个正式项目 P1。

## 我做过的东西

| 项目 | 我做了什么 | 状态 |
|---|---|---|
| [p0-01-raii](projects/p0-01-raii) | `RAII File` 类：构造即打开文件、析构即关闭、禁止拷贝、移动转移所有权。第一次把「对象管资源」这件事写明白 | 15 个测试 |
| [p0-02-buffer](projects/p0-02-buffer) | `Buffer` 类：深拷贝、移动语义、自赋值安全、带强异常安全的 `resize`。为了搞懂五大特殊成员写的 | 15 个测试 |
| [p1-jq-lite](projects/p1-jq-lite) | 手写 JSON 库 + 命令行工具：递归下降解析器、`std::variant` DOM、序列化、jq 风格路径查询、RFC 6901 JSON Pointer | 68 个测试全部通过 |

### P1 jq-lite 的数据

| 项 | 结果 |
|---|---|
| 单元测试 | 68 个（GoogleTest + ctest） |
| 官方 JSONTestSuite | `y_` 95/95 必须接受、`n_` 188/188 必须拒绝 |
| fuzz | 10 万次随机/破坏输入，零崩溃（ASan + UBSan） |
| 解析吞吐（1 MB 文档，Release） | 133.9 MiB/s，比 `nlohmann/json` 快 2.1× |
| 序列化吞吐（1 MB 文档，Release） | 紧凑 245.8 MiB/s、缩进 162.5 MiB/s |
| 工程化 | CMake `install`/`export` + `find_package(jq-lite)`、GitHub Actions 双配置 CI |

这些数字只能同场对比：笔记本睿频和后台进程会让同一份代码差 30–40%，所以 README 里都标了机器状态和构建类型。

## 仓库结构

```
cpp-journey/
├── ROADMAP.md              # 我给自己排的三年计划（阶段 0 + P1–P6）
├── docs/
│   ├── HANDOFF.md          # 项目交接文档：进度、环境、踩过的坑
│   └── python-track.md     # Python 工程能力 12 周计划
├── projects/
│   ├── p0-01-raii/         # 练习：RAII
│   ├── p0-02-buffer/       # 练习：拷贝与移动
│   └── p1-jq-lite/         # 项目 1：JSON 库 + CLI
└── scripts/                # 我配环境用的脚本（WSL2 / Ubuntu / 工具链）
```

## 怎么跑起来

我在 WSL2 + Ubuntu 24.04 上开发，依赖 `g++ 13`、`cmake ≥ 3.25`、`ninja`、`libgtest-dev`、`libbenchmark-dev`。

```bash
git clone https://github.com/dymfyx/cpp-journey.git
cd cpp-journey/projects/p1-jq-lite

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure      # 68/68

# 当命令行工具用
echo '{"a":{"b":[1,2,{"c":"d"}]}}' | ./build/jq_lite '.a.b[2].c'    # "d"
echo '{"a":{"b":[1,2,{"c":"d"}]}}' | ./build/jq_lite -r '.a.b[2].c'  # d
```

当库用（装到某个前缀，别的项目 `find_package` 就能用）：

```bash
cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
cmake --build build-rel && cmake --install build-rel --prefix /tmp/jq-stage

cd examples/consumer
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/tmp/jq-stage
cmake --build build && ./build/consumer
```

## 我的进度

| 阶段 | 内容 | 状态 |
|---|---|---|
| 阶段 0 | 工具链 + 现代 C++ 速成（RAII、拷贝/移动、异常） | 完成 |
| P1 | jq-lite：JSON 库 + CLI（原计划 10 周） | 完成，2026-11 |
| P2 | epoll HTTP 服务器（带推理接口 `/predict`） | 下一步 |
| P3 | MiniTensor：张量与自动微分，手写小 GPT | 规划中 |
| P4 | CUDA 算子库 | 规划中 |
| P5 | MiniInfer：LLM 推理引擎 | 规划中 |
| P6 | 分布式训练 | 规划中 |

## 技术栈

C++20、CMake、Ninja、GoogleTest、Google Benchmark、ASan/UBSan、GitHub Actions。后面会加 CUDA、PyTorch、pybind11。

## License

还没加，打算用 MIT。
