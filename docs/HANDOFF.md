# 交接文档 · 现代 C++ 学习项目（cpp-journey）

> 更新时间：**2026-11-11**（Asia/Shanghai）
> 用途：当前对话上下文满 / 换新 AI 窗口时，把本文件整段发给新 AI，即可无缝继续。
> 维护规则：每次 session 结束前更新「0. 30 秒速读」「6. 当前状态」「12. 变更记录」。

---

## 0. 30 秒速读

- **用户**：中山大学大一（2026 级），**人工智能专业**，高中有 OI（信息学竞赛）基础，算法/调试强，面向对象与工程化弱。
- **长期目标**：按 `ROADMAP.md`（AI 基模/系统方向）从易到难做一系列可上 GitHub、能进简历的现代 C++ 项目，边写边学。
- **仓库**：https://github.com/dymfyx/cpp-journey （**public**，monorepo，三个子项目历史完整保留）
- **工作目录**：`C:\Users\dym54\Desktop\c++项目`
- **当前进度**：
  - 阶段 0 完成：`p0-01-raii`（RAII File）12/12、`p0-02-buffer`（深拷贝/移动/resize）15/15。
  - **P1 `jq-lite` 第 1–4 周完成**：解析 / 序列化 / CLI 路径查询 / fuzz+benchmark+CI，**42 个测试全绿**（13 parser + 12 dump + 16 path + 1 fuzz 5000 次）。
  - 第 4 周代码**尚未 commit/push**（见 §6 精确状态）。
- **下一步（从这里继续）**：① 提交并推送第 4 周代码 → ② 用户手写 README 的「性能」「设计取舍」两节 → ③ 第 5 周：`install()`/`find_package` 导出、用户手写最小 benchmark + 最小 fuzz、与 `nlohmann/json` 性能对拍。
- **教学协议**：中文；默认只给「位置 + 框架 + 任务 + 输入实例」，**完整代码只在用户明确要求时给**；代码必须由用户自己写，AI 负责脚手架、测试、评审、讲解。

---

## 1. 用户画像与目标

| 项目 | 内容 |
|---|---|
| 学校/年级 | 中山大学（SYSU），2026 级大一，人工智能专业 |
| 基础 | OI 算法/调试强；C++ 面向对象、现代特性（move/variant/RAII）、工程化弱 |
| 时间投入 | 每周约 8–10 小时（通常周末集中） |
| 方向 | AI 系统 / 推理引擎 / 算子（C++ 最对口），后端/基础架构作为基本功 |
| 硬件 | RTX 5060 Laptop GPU（8GB 显存，sm_120 Blackwell），驱动 592.01，WSL 内 `nvidia-smi` 直通可用；**CUDA Toolkit 未装**（用户决定 P1 完成后再装） |
| 最终诉求 | 简历可用项目 + 工程习惯 + 每个项目一个可展示仓库 |

---

## 2. 教学协议（新 AI 必须遵守）

1. **全程中文**，术语首次出现给中文解释 + 小例子。
2. 讲一个函数/语法点按「**是什么 → 为什么需要 → 用在哪 → 坑**」讲，用户会追问到懂为止（历史问题密度高，耐心答）。
3. 用户明确要求（2026-10-22 原话）：*「你只发框架和要做什么给我，我让你发完整代码你再发，然后给我他输入的实例」*。→ 默认给：**文件位置、函数骨架（带 TODO 注释）、任务清单、输入/输出实例、验收命令**；完整实现只在用户说「给我代码 / 不会写」时才给。
4. 不要让用户长时间卡住：给**提示阶梯**（定位 → 伪代码 → 关键几行 → 完整代码），逐级升级，用户要才跳最后一层。
5. 每个项目 AI 建脚手架：`CMakeLists.txt + include/<ns>/*.hpp + src/*.cpp（骨架）+ tests/*.cpp + README.md + .gitignore`；**只让用户写核心实现**。
6. 每个里程碑让用户 commit（英文 conventional commit：`feat:` / `fix:` / `docs:` / `chore:` / `test:`）。
7. 验收标准：`-Werror` 零警告、ASan/UBSan 默认开、GTest 全绿、Release 基准数字可复现。
8. 每个项目单开 VS Code 窗口（`code .`），避免 CMake Tools 抓错项目（用户踩过）。
9. 用户会要求「详细讲解」「给个实例」「模拟一遍」，这类请求直接满足，别省。

