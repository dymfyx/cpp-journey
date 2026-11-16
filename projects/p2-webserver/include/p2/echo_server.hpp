#pragma once

#include <atomic>
#include <cstdint>

#include "p2/epoll.hpp"
#include "p2/socket.hpp"

namespace p2 {

// 第 1 周的单线程 epoll echo 服务器。
// 连接进来后，读到什么就原样写回去；对端关闭就清理连接。
class EchoServer {
public:
    explicit EchoServer(std::uint16_t port = 0);   // 构造时 create/bind/listen
    ~EchoServer() noexcept;

    EchoServer(const EchoServer&) = delete;
    EchoServer& operator=(const EchoServer&) = delete;

    void run();              // 阻塞运行，直到 stop()
    void stop() noexcept;    // 可以跨线程调用
    std::uint16_t port() const noexcept;   // 实际监听端口（port=0 时由内核分配）

private:
    Socket listener_;
    Epoll epoll_;
    std::atomic<bool> running_{false};
};

}  // namespace p2
