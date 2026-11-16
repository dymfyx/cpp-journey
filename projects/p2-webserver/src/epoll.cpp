// p2 第 1 周作业：epoll 的 RAII 封装。

#include "p2/epoll.hpp"

#include <cerrno>
#include <stdexcept>
#include <string>
#include <system_error>

#include <sys/epoll.h>
#include <unistd.h>

namespace p2 {
namespace {

[[noreturn]] void todo(const char* what) {
    throw std::logic_error(std::string("TODO: 还没有实现 Epoll::") + what);
}

[[maybe_unused]] [[noreturn]] void fail(const char* context) {
    throw std::system_error(errno, std::generic_category(), context);
}

}  // namespace

Epoll::Epoll() {
    // 提示：epoll_create1(EPOLL_CLOEXEC)，失败 fail("epoll_create1")。
    todo("Epoll");
}

Epoll::~Epoll() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

Epoll::Epoll(Epoll&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

Epoll& Epoll::operator=(Epoll&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

int Epoll::fd() const noexcept { return fd_; }

void Epoll::add(int fd, std::uint32_t events) {
    (void)fd;
    (void)events;
    // 提示：epoll_ctl(fd_, EPOLL_CTL_ADD, fd, &ev)，ev.data.fd = fd，ev.events = events。
    todo("add");
}

void Epoll::modify(int fd, std::uint32_t events) {
    (void)fd;
    (void)events;
    // 提示：EPOLL_CTL_MOD。
    todo("modify");
}

void Epoll::remove(int fd) {
    (void)fd;
    // 提示：EPOLL_CTL_DEL。注意：fd 关闭前先 DEL 更安全。
    todo("remove");
}

std::vector<EpollEvent> Epoll::wait(int timeout_ms) {
    (void)timeout_ms;
    // 提示：std::array<epoll_event, kMaxEvents> events; int n = epoll_wait(fd_, events.data(), kMaxEvents, timeout_ms);
    //       n < 0 时：EINTR → 返回空 vector（让调用方继续循环），其他 → fail；
    //       把前 n 条转成 EpollEvent{ev.data.fd, ev.events}。
    todo("wait");
}

}  // namespace p2
