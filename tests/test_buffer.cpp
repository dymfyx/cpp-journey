// 验收测试：你只需要让这些测试全部通过。不要修改这个文件。
#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

#include "raii/buffer.hpp"

namespace {

std::string bytes_of(const raii::Buffer& b) {
    if (b.data() == nullptr || b.size() == 0) {
        return {};
    }
    return std::string(b.data(), b.size());
}

bool all_zero(const raii::Buffer& b) {
    for (std::size_t i = 0; i < b.size(); ++i) {
        if (b.data()[i] != '\0') {
            return false;
        }
    }
    return true;
}

}  // namespace

TEST(BufferTest, DefaultConstructedIsEmpty) {
    raii::Buffer b;
    EXPECT_EQ(b.size(), 0u);
    EXPECT_TRUE(b.empty());
    EXPECT_EQ(b.data(), nullptr);
}

TEST(BufferTest, SizedConstructorAllocatesZeroedBytes) {
    raii::Buffer b(8);
    ASSERT_EQ(b.size(), 8u);
    ASSERT_NE(b.data(), nullptr);
    EXPECT_TRUE(all_zero(b));
}

TEST(BufferTest, ConstructFromBytesCopiesThem) {
    raii::Buffer b("hello", 5);
    EXPECT_EQ(b.size(), 5u);
    EXPECT_EQ(bytes_of(b), "hello");
}

TEST(BufferTest, NullDataWithNonZeroSizeThrows) {
    EXPECT_THROW(raii::Buffer(nullptr, 3), std::invalid_argument);
    raii::Buffer empty(nullptr, 0);  // size == 0 时不读 data，允许
    EXPECT_TRUE(empty.empty());
}

TEST(BufferTest, CopyConstructMakesDeepCopy) {
    raii::Buffer a("abc", 3);
    raii::Buffer b(a);
    EXPECT_EQ(bytes_of(b), "abc");
    EXPECT_NE(a.data(), b.data());  // 必须是两块不同的内存
    a[0] = 'X';
    EXPECT_EQ(bytes_of(b), "abc");  // 改 a 不能影响 b
}

TEST(BufferTest, CopyAssignMakesDeepCopy) {
    raii::Buffer big("0123456789", 10);
    raii::Buffer small("ab", 2);
    small = big;
    EXPECT_EQ(small.size(), 10u);
    EXPECT_EQ(bytes_of(small), "0123456789");
    EXPECT_NE(small.data(), big.data());
    big[0] = 'X';
    EXPECT_EQ(bytes_of(small), "0123456789");
}

TEST(BufferTest, CopyAssignSelfIsSafe) {
    raii::Buffer a("abc", 3);
    raii::Buffer& alias = a;
    a = alias;
    EXPECT_EQ(a.size(), 3u);
    EXPECT_EQ(bytes_of(a), "abc");
}

TEST(BufferTest, MoveConstructStealsTheBlock) {
    raii::Buffer a("abcd", 4);
    char* original = a.data();
    raii::Buffer b(std::move(a));
    EXPECT_EQ(b.data(), original);  // 移动不允许重新分配、拷贝
    EXPECT_EQ(bytes_of(b), "abcd");
    EXPECT_TRUE(a.empty());
    EXPECT_EQ(a.data(), nullptr);
}

TEST(BufferTest, MoveAssignReleasesOldBlock) {
    raii::Buffer a("AAAA", 4);
    char* stolen = a.data();
    raii::Buffer b("BBBBBBBBBB", 10);  // b 原来这块必须被释放，否则 ASan 报泄漏
    b = std::move(a);
    EXPECT_EQ(b.data(), stolen);
    EXPECT_EQ(bytes_of(b), "AAAA");
    EXPECT_TRUE(a.empty());
}

TEST(BufferTest, SelfMoveAssignIsSafe) {
    raii::Buffer a("abcd", 4);
    char* original = a.data();
    a = std::move(a);
    EXPECT_EQ(a.data(), original);
    EXPECT_EQ(bytes_of(a), "abcd");
}

TEST(BufferTest, ResizeGrowsAndZeroFillsTail) {
    raii::Buffer b("abc", 3);
    b.resize(5);
    EXPECT_EQ(b.size(), 5u);
    EXPECT_EQ(bytes_of(b), std::string("abc\0\0", 5));
}

TEST(BufferTest, ResizeShrinksAndKeepsPrefix) {
    raii::Buffer b("abcdef", 6);
    b.resize(2);
    EXPECT_EQ(b.size(), 2u);
    EXPECT_EQ(bytes_of(b), "ab");
}

TEST(BufferTest, ResizeToZeroReleases) {
    raii::Buffer b("abc", 3);
    b.resize(0);
    EXPECT_TRUE(b.empty());
    EXPECT_EQ(b.data(), nullptr);
}

TEST(BufferTest, IndexAccessReadsAndWrites) {
    raii::Buffer b(3);
    b[0] = 'x';
    b[1] = 'y';
    b[2] = 'z';
    const raii::Buffer& cb = b;
    EXPECT_EQ(cb[0], 'x');
    EXPECT_EQ(cb[2], 'z');
}

TEST(BufferTest, SwapExchangesBlocks) {
    raii::Buffer a("AAAA", 4);
    raii::Buffer b("BB", 2);
    char* pa = a.data();
    char* pb = b.data();
    a.swap(b);
    EXPECT_EQ(a.data(), pb);
    EXPECT_EQ(b.data(), pa);
    EXPECT_EQ(bytes_of(a), "BB");
    EXPECT_EQ(bytes_of(b), "AAAA");
}
