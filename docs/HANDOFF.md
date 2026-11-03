# 交接文档 · 现代 C++ 学习计划

> 生成时间：2026-10-26 10:55（Asia/Shanghai）
> 用途：如果当前 AI 对话的上下文满了、或换了新的对话窗口，把本文件内容整段发给新 AI，即可无缝继续。
> 维护规则：每次 session 结束前更新「12. 变更记录」和「0. 30 秒速读」。

---

## 0. 30 秒速读

- **用户**：中山大学大一（2026 级），**人工智能专业**，高中有信息学竞赛（OI）基础，算法/调试强，面向对象语法弱。
- **长期目标**：用一年半到三年，按 `ROADMAP.md`（AI 系统方向）从易到难做一系列可上 GitHub、能写进简历的现代 C++ 项目；用户跟着一个一个做，边写边学。
- **工作目录**：`C:\Users\dym54\Desktop\c++项目`
- **当前进度**：
  - 阶段 0 完成：`p0-01-raii` 15/15、`p0-02-buffer` 15/15，均已提交。
  - P1 `jq-lite` 第 1 周完成：JSON 解析器 13/13 已提交（commit `2beb25d`）。
  - **第 2 周进行中**：手写 JSON 序列化 `dump`，12 个新测试已就绪，用户正在重写 `src/value.cpp` 的序列化部分，目标 25/25。
- **环境**：Windows 11 25H2 + WSL2 Ubuntu-24.04（本机自装，非商店版）+ 完整 Linux C++ 工具链；VS Code Remote-WSL + clangd。
- **教学协议**：中文；先给「位置 + 框架 + 任务 + 输入实例」，**完整代码只在用户明确要求时给**；代码由用户自己写，AI 负责脚手架、测试、review。

---

## 1. 用户画像与目标

| 项目 | 内容 |
|---|---|
| 学校/年级 | 中山大学（SYSU），2026 级大一，人工智能专业 |
| 基础 | OI 算法与调试强；C++ 面向对象/工程化弱；做过竞赛式单文件程序 |
| 时间投入 | 每周约 8–10 小时 |
| 方向 | AI 系统 / 推理引擎 / 算子（多选里最对口 C++ 的），后端/基础架构作为基本功 |
| 硬件 | RTX 5060 Laptop GPU（8GB 显存，sm_120 Blackwell），驱动 592.01，WSL 里 nvidia-smi 直通可用；**CUDA Toolkit 暂未安装**（用户选择先写完 P1 再装） |
| 最终诉求 | 简历可用项目 + 工程习惯；每 1–2 个月一个可展示的仓库 |

---

## 2. 教学协议（新对话必须遵守）

1. **全程中文**，术语首次出现给中文解释 + 小例子。
2. 用户每问一个函数/语法点，讲「**是什么 → 为什么需要 → 用在哪 → 坑**」，不要直接贴大段实现。
3. **用户明确说过**（2026-10-22）：*「你只发框架和要做什么给我，我让你发完整代码你再发，然后给我他输入的实例」*。 → 默认给：**文件位置、函数骨架（带 TODO 注释）、任务清单、输入输出实例、验收命令**；完整实现只在用户说「给我代码/不会写」时才给。
4. 不许让用户长时间卡住：卡住时给**提示阶梯**（定位 → 伪代码 → 关键几行 → 完整代码），逐级升级，必须用户要才跳到最后一层。
5. 每个练手项目由 AI 建脚手架：`CMakeLists.txt + include/xxx/*.hpp + src/*.cpp（stub）+ tests/*.cpp + README.md + .gitignore`；**用户只写 `src` 里核心实现**，不要替用户写。
6. 每完成一周/一个里程碑，让用户 `git commit`（英文 conventional commit）。
7. 代码必须通过：`-Werror` 全警告、ASan/UBSan 默认开、GTest 全绿。
8. 每个项目单开 VS Code 窗口（`code .`），避免 CMake Tools 抓错项目。

---

## 3. 环境现状（已装好，不用重装）

