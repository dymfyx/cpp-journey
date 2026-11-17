# 第 1 章 语言基础

> 这一章的问题全部来自 `p0-01-raii`（File 封装）和 `p0-02-buffer`（Buffer 五大特殊成员）两个项目。它们是我从「OI 式写代码」转向「资源有主、对象有生命周期」的分水岭。

## Q1.1 RAII 是什么？为什么 File 类要这么写

**我当时的疑问**：`File` 不就是一个 `FILE*` 加几个函数吗，为什么还要写构造函数、析构函数、删掉拷贝？

**解答**：RAII = Resource Acquisition Is Initialization，**资源在构造时获取，在析构时释放**。

```cpp
class File {
public:
    explicit File(const char* path, const char* mode) {
        fp_ = std::fopen(path, mode);
        if (fp_ == nullptr) {
            throw std::runtime_error(std::string("File: failed to open ") + path +
                                     ": " + std::strerror(errno));
        }
    }
    ~File() noexcept { close(); }          // 无论正常返回、异常、提前 return，都会执行
    File(const File&) = delete;            // 独占资源：禁止拷贝
    File& operator=(const File&) = delete;
    File(File&&) noexcept;                 // 允许移动（转移所有权）
    File& operator=(File&&) noexcept;
private:
    std::FILE* fp_ = nullptr;              // 句柄初始化为空
};
```

三个关键点：

1. **析构函数是唯一保证被执行的地方**：栈展开会调用析构（见 Q1.10），所以「忘记 fclose」这个 bug 被彻底消灭；
2. **构造失败不泄漏**：`fopen` 失败时对象没构造完，析构不执行——但此时也没拿到资源，所以安全；
3. **拷贝必须禁**：两个 `File` 指向同一个 `FILE*` 会 double close。要复制内容就开新文件，不是复制对象。

**一句话**：RAII 把「资源」绑定到「对象生命周期」上，让编译器替你调用释放。

**项目里**：`projects/p0-01-raii/include/raii/file.hpp`，测试 `FileTest.OpenMissingFileThrows` / `MoveConstructTransfersOwnership`。

## Q1.2 构造与析构的顺序、初始化列表

**疑问**：成员初始化列表和函数体里赋值有什么区别？`File::File(File&& other) noexcept : fp_(other.fp_) { other.fp_ = nullptr; }` 为什么这么写？

**解答**：

- 初始化列表按**成员声明顺序**执行，不是按你写的顺序；
- 函数体赋值是「先默认构造，再赋值」——对类类型成员多一次构造/析构；
- 引用、`const` 成员、无默认构造的成员**只能**在初始化列表初始化。

```cpp
class Buffer {
public:
    Buffer(std::size_t size) : size_(size) {          // 按声明顺序：data_ 先、size_ 后
        if (size == 0) return;
        data_ = new char[size]{};                     // 零初始化
    }
private:
    char* data_ = nullptr;      // 声明顺序
    std::size_t size_ = 0;
};
```

**析构顺序与构造相反**：成员按声明逆序析构，之后才轮到基类，最后是整个对象的内存释放。

**一句话**：初始化列表 = 构造那一步就位；函数体赋值 = 先造再改，多一步。

**项目里**：`p0-02-buffer/src/buffer.cpp` 的构造函数。

## Q1.3 「三法则 / 五法则 / 零法则」

**疑问**：什么时候要写析构、拷贝、移动？

**解答**：

| 法则 | 内容 | 何时适用 |
|---|---|---|
| 零法则（Rule of 0） | 不写任何特殊成员，全用 RAII 封装的标准库类型 | **首选**，绝大多数类 |
| 三法则 | 需要自定义析构 → 通常也要拷贝构造 + 拷贝赋值 | C++98 时代 |
| 五法则 | 三法则 + 移动构造 + 移动赋值 | 手写资源类（如 `File`、`Buffer`） |

你的 `Buffer` 就是五法则：拥有 `new[]` 资源，必须自己管。`jq::Value` 则是**零法则**：内部是 `std::variant<std::string, std::vector, std::map>`，编译器生成的版本全对。

**一句话**：写着写着需要自己 `delete` 了，才需要五法则；否则交给标准库。

