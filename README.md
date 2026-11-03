# cpp-journey

手写现代 C++：从 RAII 与动态缓冲区，到 JSON 解析器，再到 eBPF/epoll 服务器、CUDA 算子与 LLM 推理引擎。
一条从大一持续到大三的 C++ 工程成长路线，每个项目都从零实现、带测试、可复现。

## 项目索引

| 目录 | 项目 | 主要内容 | 状态 |
|---|---|---|---|
| [`projects/p0-01-raii`](projects/p0-01-raii) | RAII 文件封装 | `File` 类：构造/析构/移动语义/异常安全，12 个 GTest 用例 | ✅ 12/12 |
| [`projects/p0-02-buffer`](projects/p0-02-buffer) | 动态缓冲区 | `Buffer` 类：深拷贝/移动语义/`resize`/`swap`，15 个用例 | ✅ 15/15 |
| [`projects/p1-jq-lite`](projects/p1-jq-lite) | JSON 解析器 + jq 子集 CLI | 递归下降解析、行列号报错、序列化、路径查询，25 个用例 | 🚧 第 2 周 25/25 |

## 技术栈

- **语言**：C++20（`-std=c++20`），后续 CUDA 12.8+
- **构建**：CMake ≥ 3.25 + Ninja
- **测试**：GoogleTest 1.14 + CTest
- **工具链**：GCC 13.3 / Clang 18.1（WSL2 Ubuntu 24.04）、clang-format、clang-tidy
- **质量**：开 `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`，默认启用 ASan + UBSan

## 快速开始

任选一个子项目：

```bash
cd projects/p1-jq-lite
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## 路线图

三年项目规划（阶段 0 + P1–P6，含每个项目的里程碑、验收标准与简历表述）见 [ROADMAP.md](ROADMAP.md)。

- 早期规划存档：[docs/ROADMAP-archive.md](docs/ROADMAP-archive.md)
- Python 工程能力 12 周计划：[docs/python-track.md](docs/python-track.md)

## 进度

- **2026-10** 环境搭建：Windows 11 + WSL2 Ubuntu 24.04 + GCC 13.3 / Clang 18.1 / CMake 3.28 / Ninja
- **阶段 0** ✅ 工具链 + 现代 C++ 基本功（RAII、移动语义、CMake、GTest、sanitizer）
- **P1 jq-lite** 🚧 第 1 周（解析器 13/13）✅；第 2 周（序列化 25/25）✅；CLI / fuzz / benchmark 进行中

> 提交历史记录了每个阶段的真实迭代过程（含踩坑修复），`git log` 可查。

## 环境脚本

`scripts/` 下是 Windows/WSL2 环境搭建脚本（安装 WSL、导入 Ubuntu、配置工具链、电源设置），与项目代码无关，可按需参考。

## License

暂未添加 LICENSE 文件。
