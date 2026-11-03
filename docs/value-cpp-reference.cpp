// p1 jq-lite: Value 的参考实现（对照用，不参与构建）。
//
// 位置：docs/value-cpp-reference.cpp（docs 不在 CMake 的构建范围里，不会被编译）。
// 用法：先自己写完 src/value.cpp，再逐段对照；重点看构造函数初始化列表、
//       三个 dump 辅助函数的职责划分、缩进模式下逗号/换行/收尾括号的位置。
#include "jq/value.hpp"

#include <cstddef>
#include <cstdio>
#include <string>
#include <utility>
#include <variant>

namespace jq {

// ============================================================================
// 一、序列化辅助函数
// ============================================================================
namespace {

// 追加「换行 + indent*depth 个空格」；indent < 0 是紧凑模式，不换行。
void append_indent(std::string& out, int indent, int depth) {
    if (indent < 0) {
        return;
    }
    out.push_back('\n');
    out.append(static_cast<std::size_t>(indent * depth), ' ');
}

// 按 JSON 规则转义，并加上首尾双引号。
void dump_string(std::string& out, const std::string& text) {
    out.push_back('"');
    for (const char raw : text) {
        const auto ch = static_cast<unsigned char>(raw);
        switch (ch) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (ch < 0x20) {
                    char buffer[8] = {};
                    std::snprintf(buffer, sizeof buffer, "\\u%04x", ch);
                    out += buffer;
                } else {
                    out.push_back(static_cast<char>(ch));
                }
        }
    }
    out.push_back('"');
}

// 递归序列化：先看 value 是哪种类型，再决定怎么写、要不要往下递归。
void dump_impl(std::string& out, const Value& value, int indent, int depth) {
    switch (value.type()) {
        case Type::Null:
            out += "null";
            return;
        case Type::Bool:
            out += value.as_bool() ? "true" : "false";
            return;
        case Type::Number: {
            char buffer[32] = {};
            std::snprintf(buffer, sizeof buffer, "%.17g", value.as_number());
            out += buffer;
            return;
        }
        case Type::String:
            dump_string(out, value.as_string());
            return;
        case Type::ArrayValue: {
            const Array& arr = value.as_array();
            out.push_back('[');
            bool first = true;
            for (const Value& item : arr) {
                if (!first) {
                    out.push_back(',');
                }
                first = false;
                append_indent(out, indent, depth + 1);
                dump_impl(out, item, indent, depth + 1);
            }
            if (!arr.empty()) {
                append_indent(out, indent, depth);
            }
            out.push_back(']');
            return;
        }
        case Type::ObjectValue: {
            const Object& obj = value.as_object();
            out.push_back('{');
            bool first = true;
            for (const auto& [key, item] : obj) {
                if (!first) {
                    out.push_back(',');
                }
                first = false;
                append_indent(out, indent, depth + 1);
                dump_string(out, key);
                out.push_back(':');
                if (indent >= 0) {
                    out.push_back(' ');
                }
                dump_impl(out, item, indent, depth + 1);
            }
            if (!obj.empty()) {
                append_indent(out, indent, depth);
            }
            out.push_back('}');
            return;
        }
    }
}

}  // namespace

// ============================================================================
// 二、构造函数
// ============================================================================

Value::Value() noexcept : storage_(nullptr) {}

Value::Value(std::nullptr_t) noexcept : storage_(nullptr) {}

Value::Value(bool value) noexcept : storage_(value) {}

Value::Value(int value) noexcept : storage_(static_cast<double>(value)) {}

Value::Value(double value) noexcept : storage_(value) {}

Value::Value(std::string value) : storage_(std::move(value)) {}

Value::Value(const char* value) : storage_(std::string(value)) {}

Value::Value(Array value) : storage_(std::move(value)) {}

Value::Value(Object value) : storage_(std::move(value)) {}

// ============================================================================
// 三、类型查询
// ============================================================================

Type Value::type() const noexcept {
    if (is_null()) {
        return Type::Null;
    }
    if (is_bool()) {
        return Type::Bool;
    }
    if (is_number()) {
        return Type::Number;
    }
    if (is_string()) {
        return Type::String;
    }
    if (is_array()) {
        return Type::ArrayValue;
    }
    return Type::ObjectValue;
}

bool Value::is_null() const noexcept {
    return std::holds_alternative<std::nullptr_t>(storage_);
}

bool Value::is_bool() const noexcept {
    return std::holds_alternative<bool>(storage_);
}

bool Value::is_number() const noexcept {
    return std::holds_alternative<double>(storage_);
}

bool Value::is_string() const noexcept {
    return std::holds_alternative<std::string>(storage_);
}

bool Value::is_array() const noexcept {
    return std::holds_alternative<Array>(storage_);
}

bool Value::is_object() const noexcept {
    return std::holds_alternative<Object>(storage_);
}

// ============================================================================
// 四、取值
// ============================================================================

bool Value::as_bool() const {
    return std::get<bool>(storage_);
}

double Value::as_number() const {
    return std::get<double>(storage_);
}

const std::string& Value::as_string() const {
    return std::get<std::string>(storage_);
}

const Array& Value::as_array() const {
    return std::get<Array>(storage_);
}

Array& Value::as_array() {
    return std::get<Array>(storage_);
}

const Object& Value::as_object() const {
    return std::get<Object>(storage_);
}

Object& Value::as_object() {
    return std::get<Object>(storage_);
}

// ============================================================================
// 五、size / operator==
// ============================================================================

std::size_t Value::size() const noexcept {
    if (const auto* arr = std::get_if<Array>(&storage_)) {
        return arr->size();
    }
    if (const auto* obj = std::get_if<Object>(&storage_)) {
        return obj->size();
    }
    return 0;
}

bool Value::operator==(const Value& other) const {
    return storage_ == other.storage_;
}

// ============================================================================
// 六、dump 入口
// ============================================================================

std::string Value::dump(int indent) const {
    std::string out;
    dump_impl(out, *this, indent, 0);
    return out;
}

}  // namespace jq
