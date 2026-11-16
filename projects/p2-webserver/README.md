# 第 1 周 · P2：epoll HTTP 服务器

第 1 周目标：把 socket / epoll 封装好，跑通一个**单线程 epoll echo server**，并用测试证明它真的能收能发。

后面几周要在这个骨架上长东西：HTTP 解析（W3）、线程池与主从 Reactor（W5–6）、`/predict` 动态 batching（W7）、压测（W8）。

## 目录

```
p2-webserver/
├── CMakeLists.txt
├── include/p2/
│   ├── socket.hpp        # socket fd 的 RAII 封装（★ 第 1 周）
│   ├── epoll.hpp         # epoll fd 的 RAII 封装（★ 第 1 周）
│   └── echo_server.hpp   # 单线程 echo 服务器（★ 第 1 周）
├── src/
│   ├── socket.cpp        # 你要写
│   ├── epoll.cpp         # 你要写
│   ├── echo_server.cpp   # 你要写
│   └── main.cpp          # 手动测试入口
└── tests/
    ├── test_socket.cpp
    ├── test_epoll.cpp
    └── test_echo_server.cpp
```

## 快速开始

```bash
cd projects/p2-webserver
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

现在**全部失败是正常的**——`src/*.cpp` 里每个函数都是 `todo()`，抛 `std::logic_error`。你要把它们一个个实现掉。

手动玩一下（实现完之后）：

```bash
./build/p2_echo --port 8080
# 另一个终端
printf 'hello\n' | nc 127.0.0.1 8080
```

## 分步计划

| 步 | 内容 | 验收 |
|---|---|---|
| 1 | `Socket`：RAII 包装 fd、TCP 创建/绑定/监听/连接、非阻塞、`release`、移动语义 | `test_socket` 全绿 |
| 2 | `Epoll`：add / modify / remove / wait（超时） | `test_epoll` 全绿 |
| 3 | `EchoServer`：epoll 循环 + accept + 读 + 回写 | `test_echo_server` 全绿 |
| 4 | 手动 `nc` 验证 + 连续多个客户端 | 不崩、不断连 |

## 第 1 周要搞懂的几件事

1. **RAII 的意义**：`Socket` 析构自动 `close`；移动后源对象必须置 `-1`，否则 double close 会关掉别人的 fd。
2. **非阻塞 + ET**：`accept` 要循环到 `EAGAIN`；`read` 返回 0 表示对端关闭，返回 -1 且 `EAGAIN` 表示现在没数据。
3. **epoll 的注册关系**：fd 关闭前必须先 `epoll_ctl(DEL)`（或依赖自动删除，但显式更安全）。
4. **错误处理**：系统调用失败抛 `std::system_error(errno, std::generic_category(), "...")`，不要返回 -1 让调用方猜。
5. **测试怎么测网络**：`socketpair()` 造一对已连接的 fd 测 epoll；真实 TCP 用 `port = 0` 让内核分配端口，再 `getsockname` 取回来。

## 提示

- 头文件里的接口**不要改**，测试依赖它们；
- 每个 `todo()` 实现完就删掉，否则 `-Werror=unused-function` 会报错；
- 先让 `test_socket` 绿，再写 `Epoll`，最后 `EchoServer`——一层一层来，别一次全写。
