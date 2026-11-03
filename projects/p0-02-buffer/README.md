# P0-02 · Buffer（拷贝语义 / Rule of Five）

阶段 0 的第二个练习：手写一个管理堆内存的字节缓冲区，把「拷贝 / 移动 / 自赋值 / 强异常安全」一次练熟。

## 任务

只改一个文件：`src/buffer.cpp`。

`include/raii/buffer.hpp` 是定好的接口，**不要改**；`tests/test_buffer.cpp` 是验收测试，**也不要改**。

一共 15 个测试，全绿就算过关。

## 为什么练这个

`p0-01` 的 `File` 是**禁止拷贝**的（两个对象共用 `FILE*` 会 double-close），所以它只练了移动。

`Buffer` 不一样：它管理的是**可以安全复制的内存**，所以五个特殊成员函数（析构、拷贝构造、拷贝赋值、移动构造、移动赋值）**全都要自己写**。这就是所谓 "Rule of Five"。

这里面的三个核心知识点，后面 P1–P6 每天都在用：

1. **深拷贝 vs 浅拷贝**：默认生成的拷贝构造只是把指针值抄一遍，两个 `Buffer` 指向同一块内存，析构时 `delete[]` 两次 → 堆损坏。必须自己分配新内存 + `memcpy`。
2. **移动 = 窃取 + 置空**：移动不分配内存，直接把指针拿过来，再把源置空。所以移动必须是 `noexcept`，这样 `std::vector<Buffer>` 扩容时才敢用移动（否则它宁可拷贝）。
3. **强异常安全**：拷贝赋值时 `new` 可能抛 `bad_alloc`。要求是「要么成功，要么 `*this` 保持原样」——所以顺序必须是**先分配新内存 → 拷贝 → 再释放旧内存**，不能先 `delete[]` 再分配。

## 和 File 的对照

| | File | Buffer |
|---|---|---|
| 资源 | `FILE*` | `new char[]` |
| 拷贝 | `= delete` | 必须深拷贝 |
| 移动 | 接管指针 | 接管指针 |
| 析构 | `fclose` | `delete[]` |
| 难点 | errno 错误处理 | 内存泄漏 / double free / 强异常安全 |

## 怎么跑

```bash
cd /mnt/c/Users/dym54/Desktop/c++项目/projects/p0-02-buffer

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

第一次跑应该看到 **15 个测试全红**（报 `TODO: 还没有实现 Buffer::...`），说明工程链路通了。

## 验收标准

- `ctest` 全绿（15/15）；
- 编译 **0 warning**（`-Werror` 已开）；
- **ASan/UBSan 不报错**：这条比测试本身更重要——内存管理写错时，测试可能碰巧过，但 ASan 会报 `heap-use-after-free` / `double-free` / `LeakSanitizer: detected memory leaks`；
- 不允许用 `std::vector` / `std::string` 偷懒实现内部存储（练习的是裸 `new[]` / `delete[]`）。

## 实现提示（卡住了再看）

1. `sized ctor`：`size == 0` 就直接返回（保持 `nullptr`），否则 `data_ = new char[size]{}; size_ = size;`。`new char[size]{}` 会自动清零。
2. 从字节构造：先判 `data == nullptr && size > 0` → 抛 `std::invalid_argument`；然后 `std::memcpy`。
3. 拷贝构造：**不要**直接赋值指针。`new char[other.size_]{}` + `memcpy`，再把 `size_` 设好。
4. 拷贝赋值：两种写法都行——
   - 手动版：`if (this == &other) return *this;` → 新开一块 → `memcpy` → `delete[] data_` → 接管；
   - copy-and-swap 版（更优雅）：`Buffer tmp(other); swap(tmp); return *this;`，离开作用域时 `tmp` 析构会释放旧内存。
5. 移动构造：`data_ = other.data_; size_ = other.size_; other.data_ = nullptr; other.size_ = 0;`（注意成员有默认初始值，可以直接在函数体里赋值）。
6. 移动赋值：`if (this == &other) return *this;` → `delete[] data_;` → 接管 → 置空 other。
7. `resize`：`new_size == 0` → `delete[] data_; data_ = nullptr; size_ = 0;`；变大/变小都用「新开一块 → 拷贝 `min(size_, new_size)` 字节 → 释放旧的 → 接管」的顺序，保证强异常安全。
8. 全部实现完之后，**删掉匿名命名空间里的 `todo()`**，否则 `-Wunused-function` + `-Werror` 会报错（和 p0-01 那次一样）。

## 做完之后（口述给我听，或写进 README 的「踩过的坑」）

1. 为什么移动构造必须把 `other` 置空？如果不置空，程序会在什么时刻崩？
2. 拷贝赋值里如果先 `delete[] data_` 再 `new`，当 `new` 抛 `bad_alloc` 时发生什么？
3. `std::vector<Buffer>` 扩容时，为什么「移动构造不是 `noexcept`」会让它退化成拷贝？

## 踩过的坑（真实记录）

1. **拷贝构造写成了「移动」**
   一开始直接 `data_ = other.data_; size_ = other.size_;` 再把 `other.data_ = nullptr;`，以为是深拷贝。
   实际：这是移动语义——两个对象指向同一块内存（别名），而且 `other` 是 `const Buffer&`，对它的成员赋值根本编译不过。
   教训：拷贝 = 新开内存 + 复制内容；移动 = 窃取指针 + 源置空。两者不能混。

2. **移动构造忘了把 `other.size_` 置 0**
   只清了 `other.data_`，源对象变成「指针为空、长度还是旧值」的脑裂状态：`empty()` 返回 false，之后 `resize()` 还会拿空指针去 `memcpy`。
   教训：移动后的源对象必须和默认构造出来的状态**完全一致**（`nullptr` + 0）。

3. **移动赋值：没释放旧内存 + 自赋值判断错**
   - 直接 `data_ = other.data_;` 覆盖旧指针 → 旧内存永久泄漏（ASan 的 `LeakSanitizer` 会报）；
   - 自赋值判断写成 `other.data_ == data_`（比较指针内容）而不是 `this == &other`（比较对象身份），`a = std::move(a)` 时会把对象清空。
   教训：移动赋值 = 先释放自己 → 再接管别人；自赋值判断永远是 `this == &other`。

4. **拷贝赋值 `return;` 不带返回值**
   返回类型是 `Buffer&`，却写了 `return;` → 编译错误；同时旧内存没释放，`new` 抛异常时 `size_` 已被改坏（没有强异常安全）。
   正解：copy-and-swap —— `if (this == &other) return *this; Buffer tmp(other); swap(tmp); return *this;`，既异常安全又天然处理自赋值。

5. **`resize` 里把新指针删掉了**
   ```cpp
   char* data = new char[new_size]{};
   ...
   data_ = data;
   delete[] data;   // 删的是刚分配的新内存！
   ```
   结果：`data_` 悬空（use-after-free），旧内存永远泄漏。
   教训：命名上区分新旧（`fresh`），`delete[]` 只删「自己原来那块」。

6. **`resize(0)` 没有单独处理**
   约定是 `size() == 0` ⇒ `data() == nullptr`，而 `new char[0]` 返回的是非空合法指针，必须走「释放 + 置空」分支。

7. **`todo()` 全部实现后忘记删**
   和 p0-01 一样：`-Werror=unused-function` 编译失败。
