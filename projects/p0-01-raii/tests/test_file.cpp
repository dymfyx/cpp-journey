// 验收测试：你只需要让这些测试全部通过。不要修改这个文件。
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "raii/file.hpp"

namespace {

// 每次调用生成一个全新的临时文件路径（同一台机器上不会冲突）。
std::string make_temp_path(const char* tag) {
    static std::mt19937_64 rng{std::random_device{}()};
    auto dir = std::filesystem::temp_directory_path();
    auto name = std::string("p0-01-") + tag + "-" + std::to_string(rng()) + ".bin";
    auto path = dir / name;
    std::filesystem::remove(path);
    return path.string();
}

// 测试结束时自动删掉临时文件。
class TempPath {
public:
    explicit TempPath(const char* tag) : path_(make_temp_path(tag)) {}
    ~TempPath() { std::error_code ec; std::filesystem::remove(path_, ec); }
    TempPath(const TempPath&) = delete;
    TempPath& operator=(const TempPath&) = delete;
    const std::string& path() const { return path_; }

private:
    std::string path_;
};

}  // namespace

TEST(FileTest, DefaultConstructedIsNotOpen) {
    raii::File f;
    EXPECT_FALSE(f.is_open());
}

TEST(FileTest, OpenMissingFileThrows) {
    const std::string path = make_temp_path("missing");
    EXPECT_THROW(raii::File f(path.c_str(), "r"), std::runtime_error);
}

TEST(FileTest, OpenWithBadArgumentsThrows) {
    const std::string path = make_temp_path("bad");
    EXPECT_THROW(raii::File f(nullptr, "r"), std::runtime_error);
    EXPECT_THROW(raii::File g("", "r"), std::runtime_error);
    EXPECT_THROW(raii::File h(path.c_str(), ""), std::runtime_error);
}

TEST(FileTest, CloseIsIdempotent) {
    TempPath tmp("close");
    raii::File f(tmp.path().c_str(), "w");
    ASSERT_TRUE(f.is_open());
    f.close();
    EXPECT_FALSE(f.is_open());
    f.close();  // 第二次必须安全，不能 double-close
    EXPECT_FALSE(f.is_open());
}

TEST(FileTest, WriteThenReadBack) {
    TempPath tmp("roundtrip");
    {
        raii::File f(tmp.path().c_str(), "w");
        f.write("hello, modern c++\n");
        f.write("second line\n");
    }  // 析构时必须自动关闭并 flush
    raii::File r(tmp.path().c_str(), "r");
    EXPECT_EQ(r.read_all(), "hello, modern c++\nsecond line\n");
}

TEST(FileTest, EmptyFileReadsEmpty) {
    TempPath tmp("empty");
    { raii::File f(tmp.path().c_str(), "w"); }
    raii::File r(tmp.path().c_str(), "r");
    EXPECT_EQ(r.read_all(), "");
}

TEST(FileTest, ReadIntoReturnsCountAndZeroAtEof) {
    TempPath tmp("readinto");
    {
        raii::File f(tmp.path().c_str(), "w");
        f.write("abcdef");
    }
    raii::File r(tmp.path().c_str(), "r");
    char buf[4] = {};
    EXPECT_EQ(r.read_into(buf, sizeof(buf)), 4u);
    EXPECT_EQ(std::string(buf, 4), "abcd");
    EXPECT_EQ(r.read_into(buf, sizeof(buf)), 2u);
    EXPECT_EQ(r.read_into(buf, sizeof(buf)), 0u);
}

TEST(FileTest, MoveConstructTransfersOwnership) {
    TempPath tmp("move");
    raii::File a(tmp.path().c_str(), "w");
    ASSERT_TRUE(a.is_open());
    raii::File b(std::move(a));
    EXPECT_TRUE(b.is_open());
    EXPECT_FALSE(a.is_open());
    b.write("moved");
    a.close();  // 已经空了的对象再 close 也必须安全
}

TEST(FileTest, MoveAssignReleasesPreviousHandle) {
    TempPath one("moveassign-a");
    TempPath two("moveassign-b");
    {
        raii::File a(one.path().c_str(), "w");
        a.write("AAAA");
        raii::File b(two.path().c_str(), "w");
        b.write("BBBB");
        b = std::move(a);  // b 原来的句柄必须被关掉，否则写不进 two
        EXPECT_TRUE(b.is_open());
        EXPECT_FALSE(a.is_open());
        b.write("!");
    }
    raii::File r2(two.path().c_str(), "r");
    EXPECT_EQ(r2.read_all(), "BBBB");
    raii::File r1(one.path().c_str(), "r");
    EXPECT_EQ(r1.read_all(), "AAAA!");
}

TEST(FileTest, SelfMoveAssignIsSafe) {
    TempPath tmp("selfmove");
    raii::File f(tmp.path().c_str(), "w");
    ASSERT_TRUE(f.is_open());
    raii::File& alias = f;
    f = std::move(alias);
    EXPECT_TRUE(f.is_open());
    f.write("still alive");
}

TEST(FileTest, BinaryDataIsPreserved) {
    TempPath tmp("binary");
    const std::string data("a\0b\0c", 5);
    {
        raii::File f(tmp.path().c_str(), "wb");
        f.write(data);
    }
    raii::File r(tmp.path().c_str(), "rb");
    EXPECT_EQ(r.read_all(), data);
}

TEST(FileTest, WriteOnClosedFileThrows) {
    TempPath tmp("closed");
    raii::File f(tmp.path().c_str(), "w");
    f.close();
    EXPECT_THROW(f.write("x"), std::runtime_error);
    EXPECT_THROW(f.read_all(), std::runtime_error);
}