---

## 3. 环境现状（已装好，不要重装）

### 3.1 Windows 侧
- Windows 11 25H2，build `26200.9711`；内存 16GB。
- WSL 发行版 `Ubuntu-24.04`：**import 安装**（路径 `C:\Users\dym54\AppData\Local\WSL\Ubuntu-24.04`），不是商店版；WSL 版本 `3.0.3.0`，内核 `6.18.54.1-1`。
- 启动 Ubuntu：Win+R 输入 `wsl`；或双击 `scripts/open-ubuntu.bat`；或 Windows Terminal。
- 商店 `wsl --install` 不可用（`raw.githubusercontent.com` 超时），改用「官方 MSI + ubuntu-base rootfs 导入」方案，脚本都在 `scripts/`。
- `C:\Users\dym54\.wslconfig`：`[wsl2] memory=4GB processors=6 swap=2GB autoMemoryReclaim=gradual`。

### 3.2 WSL 内（Ubuntu 24.04.5 LTS）
| 工具 | 版本 | 备注 |
|---|---|---|
| g++ | 13.3.0 | 项目标准 C++20 |
| clang / clangd | 18.1.3 | VS Code 用 clangd |
| cmake | 3.28.3 | |
| ninja | 1.11.1 | |
| git | 2.43.0 | 身份已配 |
| GoogleTest | 1.14 | 系统包 |
| Google Benchmark | 1.8.3 | `libbenchmark-dev` |
| gdb / lldb | 已装 | |

- 默认用户 `dym`（sudo 免密，`/etc/sudoers.d/90-dym`），`/etc/wsl.conf` 里 `default=dym`。
- VS Code 扩展：WSL、clangd、CMake Tools、C/C++（IntelliSense 已关，统一用 clangd）。

### 3.3 AI 从 Windows 操作 WSL 的方式
```powershell
wsl.exe -d Ubuntu-24.04 -- bash -lc 'cd "/mnt/c/Users/dym54/Desktop/c++项目/projects/p1-jq-lite" && cmake --build build && ctest --test-dir build'
```
- 需要 **danger-full-access** 沙箱；workspace-write 下 `wsl.exe` 会报 `Wsl/E_ACCESSDENIED`。
- WSL 内访问 Windows 文件：`/mnt/c/Users/dym54/Desktop/c++项目/...`（路径含中文，bash 里要加引号）。

---

## 4. 目录地图（monorepo）

```
C:\Users\dym54\Desktop\c++项目\        ← git 仓库根 (main)
├─ README.md                 # 仓库首页（项目索引/技术栈/快速开始/进度）
├─ ROADMAP.md                # AI 基模方向总路线（666 行，P1..P6 + 阶段0 + CUDA）
├─ .clang-format             # Google 基底、4 空格、100 列
├─ .github\workflows\ci.yml  # CI：Debug+ASan / Release 两个 job（未提交）
├─ docs\
│  ├─ HANDOFF.md             # 本文件
│  ├─ ROADMAP-archive.md     # 旧非 AI 版项目存档（MiniKV/RayTracer/MiniLang/RaftKV）
│  ├─ python-track.md        # Python 12 周训练计划（未启动）
│  ├─ value-cpp-讲解.md       # value.cpp 逐段讲解（spoiler，用户重写前别主动发）
│  └─ value-cpp-reference.cpp # 参考实现（用户对照过，已完成 25/25）
├─ scripts\                  # WSL 安装/导入/电源/启动脚本 + 日志
└─ projects\
   ├─ .vscode\               # 工作区配置（clangd + CMake）
   ├─ build\                 # 旧构建产物（p0-02 时期），建议删
   ├─ p0-01-raii\            # 阶段0-1：RAII File 封装（12/12）
   ├─ p0-02-buffer\          # 阶段0-2：Buffer 深拷贝/移动/resize（15/15）
   └─ p1-jq-lite\            # P1：JSON 库 + CLI + fuzz + benchmark
      ├─ include/jq/{error,parser,path,value}.hpp
      ├─ src/{parser,value,path,main}.cpp
      ├─ tests/{test_parser,test_dump,test_path,fuzz_parser}.cpp
      ├─ benchmark/bench_parser.cpp
      └─ CMakeLists.txt      # 目标：jq_lib / jq_tests / jq_dump_tests / jq_path_tests / jq_lite / jq_fuzz / jq_bench
```

