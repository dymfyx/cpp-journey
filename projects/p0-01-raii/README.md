# P0-01 · RAII File

阶段 0 的第一个练习：用 RAII 写一个不会泄漏、不会 double-close、能安全移动的文件类。

## 任务

只改一个文件：`src/file.cpp`。

`include/raii/file.hpp` 是已经定好的接口，**不要改它**；`tests/test_file.cpp` 是验收测试，**也不要去改**。

## 为什么练这个

C 风格代码里最常见的两个 bug：

1. **忘了 close / 提前 return 忘了 close** —— 句柄泄漏，跑一夜压测就 EMFILE；
2. **释放后又用 / 释放两次** —— 段错误，而且很难复现。

RAII 的答案很直接：**把资源的生命周期绑到对象的生命周期上**。构造函数是这个资源的唯一入口，析构函数是唯一出口。只要不写出裸的 `fclose`，这类 bug 在类型层面就不会发生。

这个类也是后面所有项目的模板：`Socket`、`MemoryPool`、`Page`、`Transaction` 全都是同一个套路 —— 你现在写的不是"一个作业"，是本课程后面 3 年要反复使用的那个模式。

## 移动语义：和 OI 经验不一样的地方

OI 里对象基本就是数组下标和值传递。C++ 里实现 RAII 类型时：

- **拷贝必须删掉**（`= delete`），否则两个对象持有同一个 `FILE*`，析构时 double-close；
- **移动要接管**：把句柄从 `other` 手里拿过来，然后让 `other` 变成"空的、合法的、可析构的"状态；
- **移动构造/赋值必须 `noexcept`**：`std::vector<File>` 在扩容时只会用 `noexcept` 的移动，否则它宁愿拷贝（而你拷贝已经删了，向量就推不动了）；
- **移动赋值要处理自赋值**：`f = std::move(f)` 是合法代码，不能自己把自己关掉。

## 怎么跑

一次性准备（第一次跑之前）：

```bash
sudo apt-get update && sudo apt-get install -y libgtest-dev
```

然后：

```bash
cd /mnt/c/Users/dym54/Desktop/c++项目/projects/p0-01-raii

# 配置（Debug + ASan/UBSan，构建产物在 build/）
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# 编译
cmake --build build

# 跑测试
ctest --test-dir build --output-on-failure
```

先看到 **12 个测试全红**（都报 `TODO: 还没有实现 ...`）是正常的，说明工程和测试链路通了。

## 验收标准

- `ctest` 全绿（12/12）；
- 编译过程 **0 warning**（`CMakeLists.txt` 里已经开了 `-Werror`）；
- ASan/UBSan 不报错（默认已开，跑测试时自动检查）；
- 不要用 `std::exit`、不要用裸 `fclose` 以外的资源管理方式糊弄测试。

## 实现提示（卡住了再看）

1. `fopen(path, mode)` 失败返回 `nullptr`，`errno` 里有原因，用 `std::strerror(errno)` 拼错误信息抛 `std::runtime_error`；
2. 构造函数里如果 `path == nullptr || path[0] == '\0' || mode == nullptr || mode[0] == '\0'`，也直接抛；
3. `close()`：`if (fp_) { std::fclose(fp_); fp_ = nullptr; }`，注意 `fclose` 也可能失败，但析构函数里没法往上抛，忽略即可；
4. `read_all()`：可以先 `fseek` 到末尾取大小，也可以循环 `fread` 进 `std::string` + `append`；后者更稳（支持管道/二进制）；
5. `read_into`：`std::fread(buffer, 1, size, fp_)`，返回值是实际读到的字节数；
6. 移动构造：`fp_ = other.fp_; other.fp_ = nullptr;`；
7. 移动赋值：`if (this != &other) { close(); fp_ = other.fp_; other.fp_ = nullptr; }`；
8. `write` 里注意 `data.size()` 可能是 0，也可能包含 `\0`，所以要用 `fwrite(data.data(), 1, data.size(), fp_)` 并检查返回值是否等于 `data.size()`。

## 踩过的坑（真实记录）

1. **改了 `src/file.cpp` 但没保存，`ctest` 结果没变**
   现象：编辑器里明明写完了，`ctest` 还是 `1 passed / 11 failed`。
   原因：VS Code 里改的只是内存中的缓冲区，没落盘；Ninja 看时间戳没变化，直接复用了旧 `.o`。
   教训：测试结果「没反应」时，先看标签页上的未保存圆点和 `git status`，再怀疑代码。

2. **构造函数抛的异常类型不对**
   一开始用 `std::invalid_argument` 处理 `path == nullptr`，结果 `OpenWithBadArgumentsThrows` 失败。
   原因：`std::invalid_argument` 派生自 `std::logic_error`，而测试 `EXPECT_THROW(..., std::runtime_error)` 捕获不到。
   教训：**异常类型是接口的一部分**。参数本身非法用 `invalid_argument`，运行时失败（打不开文件、系统调用失败）用 `runtime_error`。

3. **`noexcept` 函数里留了 `todo()`**
   移动构造/赋值一度是空壳 + `todo()`。`noexcept` 函数里抛异常 → 直接 `std::terminate`，不是普通异常传播。
   教训：标了 `noexcept` 的函数，内部不能有任何可能抛出的路径。

4. **全部实现后没删 `todo()`，编译失败**
   报错：`error: 'void raii::{anonymous}::todo(const char*)' defined but not used [-Werror=unused-function]`。
   教训：占位炸弹用完要拆；`-Werror` 会把「未使用的函数」也变成编译错误。

5. **`read_all()` 空函数体，编译失败**
   报错：`control reaches end of non-void function [-Werror=return-type]`。返回类型不是 `void`，就每条路径都得有 `return`。

## 做完之后（写进笔记，下次口述给我听）

1. 为什么 `File` 的移动构造可以 `noexcept`，而 `std::string` 的移动构造也 `noexcept`？
2. 如果 `File` 允许拷贝，`std::vector<File> v{...}` 在扩容时会发生什么？
3. 析构函数里要不要 `throw`？为什么它默认是 `noexcept` 的？
