// p1 jq-lite: Value 的实现（第 2 周作业：全部自己写）。
//
// 目标：补完所有 TODO，让 ctest 25/25 全绿。
// 写之前建议先想清楚三件事：
//   1. 构造函数的初始化列表怎么写（storage_ 里装什么类型）
//   2. dump 的三个辅助函数各自负责什么（缩进 / 字符串转义 / 递归分派）
//   3. 缩进模式下逗号、换行、收尾括号分别放在哪
//
// 提示文档：docs/value-cpp-讲解.md（是「参考答案讲解」，建议写完再看）。
#include "jq/value.hpp"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

namespace jq {

// ============================================================================
// 一、序列化辅助函数（第 2 周核心）
// ============================================================================
namespace {

// 在 out 末尾追加「换行 + indent*depth 个空格」。
// indent < 0（紧凑模式）时什么都不做。
// 提示：out.push_back('\n'); out.append(static_cast<std::size_t>(indent * depth), ' ');
void append_indent(std::string& out, int indent, int depth) {
    // TODO
    if(indent < 0) return;
    out.push_back('\n');
    out.append(static_cast<std::size_t>(indent * depth) , ' ');
}

// 把字符串 text 按 JSON 规则转义后，写成带双引号的形式追加到 out。
// 规则：
//   双引号 "   ->  \"       （输出：反斜杠 + 引号两个字符）
//   反斜杠 \   ->  \\       （输出：两个反斜杠字符）
//   退格/换页/换行/回车/制表 ->  \b \f \n \r \t
//   其它 < 0x20 -> \u00XX（std::snprintf(buffer, sizeof buffer, "\\u%04x", ch)）
//   其余字符（包括中文的 UTF-8 字节）原样 push_back
// 注意：char 在 Linux 上是有符号的，先 static_cast<unsigned char> 再判断。
void dump_string(std::string& out, const std::string& text) {
    out.push_back('"');
    for(const char raw : text){
        const auto ch = static_cast<unsigned char>(raw);
        switch(ch){
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
                if(ch < 0x20U){
                    char buffer[8] = {};
                    std::snprintf(buffer, sizeof buffer, "\\u%04x", ch);
                    out += buffer;
                }
                else out.push_back(static_cast<char>(ch));
        }
    }
    out.push_back('"');
}

// 递归地把 value 序列化追加到 out。
//   indent = -1 紧凑；>= 0 缩进宽度
//   depth   当前嵌套层数（顶层为 0）
// 分派方式：switch (value.type())，六个分支：
//   Null  -> "null"
//   Bool  -> "true" / "false"
//   Number-> std::snprintf(buffer, sizeof buffer, "%.17g", value.as_number())
//   String-> dump_string(out, value.as_string())
//   Array -> 空直接 "[]"；否则 '[' + 每个元素前（非首个加 ','，再 append_indent(depth+1)，再递归）
//            + 收尾 append_indent(depth) + ']'
//   Object-> 结构同数组，key 也要 dump_string；key 与 value 之间：
//            紧凑模式 ":"，缩进模式 ": "（冒号后一个空格）
void dump_impl(std::string& out, const Value& value, int indent, int depth) {
    switch(value.type()){
        case Type::Null:
            out += "null";
            return;
        case Type::Bool:
            out += value.as_bool() ? "true" : "false";
            return;
        case Type::Number:{
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
            for(const Value& item : arr){
                if(!first) out.push_back(',');
                first = false;
                append_indent(out, indent, depth + 1);
                dump_impl(out, item, indent, depth + 1);
            }
            if(!arr.empty()) append_indent(out, indent, depth);
            out.push_back(']');
            return;
        }

        case Type::ObjectValue: {
            const Object& obj = value.as_object();
            out.push_back('{');
            bool first = true;
            for(const auto& [key, item] : obj){
                if(!first) out.push_back(',');
                first = false;
                append_indent(out, indent, depth + 1);
                dump_string(out, key);
                out.push_back(':');
                if (indent >= 0) { out.push_back(' '); }
                dump_impl(out, item, indent, depth + 1);
            }
            if(!obj.empty()) append_indent(out, indent, depth);
            out.push_back('}');
            return;
        }
    }
}

}  // namespace

// ============================================================================
// 二、构造函数（9 个）
// ============================================================================
// 提示：都用成员初始化列表，例如  Value() noexcept : storage_(nullptr) {}
//       字符串/数组/对象按值传参，内部用 std::move 转移所有权，避免多一次拷贝。

Value::Value() noexcept : storage_(nullptr){}

Value::Value(std::nullptr_t) noexcept : storage_(nullptr){}

Value::Value(bool value) noexcept : storage_(value){}

Value::Value(int value) noexcept : storage_(static_cast<double>(value)){}

Value::Value(double value) noexcept : storage_(value){}

Value::Value(std::string value) : storage_(std::move(value)){}

Value::Value(const char* value) : storage_(std::string(value)){}

Value::Value(Array value) : storage_(std::move(value)){}

Value::Value(Object value) : storage_(std::move(value)){}

// ============================================================================
// 三、类型查询（7 个）
// ============================================================================
// 提示：is_* 全部是 std::holds_alternative<T>(storage_) 的一行封装；
//       type() 按顺序判断，最后兜底 Type::ObjectValue。

Type Value::type() const noexcept {
    if (is_null()){
        return Type::Null;
    }
    if (is_bool()){
        return Type::Bool;
    }
    if (is_number()){
        return Type::Number;
    }
    if (is_string()){
        return Type::String;
    }
    if (is_array()){
        return Type::ArrayValue;
    }
    if (is_object()){
        return Type::ObjectValue;
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
// 四、取值（7 个，类型不符时抛 std::bad_variant_access）
// ============================================================================
// 提示：std::get<T>(storage_)；返回 const 引用可避免拷贝大对象。

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
    if (const auto* arr = std::get_if<Array>(&storage_))
        return arr->size();
    if (const auto* obj = std::get_if<Object>(&storage_))
        return obj->size();
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
