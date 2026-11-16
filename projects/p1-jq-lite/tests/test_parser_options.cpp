// 第 8 周扩展：可配置的最大嵌套深度（ParseOptions）。
//
// 验收：本文件 4 个测试全绿，且全量 ctest 不回归（JSONTestSuite 的 500 层用例仍要能过）。
#include <gtest/gtest.h>

#include <cstddef>
#include <string>

#include "jq/error.hpp"
#include "jq/parser.hpp"

namespace {

// 生成 depth 层嵌套数组：[[[...]]]
std::string nested_arrays(std::size_t depth) {
    return std::string(depth, '[') + std::string(depth, ']');
}

}  // namespace

TEST(ParseOptionsTest, DefaultAllowsDeepButReasonableNesting) {
    EXPECT_NO_THROW(jq::parse(nested_arrays(500)));
}

TEST(ParseOptionsTest, DefaultRejectsTooDeepNesting) {
    EXPECT_THROW(jq::parse(nested_arrays(513)), jq::ParseError);
}

TEST(ParseOptionsTest, CustomMaxDepthRejectsEarlier) {
    const jq::ParseOptions options{.max_depth = 8};
    EXPECT_NO_THROW(jq::parse(nested_arrays(8), options));
    EXPECT_THROW(jq::parse(nested_arrays(9), options), jq::ParseError);
}

TEST(ParseOptionsTest, MaxDepthZeroAllowsOnlyScalars) {
    const jq::ParseOptions options{.max_depth = 0};
    EXPECT_NO_THROW(jq::parse("42", options));
    EXPECT_NO_THROW(jq::parse("\"abc\"", options));
    EXPECT_THROW(jq::parse("[]", options), jq::ParseError);
    EXPECT_THROW(jq::parse("{}", options), jq::ParseError);
}
