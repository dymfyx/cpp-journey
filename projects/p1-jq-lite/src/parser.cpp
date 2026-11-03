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

    unsigned read_hex4() {
        unsigned value = 0;
        for (int i = 0; i < 4; ++i) {
            if (at_end()) fail("invalid unicode escape");
            const char c = take();
            value <<= 4;
            if (c >= '0' && c <= '9')      value |= static_cast<unsigned>(c - '0');
            else if (c >= 'a' && c <= 'f') value |= static_cast<unsigned>(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') value |= static_cast<unsigned>(c - 'A' + 10);
            else fail("invalid unicode escape: expected hex digit");
        }
        return value;
    }

    std::string encode_utf8(unsigned codepoint) {
        std::string out;
        if (codepoint <= 0x7F) {
            out.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7FF) {
            out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        return out;
    }

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
            case '"': return Value(parse_string());
            case '[': return parse_array();
            case '{': return parse_object();
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

    std::string parse_string() {
        take();
        std::string out;
        while (true) {
            if (at_end()) fail("unterminated string");

            const char c = peek();
            if (c == '"') {
                take();
                return out;
            }

            if (c == '\\') {
                take();
                if (at_end()) fail("unterminated escape");
                switch (take()) {
                    case '"':  out.push_back('"');  break;
                    case '\\': out.push_back('\\'); break;
                    case '/':  out.push_back('/');  break;
                    case 'b':  out.push_back('\b'); break;
                    case 'f':  out.push_back('\f'); break;
                    case 'n':  out.push_back('\n'); break;
                    case 'r':  out.push_back('\r'); break;
                    case 't':  out.push_back('\t'); break;
                    case 'u':  out += encode_utf8(read_hex4()); break;
                    default:   fail("invalid escape sequence");
                }
                continue;
            }
            if (static_cast<unsigned char>(c) < 0x20) fail("control character in string");

            out.push_back(take());
        }
    }

    Value parse_array() {
        take();
        Array items;
        skip_whitespace();
        if(!at_end() && peek() == ']'){
            take();
            return Value(std::move(items));
        }
        while (true) {
            items.push_back(parse_value());
            skip_whitespace();
            if(at_end()) fail("unterminated array");
            if(peek() == ','){
                take();
                continue;
            }
            if(peek() == ']'){
                take();
                return Value(std::move(items));
            }
            fail("expected ',' or ']' in array");
        }
    }

    Value parse_object() {
        take(); 
        Object members;
        skip_whitespace();
        if(!at_end() && peek() == '}'){
            take();
            return Value(std::move(members));
        }
        while (true) {
            skip_whitespace();
            if(at_end()) fail("unterminated object");
            if(peek() != '"') fail("expected string key in object");
            std::string key = parse_string();
            skip_whitespace();
            if (at_end()) fail("unterminated object");
            if(peek() != ':') fail("expected ':' after object key");
            else take();
            Value value = parse_value();
            members.insert_or_assign(std::move(key), std::move(value));
            skip_whitespace();
            if(at_end()) fail("unterminated object");
            if(peek() == ','){
                take();
                continue;
            }
            if(peek() == '}'){
                take();
                return Value(std::move(members));
            }
            fail("expected ',' or '}' in object");
        }
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
