// p2 第 1 周作业：Socket 的 RAII 封装。
// 每个 todo() 换成真实实现，做完删掉 todo()（否则 -Werror=unused-function）。

#include "p2/socket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace p2 {
namespace {

[[noreturn]] void todo(const char* what) {
    throw std::logic_error(std::string("TODO: 还没有实现 Socket::") + what);
}

[[maybe_unused]] [[noreturn]] void fail(const char* context) {
    throw std::system_error(errno, std::generic_category(), context);
}

}  // namespace

Socket::Socket(int fd) noexcept : fd_(fd) {}

Socket::~Socket() noexcept { close(); }

void Socket::close() noexcept {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

int Socket::fd() const noexcept { return fd_; }

bool Socket::valid() const noexcept { return fd_ >= 0; }

int Socket::release() noexcept {
    const int fd = fd_;
    fd_ = -1;
    return fd;
}

// ---- 以下函数要你来写 ----

Socket Socket::create_tcp() {
    // 提示：socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)，失败 fail("socket")。
    todo("create_tcp");
}

Socket Socket::connect_tcp(std::uint16_t port, const std::string& host) {
    // 提示：inet_pton 解析 host；connect 循环处理 EINTR。
    (void)port;
    (void)host;
    todo("connect_tcp");
}

std::pair<Socket, Socket> Socket::create_pair() {
    // 提示：int fds[2]; socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds);
    todo("create_pair");
}

void Socket::set_nonblocking() {
    // 提示：fcntl(fd_, F_GETFL, 0) | O_NONBLOCK，失败 fail("fcntl")。
    todo("set_nonblocking");
}

void Socket::bind(std::uint16_t port) {
    // 提示：sockaddr_in{AF_INET, htons(port), INADDR_ANY}；设置 SO_REUSEADDR（用 int opt=1, setsockopt）。
    (void)port;
    todo("bind");
}

void Socket::listen(int backlog) {
    // 提示：listen(fd_, backlog)，失败 fail("listen")。
    (void)backlog;
    todo("listen");
}

std::optional<Socket> Socket::accept() {
    // 提示：accept4(fd_, nullptr, nullptr, SOCK_CLOEXEC)；
    //       返回 -1 时：EAGAIN/EWOULDBLOCK → nullopt，EINTR → 重试，其他 → fail。
    todo("accept");
}

std::uint16_t Socket::local_port() const {
    // 提示：getsockname 到 sockaddr_in，返回 ntohs(sin_port)。
    todo("local_port");
}

}  // namespace p2