**项目里**：`p0-02-buffer/src/buffer.cpp`（五法则）、`p1-jq-lite/include/jq/value.hpp`（零法则）。

## Q1.4 深浅拷贝：我把拷贝构造写成了「偷指针」

**当时的错误代码**（❌）：

```cpp
Buffer::Buffer(const Buffer& other) {
    if (other.data_ != nullptr) {
        data_ = other.data_;     // 只是复制指针！
        size_ = other.size_;
        other.data_ = nullptr;   // const 对象不能改，编译直接失败
    }
}
```

**为什么错**：

1. `const Buffer& other` 的成员不能赋值——编译报错；
2. 就算去掉 `const`，这也是「移动」不是「拷贝」：两个对象抢同一块内存，析构时 double free；源对象被掏空。
3. 测试 `CopyConstructMakesDeepCopy` 会立刻失败。

**正确写法**（✅）：

```cpp
Buffer::Buffer(const Buffer& other) {
    if (other.size_ == 0) return;
    data_ = new char[other.size_]{};                        // ① 新分配
    std::memcpy(data_, other.data_, other.size_);           // ② 复制内容
    size_ = other.size_;
}
```

**一句话**：拷贝 = 复制资源内容（新内存），移动 = 转移资源所有权（偷指针再置空）。

**项目里**：`p0-02-buffer/src/buffer.cpp`，测试 `CopyConstructMakesDeepCopy`。

## Q1.5 移动构造 = 浅拷贝 + 置空吗

**疑问**：`File(File&& other) : fp_(other.fp_) { other.fp_ = nullptr; }` 是不是就是「浅拷贝加个置空」？

**解答**：**结果上等价，语义上不同**。移动的约定是「资源所有权转移，源对象进入合法但未指定（moved-from）状态」。对 `std::string` 这种类型，移走之后源对象可能变成空串，也可能保留内部指针（标准不规定），所以**只能销毁或重新赋值，不能读它的值**。

```cpp
std::string a = "hello";
std::string b = std::move(a);   // a 现在合法但未指定，别用 a 的值
a = "world";                    // ✅ 重新赋值没问题
```

**一句话**：移动之后源对象「活着但被掏空」，只能析构/重新赋值，不能假设它的内容。

**项目里**：`p0-01-raii/src/file.cpp` 移动构造/赋值；`p0-02-buffer` 移动后 `other.size_ = 0` 必须一起置空（否则源对象的 `size_` 与 `data_ = nullptr` 不一致，`resize` 时 `memcpy` 空指针）。

## Q1.6 移动构造为什么要 `noexcept`

**疑问**：`noexcept` 只是修饰吗？不写会怎样？

**解答**：不是修饰，是**类型契约**，会被 `std::vector` 用来做决策：

- `vector` 扩容时要把旧元素搬到新内存；
- 如果元素的移动构造是 `noexcept`，就用**移动**（快）；
- 如果不是，为了保证「搬一半抛异常还能回滚」，只能用**拷贝**（慢，甚至编译失败如果不可拷贝）。

```cpp
Buffer(Buffer&&) noexcept;              // ✅ 扩容走移动
~Buffer() noexcept;                     // 析构默认就是 noexcept
```

**一句话**：`noexcept` 让标准库敢对你用更快的路径；写在移动函数上是明确承诺「我不会抛」。

**项目里**：`p0-02-buffer/include/raii/buffer.hpp` 的移动成员；`jq::Value` 的移动是编译器生成的（隐式 `noexcept`）。

## Q1.7 拷贝赋值：copy-and-swap 与自赋值

**错误版本**（❌，我第一版）：

```cpp
Buffer& Buffer::operator=(const Buffer& other) {
    delete[] data_;                      // 先释放自己
    data_ = new char[other.size_];       // 如果这里抛 bad_alloc，对象已损坏
    // ...
}
```

**正确版本**（✅，copy-and-swap，强异常安全）：

```cpp
Buffer& Buffer::operator=(const Buffer& other) {
    if (this == &other) return *this;     // 自赋值：a = a
    Buffer copy(other);                   // 先在临时对象里完成拷贝（可能抛）
    swap(copy);                           // 只做不抛的交换
    return *this;
}
```

好处：**若拷贝失败，this 保持原样**（强异常安全）；可拷贝/移动赋值共用一套逻辑。

