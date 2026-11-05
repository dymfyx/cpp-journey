#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "jq/value.hpp"

namespace jq {

// 路径求值失败（语法错、键不存在、类型不匹配、下标越界）时抛出。
class QueryError : public std::runtime_error {
public:
    explicit QueryError(const std::string& message);
};

// 路径中的一段：取对象成员（.key）或取数组元素（[index]）。
struct PathSegment {
    bool is_index = false;   // false = 取对象成员；true = 取数组下标
    std::string key;         // is_index == false 时有效
    std::size_t index = 0;   // is_index == true 时有效
};

// 解析路径串，例如：
//   "."                -> 空 vector（取整个文档）
//   ".name"            -> 1 段：key = "name"
//   ".users[1].id"     -> 3 段：key="users", index=1, key="id"
// 语法错误时抛 QueryError。
std::vector<PathSegment> parse_path(std::string_view text);

// 按路径从 root 取值；结果可能是任意 JSON 值（对象/数组/标量）。
// 路径不存在或类型不匹配时抛 QueryError。
Value evaluate_path(const Value& root, std::string_view path);

}  // namespace jq