**顺带待清理**：仓库根混进了若干历史垃圾文件（`_*.ps1`、`*.png`、`projects/p0-01-raii/Testing/`），是早期提交带进去的，和项目无关，建议 `git rm --cached` + 更新 `.gitignore` 后单独提交。

---

## 5. 进度时间线

| 日期 | 内容 | 结果 |
|---|---|---|
| 2026-10-20 | 环境搭建（WSL2 MSI + ubuntu-base 导入 + 工具链 + VS Code 扩展） | 完成 |
| 2026-10-21 ~ 22 | `p0-01-raii`：构造/析构/close/读写/移动语义 | 12/12 |
| 2026-10-22 ~ 23 | `p0-02-buffer`：深拷贝/移动/自引用/resize/swap | 15/15 |
| 2026-10-24 ~ 25 | P1 第 1 周：递归下降 JSON 解析器（字面量/数字/字符串/对象/数组/行列号） | 13/13 |
| 2026-10-26 ~ 11-02 | P1 第 2 周：序列化 dump 重写（含参考实现对照） | 25/25 |
| 2026-11-03 ~ 11-06 | 三仓库合并为 monorepo `cpp-journey`（git subtree 保留历史）+ P1 第 3 周 CLI/路径查询 | 已 push；41/41 |
| 2026-11-07 ~ 11-11 | P1 第 4 周：fuzz 驱动、Google Benchmark、GitHub Actions CI + 概念答疑 | 42/42，fuzz 干净，**未提交** |

P1 当前 git 历史（`projects/p1-jq-lite` 子树内）：`4249d77` 骨架 → `1e1d1de` 数字 → `2c28380` 字符串 → `2beb25d` 对象/数组 → `8fa3f83` dump（25/25）→ 之后 CLI（`efcf549` 为仓库根 HEAD）。

---

## 6. 当前状态（精确，2026-11-11 21:40 验证）

### 6.1 Git
- 分支 `main`，HEAD = `efcf549 feat: CLI 路径查询（jq 子集，41/41）`，**已推送**到 origin。
- **未提交**：
  ```
   M projects/p1-jq-lite/CMakeLists.txt        # 第 4 周接线（jq_fuzz/jq_bench/add_test）
  ?? .github/                                 # CI workflow
  ?? projects/p1-jq-lite/benchmark/           # bench_parser.cpp
  ?? projects/p1-jq-lite/tests/fuzz_parser.cpp
  ```
- **今天修的两个问题**（重要，别再犯）：
  1. `CMakeLists.txt:17` 用户误把 `add_library(jq_lib ...)` 打成 `jib`，症状是 `Cannot specify include directories for target "jq_lib" which is not built by this project` → 已改回 `jq_lib`；
  2. `src/value.cpp` 残留用户「改坏它」实验（`"` 未转义）→ 已 `git checkout -- projects/p1-jq-lite/src/value.cpp` 还原。