**一句话**：先做好新版本，再原子替换；别「先删旧的，再建新的」。

**项目里**：`p0-02-buffer/src/buffer.cpp` 的 `operator=`。

## Q1.8 自赋值为什么要判断 `this == &other`

**疑问**：`a = a;` 谁会这么写？

**解答**：不一定是人写的——`v[i] = v[j]` 在 `i == j` 时就是自赋值。如果实现是「先 `delete[] data_` 再 `memcpy(data_, other.data_)`」，自赋值时 `other.data_` 已经指向被释放的内存 → use-after-free。

**一句话**：自赋值不是笔误问题，是别名（aliasing）问题；判断 `this == &other` 是 O(1) 的保险。

**项目里**：测试 `CopyAssignSelfIsSafe`、`SelfMoveAssignIsSafe`。

## Q1.9 异常、栈展开、`std::terminate`、`abort` 的区别

**疑问**：进程直接 abort 是什么意思？栈展开就会析构吗？

**解答**：

| 终止方式 | 栈展开 | 局部对象析构 | 进程继续 |
|---|---|---|---|
| 异常被 `catch` | ✅ | ✅ | ✅ |
| 异常无人捕获 | ❌（`terminate`） | 不保证 | ❌ abort |
| `noexcept` 函数内抛出 | ❌（`terminate`） | 不保证 | ❌ abort |
| 析构函数内抛出 | ❌（`terminate`） | 不保证 | ❌ abort |
| `abort()` / `assert` 失败 | ❌ | ❌ | ❌ |
| `exit()` | ❌ | ❌（只跑 atexit） | ❌ |
| 段错误 / ASan 报错 | ❌ | ❌ | ❌ |

关键：**异常是「可恢复的提前返回」**：从 `throw` 点向上，每层作用域的自动对象按逆序析构（栈展开，由 `.eh_frame` + landing pad 实现），直到找到 `catch`；找不到就 `std::terminate()` → 默认 `abort()`。

**一句话**：异常会析构、能恢复；abort 直接杀进程、不析构。

**项目里**：`src/value.cpp` 的 `dump` 遇到 NaN/Infinity 抛 `std::runtime_error`；`p2-webserver/src/socket.cpp` 的 `fail()` 抛 `std::system_error`。实验：故意让 `parse_string` 在 10 万层嵌套时抛异常，`Depth_guard` 的析构函数把 `depth_` 还原（见第 3 章 Q3.9）。

## Q1.10 构造/析构函数里能抛异常吗

**解答**：

- **构造函数可以抛**：对象没构造完，不会有半成品对象，但已构造的**成员**会析构（所以构造顺序里先申请的资源要注意）；
- **析构函数不能抛**：析构函数默认 `noexcept`，抛出直接 `std::terminate()`。因为析构常在栈展开过程中执行，此时再抛异常 = 两个异常同时存在，C++ 直接终止进程。

**一句话**：构造可以失败（抛），析构绝不能失败（吞掉异常、记日志）。

**项目里**：所有析构函数都标 `noexcept`，内部调用的 `fclose`/`delete[]` 不会失败。

## Q1.11 `static` 有几种含义

| 位置 | 含义 | 例子 |
|---|---|---|
| 全局/命名空间 | 内部链接（本文件可见） | 老式 C 写法，现代用匿名 namespace 代替 |
| 类内数据成员 | 所有对象共享一份 | `static std::size_t count_;` |
| 类内成员函数 | 没有 `this`，属于类 | `Socket::create_tcp()`（工厂） |
| 函数内局部变量 | 首次执行时初始化，生命周期到程序结束 | `static bool inited = false;`（线程安全初始化） |

**注意**：类内 `static constexpr` 在 C++17 起隐式 `inline`，不需要类外定义；如果改成运行期可配置（如 `ParseOptions{max_depth}`），就必须换成普通成员。

**项目里**：`p1-jq-lite/src/parser.cpp` 的 `static bool is_digit(char)`（类内静态辅助函数）→ 后来改成 `Parser` 的静态成员；`ParseOptions` 从 `static constexpr kMaxDepth = 512` 改为运行期成员。

## Q1.12 `namespace` 与匿名 `namespace`

