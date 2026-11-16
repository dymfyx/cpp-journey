# cpp-journey

从零开始，用现代 C++ 逐个项目练到能交付：每个项目都有测试、性能数据、CI 和设计文档，不是跟教程抄一遍的练习。

> 学习路线与长期规划见 [ROADMAP.md](ROADMAP.md)。

## 项目

| 项目 | 内容 | 状态 |
| --- | --- | --- |
| [p0-01-raii](projects/p0-01-raii) | RAII 文件封装 `File`：构造即打开、析构即关闭、禁止拷贝、移动转移所有权 | ✅ 15/15 |
| [p0-02-buffer](projects/p0-02-buffer) | `Buffer` 五大特殊成员：深拷贝、移动语义、自赋值安全、`resize` 强异常安全 | ✅ 15/15 |
| [p1-jq-lite](projects/p1-jq-lite) | 手写 JSON 库 + CLI：递归下降解析器、`std::variant` DOM、序列化、jq 风格路径、RFC 6901 JSON Pointer | ✅ 68/68 |

## P1 jq-lite 关键数据

| 指标 | 结果 |
| --- | --- |
| 单元测试 | 68 个（gtest + ctest） |
| 官方 JSONTestSuite | `y_` 95/95 必须接受，`n_` 188/188 必须拒绝 |
| fuzz | 100,000 次随机/破坏输入，零崩溃（ASan + UBSan） |
| 解析吞吐（1 MB 文档，Release） | 约 134 MiB/s，比 `nlohmann/json` 快 **2.1×** |
| 序列化吞吐（1 MB 文档，Release） | 紧凑约 **246 MiB/s**，缩进约 163 MiB/s |
| 工程化 | CMake `install()`/`export` + `find_package(jq-lite)`、GitHub Actions 双配置 CI |

数字随机器状态波动，只做同场对比；详见 [p1 的 README](projects/p1-jq-lite/README.md)。

## 仓库结构

```
cpp-journey/
├── ROADMAP.md              # 三年学习路线（阶段 0 + P1–P6）
├── docs/
│   ├── HANDOFF.md          # 项目交接文档（进度、环境、坑）
│   └── python-track.md     # Python 工程能力 12 周计划
├── projects/
│   ├── p0-01-raii/         # 阶段 0 练习：RAII
│   ├── p0-02-buffer/       # 阶段 0 练习：特殊成员函数
│   └── p1-jq-lite/         # 项目 1：JSON 库 + CLI
└── scripts/                # 环境准备脚本（WSL2 / Ubuntu / 工具链）
```

## 快速开始

需要 Linux 环境（我用 WSL2 + Ubuntu 24.04），依赖：`g++ 13`、`cmake ≥ 3.25`、`ninja`、`libgtest-dev`、`libbenchmark-dev`。

```bash
git clone https://github.com/dymfyx/cpp-journey.git
cd cpp-journey/projects/p1-jq-lite

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure      # 68/68

# 跑起来
echo '{"a":{"b":[1,2,{"c":"d"}]}}' | ./build/jq_lite '.a.b[2].c'   # "d"
echo '{"a":{"b":[1,2,{"c":"d"}]}}' | ./build/jq_lite -r '.a.b[2].c' # d
```

作为库使用（安装后别的项目 `find_package` 即可）：

```bash
cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
cmake --build build-rel && cmake --install build-rel --prefix /tmp/jq-stage
cd examples/consumer
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/tmp/jq-stage && cmake --build build && ./build/consumer
```

## 进度

| 阶段 | 内容 | 状态 |
| --- | --- | --- |
| 阶段 0 | 工具链 + 现代 C++ 速成（RAII、特殊成员、异常） | ✅ 完成 |
| P1 | jq-lite：JSON 库 + CLI（10 周） | ✅ 完成 |
| P2 | epoll HTTP 服务器（含 AI 推理接口 `/predict`） | 下一步 |
| P3 | MiniTensor：张量与自动微分 | 规划中 |
| P4 | CUDA 算子库 | 规划中 |
| P5 | MiniInfer：LLM 推理引擎 | 规划中 |
| P6 | 分布式训练 | 规划中 |

## 技术栈

C++20、CMake、Ninja、GoogleTest、Google Benchmark、ASan/UBSan、GitHub Actions；后续阶段会加入 CUDA、PyTorch（Python 侧）、pybind11。

## License

暂未添加，计划 MIT。
