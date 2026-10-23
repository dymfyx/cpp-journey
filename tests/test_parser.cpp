// p1 jq-lite 第 1 周验收测试：解析器。你不需要改这个文件。
#include "jq/parser.hpp"

#include <gtest/gtest.h>

#include <string>

#include "jq/error.hpp"
#include "jq/value.hpp"

namespace {

using jq::Array;
using jq::Object;
using jq::Value;

TEST(ParserTest, ParsesNull) {
    const Value value = jq::parse("null");
    EXPECT_TRUE(value.is_null());
}

TEST(ParserTest, ParsesBooleans) {
    EXPECT_TRUE(jq::parse("true").as_bool());
    EXPECT_FALSE(jq::parse("false").as_bool());
}

TEST(ParserTest, ParsesNumbers) {
    EXPECT_DOUBLE_EQ(jq::parse("42").as_number(), 42.0);
    EXPECT_DOUBLE_EQ(jq::parse("0").as_number(), 0.0);
    EXPECT_DOUBLE_EQ(jq::parse("-3.25").as_number(), -3.25);
    EXPECT_DOUBLE_EQ(jq::parse("1e3").as_number(), 1000.0);
    EXPECT_DOUBLE_EQ(jq::parse("2.5E-2").as_number(), 0.025);
    EXPECT_DOUBLE_EQ(jq::parse("1.0e+2").as_number(), 100.0);
}

TEST(ParserTest, ParsesStrings) {
    EXPECT_EQ(jq::parse("\"hello\"").as_string(), "hello");
    EXPECT_EQ(jq::parse("\"\"").as_string(), "");
}

TEST(ParserTest, ParsesStringEscapes) {
    EXPECT_EQ(jq::parse("\"a\\nb\\t\\\"c\\\\d\"").as_string(), "a\nb\t\"c\\d");
    EXPECT_EQ(jq::parse("\"\\/\"").as_string(), "/");
}

TEST(ParserTest, ParsesUnicodeEscapes) {
    EXPECT_EQ(jq::parse("\"\\u4e2d\\u6587\"").as_string(), "\u4e2d\u6587");
}

TEST(ParserTest, ParsesEmptyContainers) {
    EXPECT_TRUE(jq::parse("[]").as_array().empty());
    EXPECT_TRUE(jq::parse("{}").as_object().empty());
}

TEST(ParserTest, ParsesNestedStructures) {
    const Value value = jq::parse(R"({"a":[1,true,null],"b":{"c":"d"}})");
    ASSERT_TRUE(value.is_object());
    const Object& members = value.as_object();
    ASSERT_EQ(members.size(), 2U);
    const Array& items = members.at("a").as_array();
    ASSERT_EQ(items.size(), 3U);
    EXPECT_DOUBLE_EQ(items[0].as_number(), 1.0);
    EXPECT_TRUE(items[1].as_bool());
    EXPECT_TRUE(items[2].is_null());
    EXPECT_EQ(members.at("b").as_object().at("c").as_string(), "d");
}

TEST(ParserTest, IgnoresWhitespaceEverywhere) {
    const Value value = jq::parse(" \n\t\r { \"a\" : [ 1 , 2 ] } \n");
    ASSERT_TRUE(value.is_object());
    const Array& items = value.as_object().at("a").as_array();
    ASSERT_EQ(items.size(), 2U);
    EXPECT_DOUBLE_EQ(items[0].as_number(), 1.0);
    EXPECT_DOUBLE_EQ(items[1].as_number(), 2.0);
}

TEST(ParserTest, RejectsTrailingGarbage) {
    EXPECT_THROW(jq::parse("{} x"), jq::ParseError);
    EXPECT_THROW(jq::parse("[1] 2"), jq::ParseError);
}

TEST(ParserTest, RejectsInvalidInput) {
    EXPECT_THROW(jq::parse(""), jq::ParseError);
    EXPECT_THROW(jq::parse("   "), jq::ParseError);
    EXPECT_THROW(jq::parse("\"unterminated"), jq::ParseError);
    EXPECT_THROW(jq::parse("[1,]"), jq::ParseError);
    EXPECT_THROW(jq::parse("{\"a\":}"), jq::ParseError);
    EXPECT_THROW(jq::parse("nul"), jq::ParseError);
    EXPECT_THROW(jq::parse("01"), jq::ParseError);
    EXPECT_THROW(jq::parse(".5"), jq::ParseError);
    EXPECT_THROW(jq::parse("1."), jq::ParseError);
}

TEST(ParserTest, ErrorCarriesLineAndColumn) {
    try {
        jq::parse("\n  {\"a\": }");
        FAIL() << "expected ParseError";
    } catch (const jq::ParseError& error) {
        EXPECT_EQ(error.line(), 2U);
        EXPECT_GE(error.column(), 8U);
    }
}

TEST(ParserTest, RoundTripsThroughDump) {
    const std::string text = R"({"a":[1,true,null],"b":{"c":"d"}})";
    const Value value = jq::parse(text);
    EXPECT_EQ(jq::parse(value.dump()).dump(), value.dump());
}

}  // namespace