### 6.2 构建与测试（已验证）
```bash
cd /mnt/c/Users/dym54/Desktop/c++项目/projects/p1-jq-lite
cmake --build build && ctest --test-dir build --output-on-failure   # 42/42
./build/jq_fuzz 5000                                                # 干净
```
- 42 个测试 = `jq_tests` 13 + `jq_dump_tests` 12 + `jq_path_tests` 16 + ctest 里的 `FuzzParser`（`jq_fuzz 5000`）。
- Release 基准（1MB 文档、`build-rel`、`-O3 -DNDEBUG`、sanitizer 关）：
  `Parse/Tiny 201ns`、`Parse/Large 5.19ms (~192 MiB/s)`、`Dump/Compact ~205 MiB/s`、`Dump/Pretty ~153 MiB/s`、`EvaluatePath 82.6ns`。

### 6.3 已完成 vs 待办
- ✅ P1 第 1–4 周代码与测试
- ⬜ 第 4 周代码 commit + push（需要用户执行，见 §8）
- ⬜ `projects/p1-jq-lite/README.md`：用户手写「性能」「设计取舍」两节（占位提示已给）
- ⬜ README 的「10 周里程碑」表与实际执行顺序不一致（实际：周 1 解析、周 2 dump、周 3 CLI+路径、周 4 fuzz+bench+CI；JSONTestSuite 还没做），需要用户更新
- ⬜ 第 5 周：`install()`/`install(EXPORT)` + `find_package(jq-lite)`；用户手写最小 benchmark（`std::chrono`）与最小 fuzz；与 `nlohmann/json` 对拍

---

## 7. 路线（详见 ROADMAP.md）

| 阶段 | 项目 | 状态 |
|---|---|---|
| 阶段 0 | RAII File + Buffer 移动语义 | ✅ |
| P1 | jq-lite JSON 库（解析/序列化/CLI/fuzz/bench/CI/install） | 进行中（第 5 周起） |
| P2 | epoll + 多线程 HTTP 服务器，`POST /predict` + 动态 batching | 未开始 |
| 暑假 2027 | CUDA 入门（需 12.8+，sm_120）+ pybind11 | 未开始 |
| P3 | MiniTensor（张量 + 自动微分 + MNIST） | 未开始 |
| P4 | CUDA 算子库（matmul 多级优化 / softmax / layerNorm / fused attention） | 未开始 |
| P5 | MiniInfer（LLaMA 结构 + KV cache + int8 量化 + continuous batching） | 未开始 |
| P6 | 分布式训练（ZeRO/TP/流水）或分布式推理（备选 RaftKV） | 未开始 |
| 保底 | P1 + P3 + P5 做深 + P4 三个 kernel | — |

---

## 8. 常用命令

```bash
# 进入 P1（WSL 内，注意引号包中文路径）
cd "/mnt/c/Users/dym54/Desktop/c++项目/projects/p1-jq-lite"

# Debug：构建 + 全部测试 + 单测 + fuzz
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/jq_tests --gtest_filter='ParserTest.ParsesNumbers'
./build/jq_fuzz 20000 12648430          # 12648430 = 0xC0FFEE

# Release：性能基准（sanitizer 必须关，否则数字慢 60–100 倍）
cmake -S . -B build-rel -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF
cmake --build build-rel && ./build-rel/jq_bench

# CLI 手测
echo '{"a":{"b":[1,2,{"c":"d"}]}}' | ./build/jq_lite '.a.b[2].c'      # → "d"
echo '{"a":1}' | ./build/jq_lite -c '.'                                 # → {"a":1}
```

**Git 规矩（重要）**：`commit` 在 WSL 里做；**`push` 必须在 Windows PowerShell 里做**——WSL 连不上 `github.com:443`（`Failed to connect ... port 443`）。
```powershell
git -C "C:\Users\dym54\Desktop\c++项目" push
```
git 身份：`user.name=dymfyx`、`user.email=1526864905@qq.com`；分支 `main`。

---

## 9. 已知坑 / 约定清单