**疑问**：`namespace jq` 和文件里的 `namespace { ... }` 有什么区别？

**解答**：

```cpp
namespace jq {          // 公开 API：防重名，全名 jq::parse、jq::Value

namespace {             // 匿名命名空间：本 .cpp 私有，等价于 static
[[noreturn]] void todo(const char* what) {   // 只在实现阶段存在的「占位炸弹」
    throw std::logic_error(std::string("TODO: ") + what);
}
}  // namespace

}  // namespace jq
```

- **匿名 namespace 的函数**只在本翻译单元可见，链接器不会导出，不会和别人重名；
- 花括号**必须配对**：我删「占位炸弹」时连同 `namespace {` 一起删了，结果 `}  // namespace` 提前关闭了 `namespace jq`，导致后面所有 `PathSegment`、`QueryError` 都「未定义」——23 个红线全是这一个括号造成的。

**一句话**：公开 API 放具名 namespace，内部辅助放匿名 namespace；括号错位会连锁报错。

**项目里**：`p1-jq-lite/src/pointer.cpp`、`src/parser.cpp`。

## Q1.13 为什么写 `::close`、`::socket`

**疑问**：`Socket::close()` 里 `close(fd_)` 不行吗？为什么要 `::close`？

**解答**：`::` 是作用域解析符，前面单写表示「全局命名空间」。不写的话，名字查找会先命中**成员函数自己**：

```cpp
void Socket::close() noexcept {
    if (fd_ >= 0) {
        close(fd_);     // ❌ 找到 Socket::close → 无限递归
        fd_ = -1;
    }
}
::close(fd_);           // ✅ 全局 C 函数
```

同理 `::socket`、`::bind`、`::read` 加 `::` 是工程惯例：让读者一眼知道是系统调用，同时避免被成员/ADL 遮住。

**一句话**：成员函数里调同名全局函数，必须 `::`，否则自己调自己。

**项目里**：`projects/p2-webserver/src/socket.cpp` 的 `close()`、`create_tcp()`。

## Q1.14 `this`、`*this`、`const` 成员函数

- `this`：指向当前对象的指针，成员函数里的隐式参数；
- `*this`：当前对象的引用（`dump_impl(*this, indent)` 就是传自己）；
- `const` 成员函数：`this` 类型变成 `const T*`，只能读不能改；
- `static` 成员函数**没有** `this`。

`File& File::operator=(File&& other) noexcept` 里的 `if (this == &other)` 判断的是「两个指针是否指向同一对象」，这是自赋值的唯一正确判据（不是比较内容）。

**项目里**：`p1-jq-lite/src/value.cpp` 的 `Value::dump` 调 `dump_impl(out, *this, indent, 0)`。

## Q1.15 为什么返回 `const std::string&` 而不是 `std::string`

**疑问**：`as_string()` 返回引用，安全吗？

**解答**：

```cpp
const std::string& Value::as_string() const {   // 引用：零拷贝
    return std::get<std::string>(storage_);
}
```

- 引用的生命周期 = 对象本身的生命周期；只要 `Value` 活着，引用就有效；
- 如果对象是**临时值**，引用就悬空：

```cpp
const std::string& s = jq::parse("\"hi\"").as_string();  // ❌ parse 返回的临时对象已销毁
std::string s = jq::parse("\"hi\"").as_string();         // ✅ 拷贝一份
```

**一句话**：返回引用省拷贝，但调用方必须保证对象还活着；绑定到临时对象就是悬空引用。

**项目里**：`include/jq/value.hpp` 的 `as_string/as_array/as_object`；测试 `ValueTest` 里都在对象存活作用域内使用。

## Q1.16 `explicit` 干什么

**解答**：禁止构造函数的**隐式转换**。

```cpp
class Buffer {
    explicit Buffer(std::size_t size);   // 防止函数传 int 时被悄悄转成 Buffer
};
void f(Buffer b);
f(10);        // ❌ 没有 explicit 就能编译：10 → 临时 Buffer
```

**什么时候加**：单参数构造（尤其 `int`/指针参数）、转换运算符。
**什么时候不加**：确实希望隐式转换（如 `std::string` 从 `const char*`）。

