// p1 jq-lite 第 2 周验收测试：序列化（dump）。你不需要改这个文件。
//
// 这 12 个测试描述的是 dump() 的「规格」：
//   - dump()      紧凑模式（indent = -1）
//   - dump(2)     缩进模式，每层 2 空格
//   - dump(0)     换行但不缩进
//
// 现有实现（src/value.cpp）已经能让这 12 个测试全绿；
// 你的任务是不看实现、自己重写 dump，让它们继续全绿。
#include "jq/value.hpp"

#include <gtest/gtest.h>

#include <string>

#include "jq/parser.hpp"

namespace {

using jq::Array;
using jq::Object;
using jq::Value;

TEST(DumpTest, DumpScalars) {
    EXPECT_EQ(Value().dump(), "null");
    EXPECT_EQ(Value(true).dump(), "true");
    EXPECT_EQ(Value(false).dump(), "false");
    EXPECT_EQ(Value(std::string("abc")).dump(), "\"abc\"");
    EXPECT_EQ(Value(std::string()).dump(), "\"\"");
}

TEST(DumpTest, DumpNumbers) {
    EXPECT_EQ(Value(42).dump(), "42");
    EXPECT_EQ(Value(1.0).dump(), "1");
    EXPECT_EQ(Value(-3.25).dump(), "-3.25");
    EXPECT_EQ(Value(100.0).dump(), "100");
    EXPECT_EQ(Value(0.1).dump(), "0.10000000000000001");
}

TEST(DumpTest, DumpStringEscapes) {
    EXPECT_EQ(Value(std::string("a\"b\\c")).dump(), "\"a\\\"b\\\\c\"");
}

TEST(DumpTest, DumpControlCharacters) {
    const std::string raw = "a\nb\tc\rd\be\fe";
    EXPECT_EQ(Value(raw).dump(), "\"a\\nb\\tc\\rd\\be\\fe\"");
    EXPECT_EQ(Value(std::string(1, '\x01')).dump(), "\"\\u0001\"");
}

TEST(DumpTest, DumpEmptyContainers) {
    EXPECT_EQ(Value(Array{}).dump(), "[]");
    EXPECT_EQ(Value(Object{}).dump(), "{}");
    EXPECT_EQ(Value(Array{}).dump(2), "[]");
    EXPECT_EQ(Value(Object{}).dump(2), "{}");
}

TEST(DumpTest, DumpCompactArray) {
    const Value value = Value(Array{Value(1), Value(2), Value(3)});
    EXPECT_EQ(value.dump(), "[1,2,3]");
}

TEST(DumpTest, DumpCompactObject) {
    const Value value = Value(Object{{"a", Value(1)}, {"b", Value(2)}});
    EXPECT_EQ(value.dump(), "{\"a\":1,\"b\":2}");
}

TEST(DumpTest, DumpPrettyArray) {
    const Value value = Value(Array{Value(1), Value(2)});
    EXPECT_EQ(value.dump(2), "[\n  1,\n  2\n]");
    EXPECT_EQ(value.dump(0), "[\n1,\n2\n]");
}

TEST(DumpTest, DumpPrettyObject) {
    const Value value = Value(Object{{"a", Value(1)}});
    EXPECT_EQ(value.dump(2), "{\n  \"a\": 1\n}");
}

TEST(DumpTest, DumpPrettyNested) {
    const Value value = Value(Object{{"a", Value(Array{Value(1)})}});
    EXPECT_EQ(value.dump(2), "{\n  \"a\": [\n    1\n  ]\n}");
}

TEST(DumpTest, DumpUnicodePassThrough) {
    EXPECT_EQ(Value(std::string("\u4e2d\u6587")).dump(), "\"\u4e2d\u6587\"");
}

TEST(DumpTest, DumpRoundTrip) {
    const std::string text = R"({"a":[1,true,null],"b":{"c":"d"}})";
    const Value value = jq::parse(text);
    const std::string dumped = value.dump();
    EXPECT_EQ(jq::parse(dumped), value);
    EXPECT_EQ(jq::parse(dumped).dump(), dumped);
}

}  // namespace
