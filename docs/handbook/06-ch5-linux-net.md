# 第 5 章 Linux、文件描述符与网络

> 这一章的问题来自 WSL 环境搭建和 P2（epoll HTTP 服务器）的起步。核心结论：**Linux 里「一切皆文件」，而 socket 的内核对象正是通过 fd 这个「句柄号」被你操作。**

## Q5.1 为什么在 Windows 上用 WSL2 而不是 MinGW

**疑问**：MinGW 的 g++ 也能编译，为什么还要装 WSL？

**解答**：

| 能力 | MinGW（Windows） | WSL2（Linux） |
|---|---|---|
| ASan/UBSan/TSan | ❌ 不支持 | ✅ 原生支持 |
| epoll/io_uring | ❌（Windows 是 IOCP） | ✅ |
| 系统调用、内核对象（socket 状态机、fd 表） | 看不到 | 全可见 |
| perf / strace / gdb 生态 | 弱 | 强 |
| 部署一致性（云服务器） | ❌ | ✅ |
| 动态链接运行 | 沙箱环境下受限 | 正常 |

本项目所有编译、测试、基准都在 WSL 里跑；Windows 侧只负责 VS Code 界面和 `git push`（WSL 连不上 github.com:443）。

**一句话**：学系统级 C++ 就必须在 Linux 上跑，WSL2 是「不换电脑的 Linux」。

**项目里**：`scripts/import-ubuntu.ps1`（WSL 安装脚本）、`docs/HANDOFF.md` 环境章节。

## Q5.2 fd（文件描述符）是什么

**疑问**：`ls -l /proc/$$/fd` 里那些 0/1/2/27 是什么？

**解答**：fd 是**进程 fd 表的下标**，本质是「内核对象的句柄号」：

```
用户进程                     内核
fd 0 ──→ struct file ──→ /dev/pts/3（终端）
fd 1 ──→ struct file ──→ /dev/pts/3
fd 2 ──→ struct file ──→ /dev/pts/3
fd 27 ──→ struct file ──→ vscode-server 的日志文件
```

- 0/1/2 固定是 **stdin / stdout / stderr**（终端程序都指向同一个 pty）；
- 27 是 `vscode-server` 启动时打开的日志文件，被你的 shell 继承；
- socket 会显示 `socket:[inode]`，epoll 显示 `anon_inode:[eventpoll]`，管道显示 `pipe:[inode]`。

四个作用：

1. 后续系统调用的第一个参数（`read(fd, ...)`）；
2. 属性容器（`fcntl` 设置非阻塞、`getsockopt` 读选项）；
3. 统一抽象：文件/管道/socket 都能 `epoll`；
4. 生命周期：`close(fd)` 释放；fd 号会被复用（所以先 `EPOLL_CTL_DEL` 再 `close`）。

**一句话**：fd 是号码，内核对象是房间；你只能通过号码让内核办事。

**项目里**：`Socket` 类持有一个 `int fd_`；`::close(fd_)` 在析构函数里释放。

## Q5.3 socket 对象在内核里长什么样

**疑问**：socket 对象到底装了什么？为什么说它「有状态」？

**解答**：内核里的数据结构（简化）：

```
struct file → struct socket → struct sock → struct tcp_sock
```

字段大致有：

| 内容 | 作用 |
|---|---|
| 状态机 | `CLOSED`/`LISTEN`/`SYN_SENT`/`ESTABLISHED`/`CLOSE_WAIT`/`TIME_WAIT` |
| 本地/对端地址 | IP + 端口（未 `bind`/`connect` 时为 0） |
| 收发缓冲区 | 内核队列；`recv`/`send` 只是搬进搬出 |
| 选项 | `SO_REUSEADDR`、`TCP_NODELAY`、`SO_RCVBUF`、非阻塞标志 |
| 等待队列 | `epoll` 的回调就挂在这里（可读/可写事件） |
| 引用计数 | 有几个 fd 指向它 |

`accept()` 返回的新 fd 指向**内核新创建**的 socket 对象（`ESTABLISHED`），不是监听对象本身。

**一句话**：监听 socket 是「总机」，每条连接是独立的内核对象；用户态只有句柄。

**项目里**：`projects/p2-webserver/src/socket.cpp`；`include/p2/socket.hpp` 的 `Socket(int fd)` 就是包这个句柄。

## Q5.4 TCP 为什么是「字节流」——粘包从哪来

**疑问**：`recv` 一次能收到一条完整消息吗？

**解答**：**不能**。TCP 只保证字节顺序，不保证消息边界：

- 发送方两次 `send("hello")`、`send("world")`，接收方可能一次收到 `"helloworld"`，也可能分三次收到；
- 原因：TCP 是内核缓冲区里的**字节流**，Nagle 合并、MTU 分片、网络延迟都会改变切分点。

三种解决方案：

| 方案 | 例子 |
|---|---|
| 长度前缀 | `[4 字节长度][内容]`（二进制协议常用） |
| 分隔符 | 换行、`\r\n\r\n`（HTTP 头） |
| 关闭连接表示结束 | `shutdown(SHUT_WR)` |

HTTP 用 **Content-Length** 或 **chunked** 编码；你的 `test_echo_server` 里 `recv_exact` 循环收满 N 字节，就是「长度前缀」思想。

**一句话**：TCP 给你「水管」，不给你「信封」；消息边界要自己在应用层定义。

**项目里**：`projects/p2-webserver/tests/test_echo_server.cpp` 的 `recv_exact`；第 2 周做 HTTP 请求解析时会再遇到。

## Q5.5 `epoll` 为什么比 `select`/`poll` 快

**疑问**：epoll 是啥？为什么服务器都用它？

**解答**：