1. 统一编译选项：`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wdouble-promotion`，库额外 `-Werror`；`ENABLE_SANITIZERS=ON` 默认开 ASan+UBSan。
2. 匿名 `namespace` 里的 `todo()` 占位函数，**全部实现完必须删**，否则 `-Werror=unused-function` 编译失败（用户踩过两次：p1 和 p0）。
3. `noexcept` 函数里绝不能留 `todo()`（会 `std::terminate`）。
4. `std::string_view` → `std::string` 必须显式构造（`explicit`）；`const char* + int` 是指针算术坑。
5. 少写一行 `namespace {` 会让 `}  // namespace` 提前关闭项目命名空间，症状是一堆「未定义标识符」——先看花括号配对。
6. `ctest` 在 `projects/` 根目录跑会跑到旧 `projects/build`（p0-02 的 15 个 BufferTest），看起来「全绿」其实不是当前项目——**必须在项目目录下跑**，或 `ctest --test-dir <项目>/build`。
7. CMake target 名字打错（如 `jib`）的症状是 `target_include_directories` 报 "target ... is not built by this project"。
8. `-Wsign-conversion`：`int * int` 传给 `std::string::append(size_t, char)` 必须 `static_cast<std::size_t>(...)`。
9. fuzz 的失败信息要看 `dump`（不是只看原输入）：`generated valid JSON rejected!` / `dump output is not valid JSON!` / `roundtrip mismatch!` 三种含义不同。
10. 基准数字只能来自 Release 构建（`build-rel`，`ENABLE_SANITIZERS=OFF`），否则慢 60–100 倍，写进 README 就是误导。
11. WSL 内存 4GB，VS Code + clangd + 编译会紧张；必要时调 `.wslconfig` 到 6GB。
12. LF/CRLF 警告是 `core.autocrlf=true` 的正常行为，可忽略（可选加 `.gitattributes`）。
13. 中文路径在 WSL bash 里必须加引号；`wsl.exe` 从 Windows 调用需要 danger-full-access。
14. 用户容易忘记保存文件（编辑器里有改动、磁盘没变）——review 前先 `git diff`。

---

## 10. 挂起 / 未启动事项

- [ ] P1 第 5 周任务（install/export、手写 min bench/fuzz、nlohmann 对拍）
- [ ] `projects/p1-jq-lite/README.md` 性能与设计取舍两节
- [ ] 清理仓库根垃圾文件（`_*.ps1`、`*.png`、`Testing/`）
- [ ] JSONTestSuite（y_ 全过、n_ 全拒）—— 原计划第 4 周，尚未做
- [ ] CUDA Toolkit 12.8+ / ncu / nsys / pybind11 / PyTorch（P1 后）
- [ ] `docs/python-track.md` 的 12 周 Python 训练（用户提过，尚未启动）
- [ ] P2 epoll WebServer 脚手架（P1 毕业后）

---

## 11. 给下一个 AI 的开场白（复制粘贴）

```
我在跟 AI 学现代 C++，项目在 C:\Users\dym54\Desktop\c++项目（仓库 dymfyx/cpp-journey）。
请先读 docs/HANDOFF.md 和 ROADMAP.md，然后按教学协议继续：
1) 我当前状态：P1 jq-lite 第 4 周代码完成（42/42、fuzz 干净），但还没 commit/push；
2) 下一步：帮我 review 未提交的改动 → 提交 → 提醒我在 Windows 里 push → 然后我开始写 README 的性能/设计取舍两节，并进第 5 周（install/export、手写最小 benchmark 与 fuzz、和 nlohmann/json 对比）；
3) 默认只给我「位置 + 框架 + 任务 + 输入实例」，完整代码等我明确要；讲解要详细，我会追问。
```

---

## 12. 变更记录

| 日期 | 变更 |
|---|---|
| 2026-10-26 | 创建（阶段 0 完成、P1 第 2 周进行中） |
| 2026-11-11 | 全面重写：单仓库 GitHub 上线、P1 第 1–4 周完成（42/42）、第 4 周未提交状态、修 `jib` 拼写与 value.cpp 实验残留、补 §8 命令与 §9 坑清单 |
