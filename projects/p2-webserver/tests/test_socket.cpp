#include <cstring>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include <unistd.h>

#include "p2/socket.hpp"

namespace {

TEST(SocketTest, DefaultIsInvalid) {
    const p2::Socket s;
    EXPECT_FALSE(s.valid());
    EXPECT_EQ(s.fd(), -1);
}

TEST(SocketTest, CreateTcpIsValid) {
    const p2::Socket s = p2::Socket::create_tcp();
    EXPECT_TRUE(s.valid());
    EXPECT_GE(s.fd(), 0);
}

TEST(SocketTest, MoveTransfersOwnership) {
    p2::Socket a = p2::Socket::create_tcp();
    const int fd = a.fd();
    p2::Socket b(std::move(a));
    EXPECT_FALSE(a.valid());
    EXPECT_EQ(b.fd(), fd);
}

TEST(SocketTest, CloseIsIdempotent) {
    p2::Socket s = p2::Socket::create_tcp();
    s.close();
    s.close();
    EXPECT_FALSE(s.valid());
}

TEST(SocketTest, PairIsConnected) {
    auto [a, b] = p2::Socket::create_pair();
    const char msg[] = "hi";
    ASSERT_EQ(::write(a.fd(), msg, 2), 2);

    char buf[8] = {};
    ASSERT_EQ(::read(b.fd(), buf, sizeof buf), 2);
    EXPECT_EQ(std::string(buf, 2), "hi");
}

TEST(SocketTest, BindListenExposesPort) {
    p2::Socket s = p2::Socket::create_tcp();
    s.set_nonblocking();
    s.bind(0);
    s.listen();
    EXPECT_NE(s.local_port(), 0);
}

}  // namespace
