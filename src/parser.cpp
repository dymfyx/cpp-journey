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

#include <stdexcept>
#include <string>

#include "jq/error.hpp"

namespace jq {

namespace {

[[noreturn]] void todo(const char* what) {
    throw std::logic_error(std::string("TODO: 还没有实现 ") + what);
}

}  // namespace

Value parse(std::string_view text) {
    (void)text;
    todo("jq::parse");
}

}  // namespace jq
