// p1 jq-lite: JSON 值（DOM）的类型定义。
//
// 这是「接口契约」：类型别名、枚举、函数签名都不要改——测试和后续 CLI 都依赖它们。
// 你的任务是实现 src/value.cpp 里所有函数体（构造函数、查询/取值、dump、比较）。
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
    // ---- 构造：六种 JSON 值各一个入口 ----
    // 空值用 std::nullptr_t 表示；JSON 没有整数类型，Value(int) 内部转成 double。
    Value() noexcept;                  // null
    Value(std::nullptr_t) noexcept;    // null（显式传 nullptr 时）
    Value(bool value) noexcept;        // true / false
    Value(int value) noexcept;         // 数字（内部存 double）
    Value(double value) noexcept;      // 小数
    Value(std::string value);          // 字符串（按值传，实现里可用 std::move）
    Value(const char* value);          // 字符串字面量
    Value(Array value);                // 数组
    Value(Object value);               // 对象

    // 五个特殊成员函数（拷贝构造/移动构造/拷贝赋值/移动赋值/析构）故意不声明：
    // 交给编译器隐式生成（Rule of Zero）。想一想：为什么 variant 已经让它们正确了？

    // ---- 查询 ----
    Type type() const noexcept;
    bool is_null() const noexcept;
    bool is_bool() const noexcept;
    bool is_number() const noexcept;
    bool is_string() const noexcept;
    bool is_array() const noexcept;
    bool is_object() const noexcept;

    // ---- 取值：类型不匹配时抛 std::bad_variant_access ----
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
    // 六选一：nullptr_t / bool / double / string / Array / Object。
    std::variant<std::nullptr_t, bool, double, std::string, Array, Object> storage_;
};

}  // namespace jq