### 3.1 Windows 侧
- Windows 11 25H2，build `26200.9711`。
- 电源：合盖休眠已恢复（`scripts/restore-power.bat` 在 2026-10-20 15:02 执行过，日志 `scripts/restore-power.log`）。
- WSL 发行版：`Ubuntu-24.04`（import 安装，路径 `C:\Users\dym54\AppData\Local\WSL\Ubuntu-24.04`，不是商店版）。
- WSL 版本：WSL `3.0.3.0`，内核 `6.18.54.1-1`，发行版状态 Running。
- 启动 Ubuntu：Win+R 输入 `wsl`；或双击 `scripts/open-ubuntu.bat`；或 Windows Terminal。
- 说明：商店 `wsl --install` 在本机不可用（`raw.githubusercontent.com` 超时），已改用「官方 MSI 装 WSL + ubuntu-base rootfs 导入」方案；相关脚本都在 `scripts/`。

### 3.2 WSL 内（Ubuntu 24.04.5 LTS）
| 工具 | 版本 |
|---|---|
| g++ | 13.3.0 |
| clang / clangd | 18.1.3 |
| cmake | 3.28.3 |
| ninja | 1.11.1 |
| git | 2.43.0 |
| gdb / lldb | 已装 |
| GoogleTest | 1.14（系统包） |

- 默认用户 `dym`（sudo 免密，`/etc/sudoers.d/90-dym`），`/etc/wsl.conf` 里 `default=dym`。
- VS Code 扩展：WSL、clangd、CMake Tools、C/C++（`C_Cpp.intelliSenseEngine` 已在 `projects/.vscode/settings.json` 关掉，统一用 clangd）。

### 3.3 重要调用方式（AI 从 Windows 沙箱操作 WSL）
```powershell
wsl.exe -d Ubuntu-24.04 --cd 'C:\Users\dym54\Desktop\c++项目\projects\p1-jq-lite' -- bash -lc 'git status --short'
```
- 需要在 **danger-full-access** 沙箱模式下才能调用 `wsl.exe`；workspace-write 会报 `Wsl/E_ACCESSDENIED`。
- 在 WSL 内部访问 Windows 文件：`/mnt/c/Users/dym54/Desktop/c++项目/...`。

---

## 4. 目录地图

```
C:\Users\dym54\Desktop\c++项目\
├─ ROADMAP.md                # AI 系统方向总路线（P1..P6 + 阶段0 + CUDA，约 39KB）
├─ ROADMAP-archive.md        # 旧非 AI 版项目存档（MiniKV/RayTracer/MiniLang/RaftKV）
├─ HANDOFF.md                # 本文件
├─ .clang-format             # Google 基底，4 空格，100 列
├─ scripts\                  # WSL 安装/电源/启动脚本（见下）
├─ 家教简历\                  # 与项目无关，忽略
└─ projects\
   ├─ .vscode\               # 工作区 VS Code 配置（clangd + CMake）
   ├─ build\                 # 旧构建产物（p0-02 时期），可删
   ├─ p0-01-raii\            # 阶段0 练习1：RAII File 封装（15/15，已提交）
   ├─ p0-02-buffer\          # 阶段0 练习2：Buffer 深拷贝/移动/resize（15/15，已提交）
   └─ p1-jq-lite\            # P1 主项目：JSON 解析/序列化（第 1 周 13/13 已提交）
```

`scripts/` 里的关键文件：
- `install-wsl2.ps1/.bat`、`finish-wsl2.ps1/.bat`、`install-wsl-msi.bat`：WSL 本体安装
- `download-wsl-msi.py`、`download-ubuntu-base.py`：下载器（带 SHA256 校验）
- `import-ubuntu.ps1/.bat`：导入 ubuntu-base rootfs 成发行版
- `setup-ubuntu-post.ps1/.bat`：换 apt 源（http 回退链）、装工具链、建用户 dym
- `open-ubuntu.bat`：一键启动 Ubuntu
- `stay-awake.ps1/.bat`、`restore-power.ps1/.bat`：临时禁用/恢复睡眠
- `*.log`：安装日志

---

## 5. 已完成进度

| 时间 | 内容 | 结果 |
|---|---|---|
| 2026-10-20 | 环境搭建（WSL2 + Ubuntu + 工具链 + VS Code） | 完成 |
| 2026-10-21 ~ 22 | p0-01 `RAII File`：构造/析构/close/读写/移动语义 | 15/15，commit `32ea2e6`、`1df489e`（README 踩坑） |
| 2026-10-22 ~ 23 | p0-02 `Buffer`：深拷贝/移动/自赋值/resize/swap | 15/15，commit `1d95e5d` |
| 2026-10-24 ~ 25 | P1 第 1 周：JSON 解析器（字面量、数字、字符串、对象、数组、错误定位） | 13/13，commit `1e1d1de`、`2c28380`、`2beb25d` |
| 2026-10-26 | P1 第 2 周：序列化 dump 测试脚手架 + 原实现验证 25/25 | 测试已就绪，用户重写中 |

