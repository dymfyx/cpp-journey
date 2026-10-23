// p1 jq-lite: Value 的实现（基础设施，已写好）。
//
// 第 2 周的任务是：不看这个文件，自己重写 dump()，并补齐序列化测试。
#include "jq/value.hpp"

#include <cstdio>
#include <string>
#include <utility>

namespace jq {

namespace {

void append_indent(std::string& out, int indent, int depth) {
    if (indent < 0) {
        return;
    }
    out.push_back('\n');
    out.append(static_cast<std::size_t>(indent * depth), ' ');
}

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
                if (ch < 0x20U) {
                    char buffer[8] = {};
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(ch));
                    out += buffer;
                } else {
                    out.push_back(static_cast<char>(ch));
                }
        }
    }
    out.push_back('"');
}

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
            std::snprintf(buffer, sizeof(buffer), "%.17g", value.as_number());
            out += buffer;
            return;
        }
        case Type::String:
            dump_string(out, value.as_string());
            return;
        case Type::ArrayValue: {
            const Array& items = value.as_array();
            if (items.empty()) {
                out += "[]";
                return;
            }
            out.push_back('[');
            for (std::size_t i = 0; i < items.size(); ++i) {
                if (i != 0U) {
                    out.push_back(',');
                }
                append_indent(out, indent, depth + 1);
                dump_impl(out, items[i], indent, depth + 1);
            }
            append_indent(out, indent, depth);
            out.push_back(']');
            return;
        }
        case Type::ObjectValue: {
            const Object& members = value.as_object();
            if (members.empty()) {
                out += "{}";
                return;
            }
            out.push_back('{');
            bool first = true;
            for (const auto& [key, item] : members) {
                if (!first) {
                    out.push_back(',');
                }
                first = false;
                append_indent(out, indent, depth + 1);
                dump_string(out, key);
                out += (indent < 0) ? ":" : ": ";
                dump_impl(out, item, indent, depth + 1);
            }
            append_indent(out, indent, depth);
            out.push_back('}');
            return;
        }
    }
}

}  // namespace

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

bool Value::is_null() const noexcept { return std::holds_alternative<std::nullptr_t>(storage_); }
bool Value::is_bool() const noexcept { return std::holds_alternative<bool>(storage_); }
bool Value::is_number() const noexcept { return std::holds_alternative<double>(storage_); }
bool Value::is_string() const noexcept { return std::holds_alternative<std::string>(storage_); }
bool Value::is_array() const noexcept { return std::holds_alternative<Array>(storage_); }
bool Value::is_object() const noexcept { return std::holds_alternative<Object>(storage_); }

bool Value::as_bool() const { return std::get<bool>(storage_); }
double Value::as_number() const { return std::get<double>(storage_); }
const std::string& Value::as_string() const { return std::get<std::string>(storage_); }
const Array& Value::as_array() const { return std::get<Array>(storage_); }
Array& Value::as_array() { return std::get<Array>(storage_); }
const Object& Value::as_object() const { return std::get<Object>(storage_); }
Object& Value::as_object() { return std::get<Object>(storage_); }

std::size_t Value::size() const noexcept {
    if (const auto* items = std::get_if<Array>(&storage_)) {
        return items->size();
    }
    if (const auto* members = std::get_if<Object>(&storage_)) {
        return members->size();
    }
    return 0;
}

bool Value::operator==(const Value& other) const { return storage_ == other.storage_; }

std::string Value::dump(int indent) const {
    std::string out;
    dump_impl(out, *this, indent, 0);
    return out;
}

}  // namespace jq
