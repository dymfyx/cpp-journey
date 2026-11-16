#include <cstring>
#include <utility>

#include <gtest/gtest.h>

#include <sys/epoll.h>
#include <unistd.h>

#include "p2/epoll.hpp"
#include "p2/socket.hpp"

namespace {

TEST(EpollTest, WaitTimesOutOnEmpty) {
    p2::Epoll epoll;
    const auto events = epoll.wait(50);
    EXPECT_TRUE(events.empty());
}

TEST(EpollTest, ReportsReadableSocketpairEnd) {
    auto [a, b] = p2::Socket::create_pair();
    p2::Epoll epoll;
    epoll.add(a.fd(), EPOLLIN);

    const char msg[] = "x";
    ASSERT_EQ(::write(b.fd(), msg, 1), 1);

    const auto events = epoll.wait(1000);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0].fd, a.fd());
    EXPECT_NE(events[0].events & EPOLLIN, 0U);
}

TEST(EpollTest, RemoveStopsReporting) {
    auto [a, b] = p2::Socket::create_pair();
    p2::Epoll epoll;
    epoll.add(a.fd(), EPOLLIN);
    epoll.remove(a.fd());

    const char msg[] = "x";
    ASSERT_EQ(::write(b.fd(), msg, 1), 1);

    EXPECT_TRUE(epoll.wait(100).empty());
}

}  // namespace