P1 仓库 commit 历史（`projects/p1-jq-lite`）：
```
2beb25d feat: 支持 JSON 对象与数组解析，第 1 周 13/13 全绿
2c28380 feat: 支持 JSON 字符串解析（转义与 UTF-8）
1e1d1de feat: 支持 JSON 数字解析
4249d77 chore: jq-lite 工程骨架与第 1 周验收测试
```

---

## 6. 当前任务（第 2 周，进行中）★ 新对话从这里继续

**目标**：用户**不看现有实现**，重写 `projects/p1-jq-lite/src/value.cpp` 的序列化部分，让 `ctest` 达到 **25/25**。

### 6.1 已完成的部分
- 新建 `tests/test_dump.cpp`（12 个测试）：
  `DumpScalars`、`DumpNumbers`、`DumpStringEscapes`、`DumpControlCharacters`、`DumpUnicodePassThrough`、`DumpEmptyContainers`、`DumpCompactArray`、`DumpCompactObject`、`DumpPrettyArray`、`DumpPrettyObject`、`DumpPrettyNested`、`DumpRoundTrip`。
- `CMakeLists.txt` 新增 `jq_dump_tests` target。
- 用**原始 dump 实现**验证过：`jq_tests` 13 + `jq_dump_tests` 12 = **25/25 pass**（测试本身是正确的）。

### 6.2 重写边界（`src/value.cpp`，共 175 行）
| 行号 | 内容 | 处理 |
|---|---|---|
| 14–119 | 匿名 namespace：`append_indent` / `dump_string` / `dump_impl` | **删除重写** |
| 169–173 | `Value::dump(int indent)` | **删除重写** |
| 1–13 | 文件头 include | 保留 |
| 123–155 | `type()`、`is_*()`、`as_*()` | 保留 |
| 157–167 | `size()`、`operator==` | 保留 |

### 6.3 规格（测试即规格，实现可自定）
- `dump(-1)` 紧凑：`[1,2]`、`{"a":1}`；`dump(n>=0)` 缩进：元素前 `"\n" + n*depth 空格`，收尾括号前回到 `depth`。
- 空容器：`[]` / `{}`（缩进模式下也**不换行**）。
- 字符串转义：`"` → `\"`、`\` → `\\`、`\b \f \n \r \t` 对应转义、其余 `< 0x20` → `\u00XX`（小写十六进制）、UTF-8 非 ASCII 原样。
- 数字：`std::snprintf(buf, sizeof buf, "%.17g", v.as_number())`（如 `0.1` → `0.10000000000000001`）。
- 已知坑：`indent * depth` 是 `int`，`std::string::append` 要 `std::size_t` → `static_cast<std::size_t>(...)`（否则 `-Wsign-conversion` + `-Werror` 编译失败）。

### 6.4 用户当前 git 状态（需要先提交脚手架）
```
 M CMakeLists.txt        ← 新增 jq_dump_tests target
 M src/parser.cpp        ← 用户按建议做的清理（已可提交）
?? tests/test_dump.cpp   ← 新增 12 个测试
```
建议先做：
```bash
git add -A
git commit -m "chore: 第 2 周序列化测试脚手架 + parser 清理"
```
之后用户的 `value.cpp` 改动才能用 `git diff src/value.cpp` 和原版对比学习。

### 6.5 验收命令
```bash
cd /mnt/c/Users/dym54/Desktop/c++项目/projects/p1-jq-lite
cmake --build build
ctest --test-dir build --output-on-failure          # 目标：25/25
./build/jq_dump_tests --gtest_filter='DumpTest.DumpScalars'   # 单测
```

---

## 7. 接下来的路线（详见 ROADMAP.md）

### P1 jq-lite 剩余周次（共 10 周）
| 周 | 内容 |
|---|---|
| 1 | JSON 解析器（已完成，13/13） |
| 2 | JSON 序列化 dump（进行中） |
| 3 | CLI `jq-lite`：`jq-lite .name file.json`、`dump`、`--help` |
| 4 | 真实文件：解析 `tokenizer.json`（AI 方向专属任务）+ 性能优化 |
| 5–6 | fuzz（libFuzzer）+ benchmark（Google Benchmark，对比 nlohmann/json） |
| 7 | README + 架构设计文档 + ASan/UBSan 收尾 |
| 8 | GitHub Actions CI（ubuntu/windows 双平台） |
| 9–10 | 扩展（JSON Pointer / jq 表达式子集 / 流式解析） |

### 之后（AI 系统方向）
- 暑假 2027：CUDA 入门（RTX 5060 = sm_120，需 CUDA 12.8+）→ 写第一个 kernel（vector add），配 pybind11
- P2 WebServer：epoll 高并发 HTTP + `POST /predict` + 动态 batching（服务层）
- P3 MiniTensor：张量 + 自动微分 + MNIST 复现（框架层）
- P4 CUDA 算子库：matmul 多级优化 / softmax / layernorm / fused attention，ncu 分析 + cuBLAS 对比
- P5 MiniInfer：LLaMA 结构 + KV cache + int8 量化 + continuous batching
- P6 分布式训练 / 推理（备选 RaftKV）
- 保底：P1 + P3 + P5 做深 + P4 三个 kernel

---

## 8. 常用命令速查

```bash
# 进入项目（WSL 内）
cd /mnt/c/Users/dym54/Desktop/c++项目/projects/p1-jq-lite

