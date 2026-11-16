#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

namespace p2 {

// socket fd 的 RAII 封装：构造/接管时持有 fd，析构时 close。
//
// 规则：
//   - 禁止拷贝（两个对象不能同时拥有一个 fd）；
//   - 移动后源对象必须变成无效（fd_ = -1），否则 double close 会误关别人的 fd；
//   - 失败一律抛 std::system_error(errno, std::generic_category(), "上下文")，
//     不要返回 -1 让调用方猜。
class Socket {
public:
    Socket() noexcept = default;
    explicit Socket(int fd) noexcept;   // 接管一个已有 fd
    ~Socket() noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    int fd() const noexcept;
    bool valid() const noexcept;
    void close() noexcept;          // 幂等：重复调用安全

    // 交出所有权：返回 fd 并把自身置空，调用方负责 close。
    int release() noexcept;

    void set_nonblocking();                          // fcntl(O_NONBLOCK)
    void bind(std::uint16_t port);                   // 绑定 0.0.0.0:port，port=0 由内核分配
    void listen(int backlog = 128);                  // listen(2)
    std::optional<Socket> accept();                  // 非阻塞：EAGAIN 时返回 nullopt
    std::uint16_t local_port() const;                // getsockname 取实际端口

    static Socket create_tcp();                      // socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0)
    static Socket connect_tcp(std::uint16_t port, const std::string& host = "127.0.0.1");
    static std::pair<Socket, Socket> create_pair();  // socketpair(AF_UNIX, SOCK_STREAM) 测试用

private:
    int fd_ = -1;
};

}  // namespace p2
