#include "jq/pointer.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "jq/parser.hpp"

namespace {

using jq::evaluate_pointer;
using jq::parse;
using jq::parse_pointer;
using jq::QueryError;

jq::Value make_doc() {
    return parse(R"({"a":{"b":1},"arr":[10,20],"1":"numeric-key","a/b":"slash","a~b":"tilde"})");
}

TEST(PointerTest, ParseEmptyPointerIsRoot) {
    EXPECT_TRUE(parse_pointer("").empty());
}

TEST(PointerTest, ParseRootSlashIsOneEmptyKey) {
    const auto tokens = parse_pointer("/");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0], "");
}

TEST(PointerTest, ParseSimplePath) {
    const auto tokens = parse_pointer("/a/b");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], "b");
}

TEST(PointerTest, ParseArrayIndexToken) {
    const auto tokens = parse_pointer("/arr/0");
    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], "arr");
    EXPECT_EQ(tokens[1], "0");
}

TEST(PointerTest, ParseUnescapeSlash) {
    const auto tokens = parse_pointer("/a~1b");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0], "a/b");
}

TEST(PointerTest, ParseUnescapeTilde) {
    const auto tokens = parse_pointer("/a~0b");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0], "a~b");
}

TEST(PointerTest, ParseUnescapeOrderMatters) {
    const auto tokens = parse_pointer("/~01");
    ASSERT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0], "~1");
}

TEST(PointerTest, ParseMustStartWithSlash) {
    EXPECT_THROW(parse_pointer("a/b"), QueryError);
}

TEST(PointerTest, ParseRejectsInvalidEscape) {
    EXPECT_THROW(parse_pointer("/a~2b"), QueryError);
    EXPECT_THROW(parse_pointer("/a~"), QueryError);
}

TEST(PointerTest, EvaluateRootReturnsWholeDocument) {
    const auto doc = make_doc();
    EXPECT_EQ(evaluate_pointer(doc, ""), doc);
}

TEST(PointerTest, EvaluateNestedKey) {
    EXPECT_EQ(evaluate_pointer(make_doc(), "/a/b").as_number(), 1.0);
}

TEST(PointerTest, EvaluateArrayIndex) {
    EXPECT_EQ(evaluate_pointer(make_doc(), "/arr/1").as_number(), 20.0);
}

TEST(PointerTest, EvaluateNumericTokenOnObjectIsKey) {
    EXPECT_EQ(evaluate_pointer(make_doc(), "/1").as_string(), "numeric-key");
}

TEST(PointerTest, EvaluateEscapedTokens) {
    EXPECT_EQ(evaluate_pointer(make_doc(), "/a~1b").as_string(), "slash");
    EXPECT_EQ(evaluate_pointer(make_doc(), "/a~0b").as_string(), "tilde");
}

TEST(PointerTest, EvaluateMissingKeyThrows) {
    EXPECT_THROW(evaluate_pointer(make_doc(), "/nope"), QueryError);
}

TEST(PointerTest, EvaluateIndexOutOfRangeThrows) {
    EXPECT_THROW(evaluate_pointer(make_doc(), "/arr/2"), QueryError);
}

TEST(PointerTest, EvaluateLeadingZeroIndexThrows) {
    EXPECT_THROW(evaluate_pointer(make_doc(), "/arr/01"), QueryError);
}

TEST(PointerTest, EvaluateNonNumericTokenOnArrayThrows) {
    EXPECT_THROW(evaluate_pointer(make_doc(), "/arr/x"), QueryError);
}

TEST(PointerTest, EvaluateKeyOnScalarThrows) {
    EXPECT_THROW(evaluate_pointer(make_doc(), "/a/b/x"), QueryError);
}

TEST(PointerTest, EvaluateSameAsJqPathForSimpleDocument) {
    const auto doc = make_doc();
    EXPECT_EQ(evaluate_pointer(doc, "/a"), doc.as_object().at("a"));
}

}  // namespace
