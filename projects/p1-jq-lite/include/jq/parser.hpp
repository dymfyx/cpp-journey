// p1 jq-lite: JSON 文本 -> Value。语法遵循 RFC 8259。
#pragma once

#include <cstddef>
#include <string_view>

#include "jq/value.hpp"

namespace jq {

// 解析选项。默认值不是 RFC 8259 的规定，而是本实现的工程选择（见 docs/design.md 第 4 节）。
struct ParseOptions {
    // 允许的最大嵌套深度（数组、对象各自计一层）。
    //   - 默认 512：真实数据通常 < 10 层，同时要能接受 JSONTestSuite 里 500 层的用例；
    //   - 超过上限抛 ParseError，而不是让递归把调用栈耗尽（那会 abort 整个进程）；
    //   - 0 表示不允许任何容器，只接受标量。
    std::size_t max_depth = 512;
};

// 解析一整段 JSON 文本。输入必须是「一个值 + 可选空白」，有多余内容就抛 ParseError。
// 失败时抛 ParseError，其 what() 带行号和列号。
Value parse(std::string_view text);

// 带选项的版本。示例：
//   jq::parse(text, jq::ParseOptions{.max_depth = 8});
Value parse(std::string_view text, const ParseOptions& options);

}  // namespace jq
