// p1 jq-lite: 解析错误类型。带 1-based 的行号、列号，CLI 会直接打印它。
#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>

namespace jq {

namespace detail {

inline std::string format_parse_error(const std::string& message, std::size_t line,
                                      std::size_t column) {
    return message + " (line " + std::to_string(line) + ", column " + std::to_string(column) + ")";
}

}  // namespace detail

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message, std::size_t line, std::size_t column)
        : std::runtime_error(detail::format_parse_error(message, line, column)),
          line_(line),
          column_(column) {}

    std::size_t line() const noexcept { return line_; }
    std::size_t column() const noexcept { return column_; }

private:
    std::size_t line_;
    std::size_t column_;
};

}  // namespace jq
