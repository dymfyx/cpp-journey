// p1 jq-lite: JSON 值（DOM）的类型定义。这个头文件是基础设施，不要改。
//
// 设计：用 std::variant 表示“六选一”的值，天然避免了继承体系里的 dynamic_cast。
// Array = std::vector<Value>，Object = std::map<std::string, Value>（键有序）。
#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace jq {

class Value;

using Array = std::vector<Value>;
using Object = std::map<std::string, Value>;

// 说明：枚举值用 ArrayValue / ObjectValue，是为了不和上面的 Array / Object 别名
// 在 -Wshadow 下冲突（命名是要被编译器检查的）。
enum class Type { Null, Bool, Number, String, ArrayValue, ObjectValue };

class Value {
public:
    Value() noexcept : storage_(nullptr) {}
    Value(std::nullptr_t) noexcept : storage_(nullptr) {}
    Value(bool value) noexcept : storage_(value) {}
    Value(int value) noexcept : storage_(static_cast<double>(value)) {}
    Value(double value) noexcept : storage_(value) {}
    Value(std::string value) : storage_(std::move(value)) {}
    Value(const char* value) : storage_(std::string(value)) {}
    Value(Array value) : storage_(std::move(value)) {}
    Value(Object value) : storage_(std::move(value)) {}

    Value(const Value&) = default;
    Value(Value&&) = default;
    Value& operator=(const Value&) = default;
    Value& operator=(Value&&) = default;
    ~Value() = default;

    Type type() const noexcept;
    bool is_null() const noexcept;
    bool is_bool() const noexcept;
    bool is_number() const noexcept;
    bool is_string() const noexcept;
    bool is_array() const noexcept;
    bool is_object() const noexcept;

    // 访问器：类型不匹配时抛 std::bad_variant_access。
    bool as_bool() const;
    double as_number() const;
    const std::string& as_string() const;
    const Array& as_array() const;
    Array& as_array();
    const Object& as_object() const;
    Object& as_object();

    // array / object 的元素个数；其它类型返回 0。
    std::size_t size() const noexcept;

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const { return !(*this == other); }

    // 序列化：indent < 0 紧凑输出；indent >= 0 每个层级缩进 indent 个空格。
    std::string dump(int indent = -1) const;

private:
    std::variant<std::nullptr_t, bool, double, std::string, Array, Object> storage_;
};

}  // namespace jq
