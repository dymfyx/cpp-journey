#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace p2 {

// epoll_wait 返回的一条就绪事件。
struct EpollEvent {
    int fd = -1;
    std::uint32_t events = 0;   // EPOLLIN / EPOLLOUT / EPOLLERR / ...
};

// epoll fd 的 RAII 封装。
// 事件常量在 .cpp 里用 <sys/epoll.h>，头文件不暴露系统头，保持干净。
class Epoll {
public:
    Epoll();
    ~Epoll();

    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;
    Epoll(Epoll&& other) noexcept;
    Epoll& operator=(Epoll&& other) noexcept;

    int fd() const noexcept;

    void add(int fd, std::uint32_t events);       // EPOLL_CTL_ADD
    void modify(int fd, std::uint32_t events);    // EPOLL_CTL_MOD
    void remove(int fd);                          // EPOLL_CTL_DEL

    // 最多等 timeout_ms 毫秒；返回这次就绪的事件列表。
    // 超时返回空 vector（不要抛异常）。
    std::vector<EpollEvent> wait(int timeout_ms);

private:
    static constexpr std::size_t kMaxEvents = 64;
    int fd_ = -1;
};

}  // namespace p2
