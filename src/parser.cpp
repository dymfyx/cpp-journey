// p1 jq-lite 第 1 周：你要实现的就是这个文件。
//
// 目标：把一个 std::string_view 解析成 jq::Value，失败抛 jq::ParseError（带行号列号）。
//
// 建议的递归下降骨架（不强制，但先想清楚再动手）：
//
//   parse_value()   -> 跳过空白，看第一个字符决定分派：
//                      '{' -> parse_object()
//                      '[' -> parse_array()
//                      '"' -> parse_string()
//                      't'/'f' -> true / false
//                      'n'     -> null
//                      否则  parse_number()
//   parse_object()  -> '{' 空白 ( '}' | string 空白 ':' 空白 value ( 空白 ',' ... ) 空白 '}' )
//   parse_array()   -> '[' 空白 ( ']' | value ( 空白 ',' ... ) 空白 ']' )
//   parse_string()  -> 处理转义：\" \\ \/ \b \f \n \r \t 和 \uXXXX
//   parse_number()  -> 符号/整数/小数/指数，可用 std::from_chars 转 double
//
// 实现要点：
//   - 用「当前位置 + 行列号」两个成员记录进度；遇到 '\n' 时行号 +1、列号归 1。
//   - 错误信息里带上实际遇到的字符，例如：expected ',' or '}' but got 'x'。
//   - \uXXXX 需要转成 UTF-8 字节（BMP 之外的代理对 week 3 再处理，先用 &#92;u 直接转 BMP）。
//   - 数字语法必须严格：不允许前导 0、不允许 .5、不允许 1.、不允许 +1、不允许 NaN/Infinity。
//   - 解析完一个值后必须吃掉尾部空白，且不能有剩余字符。
//
// 验收：cmake --build build && ctest --test-dir build --output-on-failure
//       目标 13/13 全绿。
#include "jq/parser.hpp"

#include <charconv>  
#include <cstddef>
#include <string>
#include <string_view>

#include "jq/error.hpp"

namespace jq {
namespace {

class Parser {
public:
    explicit Parser(std::string_view text) : text_(text) {}

    Value parse_document() {
        Value value = parse_value();
        skip_whitespace();
        if (!at_end()) fail("unexpected trailing characters");
        return value;
    }

private:
    bool at_end() const { return pos_ >= text_.size(); }

    char peek() const { return at_end() ? '\0' : text_[pos_]; }

    static bool is_digit(char c) { return c >= '0' && c <= '9'; }

    char take() {
        const char c = text_[pos_++];
        if (c == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
        return c;
    }

    [[noreturn]] void fail(const std::string& message) const {
        throw ParseError(message, line_, column_);
    }

    void skip_whitespace() {
        while (!at_end() &&
               (peek() == ' ' || peek() == '\t' || peek() == '\n' || peek() == '\r')) {
            take();
        }
    }

    Value parse_value() {
        skip_whitespace();
        if (at_end()) fail("unexpected end of input");
        switch (peek()) {
            case 't': expect_literal("true");  return Value(true);
            case 'f': expect_literal("false"); return Value(false);
            case 'n': expect_literal("null");  return Value();
           default:
                if (peek() == '-' || (peek() >= '0' && peek() <= '9')) 
                    return parse_number();
                fail(std::string("unexpected character '") + peek() + "'");
        }
    }

    void expect_literal(std::string_view literal) {
        for (char expected : literal) {
            if (at_end() || take() != expected) {
                fail(std::string("invalid literal, expected \"") + std::string(literal) + "\"");
            }
        }
    }
     
    Value parse_number(){
        const std::size_t start = pos_;

        //负号可选
        if (!at_end() && peek() == '-') take();

        //0开头，读整数/小数
        if (peek() == '0') {
            take();
            if (!at_end() && is_digit(peek())) fail("invalid number: leading zero");
        }
        else if (is_digit(peek())){
            while(!at_end() && is_digit(peek())) take();
        }
        else fail("invalid number: expected digit");

        //读小数
        if (peek() == '.'){
            take();
            if (at_end() || !is_digit(peek())) fail("invalid number: expected digit after '.'");
            else while (!at_end() && is_digit(peek())) take();  
        }

        //读指数
        if (!at_end() && (peek() == 'e' || peek() == 'E')) {
            take();
            if (!at_end() && (peek() == '-' || peek() == '+')) take();
            if (at_end() || !is_digit(peek())) fail("invalid number: expected digit in exponent");
            else while (!at_end() && is_digit(peek())) take();
        }

        double value = 0.0;
        const char* first = text_.data() + start;
        const char* last  = text_.data() + pos_;
        const auto [ptr, ec] = std::from_chars(first, last, value);
        if (ec != std::errc{} || ptr != last) fail("invalid number");
        return Value(value);
    }

    std::string_view text_;
    std::size_t pos_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;
};

}  // namespace

Value parse(std::string_view text) {
    Parser parser(text);
    return parser.parse_document();
}

}  // namespace jq