**项目里**：`Socket(int fd)` 是 `explicit`（防止 `Socket s = 3;`）；`Value(int)` 故意**不** explicit，让 `Value v = 42;` 自然工作。

## Q1.17 `static_cast` 与 C 风格转换

**疑问**：`static_cast<std::size_t>(...)` 是什么？为什么到处都要它？

**解答**：

- `std::size_t`：无符号整数类型（64 位系统 8 字节），`sizeof` 的返回类型；
- `static_cast<T>(x)`：编译期检查的显式转换，比 C 风格 `(T)x` 安全（能查就查，不能查才用别的转换）；
- 项目开了 `-Wconversion -Wsign-conversion -Werror`，`int` 隐式转 `size_t` 会告警并报错——所以必须写出转换：

```cpp
out.append(static_cast<std::size_t>(indent * depth), ' ');
unsigned char ch = static_cast<unsigned char>(raw);   // 避免 0x80+ 变成负数
```

**一句话**：`static_cast` 是「我明确知道这里要转」的声明；`-Wconversion` 逼你把这种声明写出来。

**项目里**：`src/value.cpp` 的 `append_indent`（`int`→`size_t`）、`dump_string`（`char`→`unsigned char`）。

## Q1.18 栈和堆到底差在哪

**解答**：

| | 栈 | 堆 |
|---|---|---|
| 分配方式 | 编译器自动（函数帧） | `new`/`malloc` 手动 |
| 大小 | 默认 8MB，编译期确定 | 受物理内存限制，运行期申请 |
| 速度 | 寄存器/指针移动，纳秒级 | 分配器查找，慢 10–100 倍 |
| 生命周期 | 作用域结束自动销毁 | 手动 `delete`，否则泄漏 |
| 典型错误 | 栈溢出（递归太深） | 泄漏、double free、悬空指针 |

进程内存布局：`.text`（代码）→ `.rodata`（常量）→ `.data`（初始化全局）→ `.bss`（未初始化全局，**OI 的大数组在这里**）→ 堆（向上长）→ 栈（向下长）。

`Buffer` 是「栈上的头 + 堆上的身体」：对象本身在栈上（`char* + size_t`），数据块在堆上（`new char[]`）——这正是 RAII 的物理画像。

**一句话**：栈管生命周期（自动），堆管大小（灵活）；RAII 就是把堆资源挂在栈对象的生命周期上。

**项目里**：`p0-02-buffer` 的 `Buffer` 对象在栈上、`data_` 在堆上；`p1-jq-lite` 的 `Value` 同理（`std::string`/`vector` 内部堆分配）。

## Q1.19 虚函数、vptr、vtable、多态

**解答**：

- **继承**：派生类 is-a 基类，复用接口/实现；
- **虚函数**：允许派生类覆盖，调用时**动态绑定**；
- **实现**：每个含虚函数的对象头部多一个 `vptr`（8 字节）指向类的 `vtable`（函数指针表）；调用 `obj->f()` 编译成「取 vptr → 取表项 → 间接调用」；
- **代价**：对象变大、不能内联、每对象可能 `new`、切片风险；
- **多态**：编译期（重载/模板/CRTP）与运行期（虚函数）两大类。

**项目里的取舍**：`jq::Value` **不用**继承，用 `std::variant`——值语义、栈上、可拷贝、可比较，代价是不能扩展新 JSON 类型（JSON 类型是封闭集合，不需要）。

**一句话**：虚函数换来运行期扩展性，代价是每个对象 8 字节 + 间接调用；封闭集合用 `variant` 更快更值语义。

**项目里**：`docs/design.md` 第 2 节「DOM 布局」；`sizeof(jq::Value) == 56`（实测，见第 2 章 Q2.1）。

## Q1.20 本章检查表

- [ ] 能说出 RAII 三要素（构造获取、析构释放、禁用拷贝）
- [ ] 能默写 Buffer 的五法则，并解释每一条的 `noexcept`
- [ ] 能讲清「拷贝 = 深拷贝、移动 = 转移 + 置空」
- [ ] 能画出异常从抛出到 catch 的栈展开过程
- [ ] 能解释 `::close` 不加 `::` 为什么会无限递归
- [ ] 能说出栈（8MB、自动、快）与堆（大、手动、慢）的三点区别
