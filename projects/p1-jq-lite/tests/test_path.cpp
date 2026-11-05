#include <gtest/gtest.h>

#include "jq/parser.hpp"
#include "jq/path.hpp"
#include "jq/value.hpp"

namespace {

// 把 JSON 文本解析成 Value 树，测试里反复要用。
jq::Value doc(std::string_view text) { return jq::parse(text); }

}  // namespace

// ---------------------------------------------------------------------------
// parse_path 的语法解析
// ---------------------------------------------------------------------------

TEST(PathTest, ParsePathSegmentsShape) {
    const auto segs = jq::parse_path(".users[1].id");
    ASSERT_EQ(segs.size(), 3u);
    EXPECT_FALSE(segs[0].is_index);
    EXPECT_EQ(segs[0].key, "users");
    EXPECT_TRUE(segs[1].is_index);
    EXPECT_EQ(segs[1].index, 1u);
    EXPECT_FALSE(segs[2].is_index);
    EXPECT_EQ(segs[2].key, "id");
}

TEST(PathTest, ParseBareDotIsEmptyPath) {
    EXPECT_TRUE(jq::parse_path(".").empty());
}

TEST(PathTest, ParseRootArrayIndex) {
    const auto segs = jq::parse_path(".[2]");
    ASSERT_EQ(segs.size(), 1u);
    EXPECT_TRUE(segs[0].is_index);
    EXPECT_EQ(segs[0].index, 2u);
}

// ---------------------------------------------------------------------------
// evaluate_path 的正常路径
// ---------------------------------------------------------------------------

TEST(PathTest, IdentityReturnsWholeDocument) {
    const auto v = doc(R"({"a":1,"b":[2]})");
    EXPECT_EQ(jq::evaluate_path(v, ".").dump(), R"({"a":1,"b":[2]})");
}

TEST(PathTest, SimpleKeyReturnsString) {
    const auto v = doc(R"({"name":"dym","age":19})");
    EXPECT_EQ(jq::evaluate_path(v, ".name").dump(), "\"dym\"");
    EXPECT_EQ(jq::evaluate_path(v, ".age").dump(), "19");
}

TEST(PathTest, NestedKeys) {
    const auto v = doc(R"({"a":{"b":{"c":42}}})");
    EXPECT_EQ(jq::evaluate_path(v, ".a.b.c").dump(), "42");
}

TEST(PathTest, ArrayIndexInsideObject) {
    const auto v = doc(R"({"items":[10,20,30]})");
    EXPECT_EQ(jq::evaluate_path(v, ".items[1]").dump(), "20");
}

TEST(PathTest, RootArrayIndex) {
    const auto v = doc(R"([1,2,3])");
    EXPECT_EQ(jq::evaluate_path(v, ".[0]").dump(), "1");
    EXPECT_EQ(jq::evaluate_path(v, ".[2]").dump(), "3");
}

TEST(PathTest, MixedChain) {
    const auto v = doc(R"({"users":[{"id":1},{"id":2}]})");
    EXPECT_EQ(jq::evaluate_path(v, ".users[1].id").dump(), "2");
}

TEST(PathTest, ReturnsSubtreeForContainer) {
    const auto v = doc(R"({"a":{"x":1,"y":[true,null]}})");
    EXPECT_EQ(jq::evaluate_path(v, ".a").dump(), R"({"x":1,"y":[true,null]})");
    EXPECT_EQ(jq::evaluate_path(v, ".a.y").dump(), R"([true,null])");
}

// ---------------------------------------------------------------------------
// evaluate_path 的错误路径
// ---------------------------------------------------------------------------

TEST(PathTest, MissingKeyThrows) {
    const auto v = doc(R"({"a":1})");
    EXPECT_THROW(jq::evaluate_path(v, ".missing"), jq::QueryError);
}

TEST(PathTest, IndexOutOfRangeThrows) {
    const auto v = doc(R"({"items":[10,20]})");
    EXPECT_THROW(jq::evaluate_path(v, ".items[5]"), jq::QueryError);
}

TEST(PathTest, IndexOnObjectThrows) {
    const auto v = doc(R"({"name":"x"})");
    EXPECT_THROW(jq::evaluate_path(v, ".name[0]"), jq::QueryError);
}

TEST(PathTest, KeyOnArrayThrows) {
    const auto v = doc(R"({"items":[1,2]})");
    EXPECT_THROW(jq::evaluate_path(v, ".items.x"), jq::QueryError);
}

TEST(PathTest, QueryOnScalarThrows) {
    const auto v = doc("42");
    EXPECT_THROW(jq::evaluate_path(v, ".x"), jq::QueryError);
}

TEST(PathTest, MalformedPathThrows) {
    const auto v = doc(R"({"a":1})");
    EXPECT_THROW(jq::evaluate_path(v, ""), jq::QueryError);
    EXPECT_THROW(jq::evaluate_path(v, "a"), jq::QueryError);      // 必须以 '.' 开头
    EXPECT_THROW(jq::evaluate_path(v, ".["), jq::QueryError);     // 没闭合
    EXPECT_THROW(jq::evaluate_path(v, ".[abc]"), jq::QueryError); // 下标不是数字
    EXPECT_THROW(jq::evaluate_path(v, ".a["), jq::QueryError);
    EXPECT_THROW(jq::evaluate_path(v, ".a]"), jq::QueryError);    // 多余的 ']'
}
