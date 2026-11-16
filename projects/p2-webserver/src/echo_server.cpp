// p2 第 1 周作业：单线程 epoll echo 服务器。
//
// 结构建议：
//   EchoServer ctor：
//     listener_ = Socket::create_tcp();
//     listener_.set_nonblocking();
//     listener_.bind(port);
//     listener_.listen();
//     epoll_.add(listener_.fd(), EPOLLIN);
//
//   run()：
//     running_ = true;
//     while (running_) {
//         for (const auto& ev : epoll_.wait(100)) {     // 100ms 超时，便于 stop() 生效
//             if (ev.fd == listener_.fd()) { accept_all(); continue; }
//             if (ev.events & (EPOLLERR | EPOLLHUP)) { close_conn(ev.fd); continue; }
//             if (ev.events & EPOLLIN) { echo_once(ev.fd); }
//         }
//     }
//
//   accept_all()：循环 accept() 直到 nullopt；每个新连接 set_nonblocking() 后 add(EPOLLIN | EPOLLET)。
//   echo_once(fd)：循环 read 直到 EAGAIN（ET 必须读到空）；read 返回 0 → 关闭连接；
//                  读到的字节直接 write 回去（第 1 周不考虑部分写，W5 再处理）。

#include "p2/echo_server.hpp"

#include <cerrno>
#include <stdexcept>
#include <string>

#include <sys/epoll.h>
#include <unistd.h>

namespace p2 {
namespace {

[[noreturn]] void todo(const char* what) {
    throw std::logic_error(std::string("TODO: 还没有实现 EchoServer::") + what);
}

}  // namespace

EchoServer::EchoServer(std::uint16_t port) {
    (void)port;
    todo("EchoServer");
}

EchoServer::~EchoServer() noexcept = default;

void EchoServer::run() { todo("run"); }

void EchoServer::stop() noexcept { running_.store(false); }

std::uint16_t EchoServer::port() const noexcept { return listener_.local_port(); }

}  // namespace p2
