#include <cstdint>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include <sys/socket.h>

#include "p2/echo_server.hpp"
#include "p2/socket.hpp"

namespace {

std::string recv_exact(p2::Socket& s, std::size_t bytes) {
    std::string out;
    out.resize(bytes);
    std::size_t got = 0;
    while (got < bytes) {
        const ssize_t n = ::recv(s.fd(), out.data() + got, bytes - got, 0);
        if (n <= 0) {
            break;
        }
        got += static_cast<std::size_t>(n);
    }
    out.resize(got);
    return out;
}

TEST(EchoServerTest, EchoesSingleMessage) {
    p2::EchoServer server(0);
    const std::uint16_t port = server.port();
    ASSERT_NE(port, 0);

    std::thread runner([&server] { server.run(); });

    p2::Socket client = p2::Socket::connect_tcp(port);
    const std::string msg = "hello, echo\n";
    ASSERT_EQ(::send(client.fd(), msg.data(), msg.size(), 0),
              static_cast<ssize_t>(msg.size()));

    EXPECT_EQ(recv_exact(client, msg.size()), msg);

    server.stop();
    runner.join();
}

TEST(EchoServerTest, EchoesMultipleClients) {
    p2::EchoServer server(0);

    std::thread runner([&server] { server.run(); });

    for (int i = 0; i < 3; ++i) {
        p2::Socket client = p2::Socket::connect_tcp(server.port());
        const std::string msg = "client-" + std::to_string(i);
        ASSERT_EQ(::send(client.fd(), msg.data(), msg.size(), 0),
                  static_cast<ssize_t>(msg.size()));
        EXPECT_EQ(recv_exact(client, msg.size()), msg);
    }

    server.stop();
    runner.join();
}

}  // namespace