| | `select` | `poll` | `epoll` |
|---|---|---|---|
| 复杂度 | O(n) 每次调用 | O(n) 每次调用 | O(1) 添加/删除，`epoll_wait` 返回就绪的 |
| fd 上限 | 1024 | 无硬上限 | 无硬上限 |
| 数据拷贝 | 每次全量拷贝 fd 集合 | 每次全量拷贝 | 内核注册一次，只拷贝就绪事件 |
| 触发模式 | 水平触发 | 水平触发 | **支持边缘触发（ET）** |

epoll 三个调用：

```cpp
int epfd = ::epoll_create1(EPOLL_CLOEXEC);          // 创建实例
::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);          // 注册/修改/删除
::epoll_wait(epfd, events, maxevents, timeout_ms);  // 等事件
```

**LT vs ET**（常考）：

- LT（水平触发）：只要缓冲区有数据，每次 `epoll_wait` 都返回；
- ET（边缘触发）：只在状态**变化**时返回一次，必须**循环读到 `EAGAIN`** 才不丢数据；配合非阻塞 fd 使用。

**一句话**：epoll 把「每次问所有人」变成「内核告诉你谁有事」；ET 快但要读干净。

**项目里**：`include/p2/epoll.hpp`（Week 1 接口）；第 2 周实现。

## Q5.6 非阻塞 fd 与 `EAGAIN`

**疑问**：为什么服务器要把 fd 设成非阻塞？

**解答**：`fcntl(fd, F_SETFL, flags | O_NONBLOCK)` 之后：

- `read` 没有数据时立刻返回 `-1`，`errno == EAGAIN`（或 `EWOULDBLOCK`），而不是阻塞整个线程；
- 这样**一个线程**就能用 epoll 管理成千上万连接（事件驱动）。

ET 模式下必须：

```cpp
while (true) {
    const ssize_t n = ::read(fd, buf, sizeof buf);
    if (n > 0) { /* 处理 */ continue; }
    if (n == 0) { /* 对端关闭 */ break; }
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;   // 读干净了
    /* 其他 errno：错误 */
}
```

另一个常坑：`EINTR`（被信号打断）要重试。

**一句话**：非阻塞 + EAGAIN 循环 = ET 模式的正确姿势。

**项目里**：`Socket::set_nonblocking()`（用 `fcntl`）；Week 1 测试 `NonBlockingReadReturnsEagain`。

## Q5.7 `::` 与系统调用

**疑问**：为什么写 `::socket`、`::close`，不直接 `socket`/`close`？

**解答**：`::` 表示「全局命名空间」。在成员函数里写 `close(fd_)` 会命中 `Socket::close` 自己 → 无限递归；写 `::close(fd_)` 才是 C 标准库函数。

另外 `::socket` 明确告诉读者「这是系统调用」，也避免 ADL（参数依赖查找）把同名函数找进来。

**一句话**：`::` 是「别找成员，找全局」的显式声明。

**项目里**：`src/socket.cpp` 的 `close()`、`create_tcp()`、`bind()`。

## Q5.8 字节序：`htons`/`ntohl` 为什么必须用

**疑问**：端口号为什么要转换？

**解答**：网络字节序是**大端**（big-endian），x86 主机是**小端**。`sockaddr_in` 里的端口/地址必须用网络序：

```cpp
sockaddr_in addr{};
addr.sin_family = AF_INET;                  // 地址族（主机序，不需要转换）
addr.sin_port = htons(port);                // 端口：host → network short
addr.sin_addr.s_addr = htonl(INADDR_ANY);   // 地址：host → network long
```

读回来时用 `ntohs`/`ntohl` 反向转换。漏掉会得到「端口 0 或乱码」。

**一句话**：跨机器传输的整数统一用大端；`hton*`/`ntoh*` 是转换函数。

**项目里**：`Socket::bind` 和 `local_port`（`getsockname` + `ntohs`）。

## Q5.9 一次完整的服务器启动序列

**疑问**：从零到能接受连接，步骤是什么？

**解答**：

```cpp
const int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);  // 1. 造端点
int one = 1;
::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);      // 2. 允许复用地址
sockaddr_in addr{}; addr.sin_family = AF_INET;
addr.sin_port = htons(8080); addr.sin_addr.s_addr = htonl(INADDR_ANY);
::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr);       // 3. 绑地址
::listen(fd, SOMAXCONN);                                           // 4. 开始监听
::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL, 0) | O_NONBLOCK);        // 5. 非阻塞
int epfd = ::epoll_create1(EPOLL_CLOEXEC);                         // 6. 建 epoll
epoll_event ev{.events = EPOLLIN, .data = {.fd = fd}};
::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);                         // 7. 注册
while (true) {
    const int n = ::epoll_wait(epfd, events, 64, -1);              // 8. 等事件
    // 9. accept 新连接 / 读写数据
}
```

`SO_REUSEADDR` 的意义：服务器重启时 `TIME_WAIT` 状态的端口还能立刻复用，否则要等 60 秒。

**一句话**：socket → setsockopt → bind → listen → epoll → accept/read/write，顺序别乱。

**项目里**：`projects/p2-webserver/src/socket.cpp`、`src/echo_server.cpp`（Week 1）。

## Q5.10 本章检查表

- [ ] 能说出 WSL 相比 MinGW 的四个不可替代点
- [ ] 能解释 fd 是「进程 fd 表的下标」以及它的四个用途
- [ ] 能画出 socket 对象在内核里的关键字段
- [ ] 能举出「粘包」的三个原因 和 三种消息边界方案
- [ ] 能说出 epoll 比 select/poll 快的两点原因
- [ ] 能写出 ET 模式「读到 EAGAIN」的循环
- [ ] 能默写服务器启动的 8 个步骤
