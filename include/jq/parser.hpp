// p1 jq-lite: JSON 文本 -> Value。语法遵循 RFC 8259。
#pragma once

#include <string_view>

#include "jq/value.hpp"

namespace jq {

// 解析一整段 JSON 文本。输入必须是「一个值 + 可选空白」，有多余内容就抛 ParseError。
// 失败时抛 ParseError，其 what() 带行号和列号。
Value parse(std::string_view text);

}  // namespace jq