# 首次配置 / 重新配置
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# 增量构建 / 全部测试 / 单测
cmake --build build
ctest --test-dir build --output-on-failure
./build/jq_tests --gtest_filter='ParserTest.ParsesNumbers'

# git
git status --short
git log --oneline -5
git diff src/value.cpp
```

git 身份（已配置）：`user.name=dymfyx`，`user.email=1526864905@qq.com`。
**GitHub 推送还没做**（用户决定 P1 完成后再做）。

---

## 9. 已知约定与坑

1. 所有 C++ 项目统一编译选项：`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wdouble-promotion`，库加 `-Werror`；`ENABLE_SANITIZERS=ON` 默认开 ASan+UBSan。
2. `todo()` 占位函数在匿名 namespace 里，**所有函数实现完必须删除**，否则 `-Werror=unused-function` 报错（用户已真实踩过）。
3. `noexcept` 函数里绝不能留 `todo()`（会 `std::terminate`），只能直接实现。
4. `std::string` 与 `std::string_view` 拼接时需先转 `std::string`。
5. 中文注释/字符串都是 UTF-8，`printf` 输出正常。
6. 不要用 `using namespace std;`；命名空间按项目分（`raii::`、`jq::`）。
7. 头文件只放声明/内联；实现放 `.cpp`。
8. CMake 用 target-based（`target_include_directories(... PUBLIC include)`、`target_link_libraries`），不用 `file(GLOB)`。
9. 用户容易忘记 `Ctrl+S`（之前出现过磁盘文件未改、编辑器里有改动的情况）——review 前先 `git diff`。
10. WSL 内存：8GB 左右，必要时 `.wslconfig` 调 `memory=6GB processors=6`。

---

## 10. 未完成 / 挂起事项

- [ ] P1 第 2 周：dump 重写 → 25/25（**当前任务**）
- [ ] P1 第 3 周起：CLI → tokenizer.json → fuzz/benchmark → CI
- [ ] GitHub：P1 完成后创建仓库并推送（还没做）
- [ ] CUDA Toolkit 12.8+、ncu/nsys、pybind11、PyTorch（P1 后）
- [ ] ROADMAP 后续项目按第 7 节推进

---

## 11. 给下一个 AI 的开场白（复制粘贴）

```
我在跟一个 AI 学习现代 C++，这是交接文档（附在下面/路径 C:\Users\dym54\Desktop\c++项目\HANDOFF.md）。
请先完整读一遍 HANDOFF.md 和 ROADMAP.md，然后按「教学协议」继续：
1) 我当前任务是 P1 第 2 周：重写 projects/p1-jq-lite/src/value.cpp 的序列化部分，让 ctest 25/25；
2) 先确认我的 git 状态（脚手架是否已提交），再问我 card 在哪一步、贴报错；
3) 默认只给「位置 + 框架 + 任务 + 输入实例」，完整代码等我明确要。
工作目录：C:\Users\dym54\Desktop\c++项目
```

（也可以直接把本文件全文粘贴给新对话，效果一样。）

---

## 12. 变更记录

| 日期 | 变更 |
|---|---|
| 2026-10-26 | 创建本文件：记录环境、进度、第 2 周任务与教学协议 |
