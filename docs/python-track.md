# Python 工程能力训练计划（12 周，2026.11–2027.1）

> 与 P1（jq-lite）并行执行：**每周 2–3 小时，不占 P1 时间**。目标不是「会写 Python」，而是达到基模岗位的**工程级**水平：能写可复现的训练/评测脚本、能改 HuggingFace 官方 example、能独立调试数据与性能问题。
> 配套 ROADMAP §13。全程一个仓库：`C:\Users\dym54\Desktop\c++项目\projects\python-lab`。

---

## 0. 仓库骨架（第 1 周搭好）

```
python-lab/
├── pyproject.toml          # 项目元数据 + 依赖（uv 管理）
├── README.md               # 每周进度表 + 验收记录
├── src/pylab/              # 库代码（工具函数、数据、模型）
├── scripts/                # CLI 脚本（argparse/typer）
├── tests/                  # pytest
├── data/                   # 小样本数据（git 忽略大文件）
└── .github/workflows/ci.yml  # ruff + mypy + pytest
```

工具链（Ubuntu/WSL 内）：
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh   # uv
uv init python-lab && cd python-lab
uv add numpy pytest ruff mypy
```

---

## 每周任务

| 周 | 主题 | 任务（每条都要有代码 + 测试/记录） | 验收 |
|---|---|---|---|
| W1 | 工程化起步 | 搭骨架；`ruff`/`mypy`/`pytest` 跑通；CI 绿；写 `hello.py` CLI 并测试 | `uv run pytest` 绿，CI badge 亮 |
| W2 | 语言基础 | dataclass、typing（Optional/Union/Sequence）、生成器、上下文管理器各写 1 个小工具 | 5 个工具进 `src/pylab/`，有 pytest |
| W3 | 装饰器与异常 | 写 `@retry`、`@timer`、`@cache` 装饰器；异常链 `raise ... from ...`；argparse 子命令 | 3 个装饰器有测试；CLI 能跑 |
| W4 | 测试工程 | pytest fixture、parametrize、mock（模拟文件/网络）；测试覆盖数据清洗函数 | 覆盖率 ≥ 80%（pytest-cov） |
| W5 | numpy 向量化 | 把 5 道 OI 题改成 numpy 批量版（前缀和、滑动窗口、矩阵运算），对比循环版耗时 | 有耗时对比表 |
| W6 | numpy 内存布局 | 理解 shape/stride/contiguous、`reshape` vs `view`；用 `timeit` 优化一个 100 万数组的操作 | 报告里写出「为什么快了 X 倍」 |
| W7 | 数据处理 | 流式读 JSONL（逐行，不全文加载）、清洗、去重、统计词频；pandas 基础分组/聚合 | 处理 100MB 文件内存 < 500MB |
| W8 | PyTorch 入门 | Tensor 基础、autograd 手算一个线性回归的梯度；写训练循环（SGD） | 线性回归 loss 收敛图 |
| W9 | PyTorch 训练 | Dataset/DataLoader/MLP 训练 MNIST（PyTorch 版），加 val、early stop | 测试集 ≥ 97% |
| W10 | 训练工程 | config（dataclass/yaml）+ seed 固定 + checkpoint 保存/加载 + logging + wandb | 同一 seed 两次训练 loss 完全一致 |
| W11 | 性能与调试 | cProfile/py-spy 找热点；torch.profiler 看算子耗时；故意制造 OOM/NaN 并修复 | 写一份《OOM/NaN 排查手册》进 README |
| W12 | 综合验收 | 写一个 ≥300 行训练脚本（数据→训练→评测→记录→复现），配实验报告 | 换机器能一键复现结果 |

---

## 能力验收（12 周结束时）

- [ ] 不看 IDE 提示能写完 300 行训练脚本
- [ ] `ruff` + `mypy` + `pytest` 全绿，CI 有 badge
- [ ] 能解释「为什么 numpy 向量化快」「为什么 DataLoader 用多进程」
- [ ] 能读懂 HuggingFace example 并改一个参数重跑
- [ ] 有 3 份可复现实验记录（config + seed + 结果表）
- [ ] 仓库 README 有每周进度与踩坑记录

---

## 资源（按需查，别通读）

- Python 官方教程 + `typing` 文档；《Fluent Python》按章节查
- numpy 官方 quickstart、broadcasting 文档
- PyTorch 官方 60 分钟入门 + Autograd 教程
- CS336 的 Python/PyTorch 部分、Karpathy 的 nanoGPT 训练脚本（W10 起读）
- ruff / mypy / pytest 官方文档的前两页就够

---

## 与主线的衔接

- W1–W4 与 P1 的 CMake/测试/CI 经验互相迁移
- W5–W7 直接服务「数据 pipeline」和 P1 的 JSON 处理
- W8–W12 是暑假 nanoGPT 复现（ROADMAP §4）的前置条件
- 每次 `git commit` 用 Conventional Commits，与 C++ 项目保持同一套规范
