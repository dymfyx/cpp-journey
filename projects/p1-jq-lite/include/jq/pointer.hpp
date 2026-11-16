#ifndef JQ_POINTER_HPP
#define JQ_POINTER_HPP

#include <string>
#include <string_view>
#include <vector>

#include "jq/path.hpp"
#include "jq/value.hpp"

namespace jq {

// RFC 6901 JSON Pointer：把 "/a/b/0" 解析成解码后的 token 序列 {"a", "b", "0"}。
//
//   - 空串 "" 表示整个文档，返回空 vector；
//   - 必须以 '/' 开头，否则抛 QueryError；
//   - token 里的 "~1" 解码成 '/'，"~0" 解码成 '~'
//     （顺序：先把 "~1" 换成 '/'，再把 "~0" 换成 '~'，反过来会错）；
//   - 非法转义（如 "~2"、结尾单个 "~"）抛 QueryError。
//
// 与 path.hpp 的 parse_path 的区别：pointer 用 '/' 分隔、以 '/' 开头，
// 且数字形式的 token 不预先决定是 key 还是 index——由 evaluate_pointer
// 根据当前值的类型决定（对象就按 key 查，数组就按下标查）。
std::vector<std::string> parse_pointer(std::string_view pointer);

// 按 RFC 6901 求值：
//   - 对象：token 作为 key 查找；
//   - 数组：token 必须是 "0" 或 [1-9][0-9]*（没有前导 0），否则抛 QueryError；
//   - 标量：抛 QueryError；
//   - key 不存在 / 下标越界：QueryError。
Value evaluate_pointer(const Value& root, std::string_view pointer);

}  // namespace jq

#endif  // JQ_POINTER_HPP
