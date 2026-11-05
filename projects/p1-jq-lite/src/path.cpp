// 第 3 周：CLI 路径查询（jq 子集）。
//
// 这个文件要你实现两个函数：parse_path（语法解析）和 evaluate_path（沿路径取值）。
// 先把 parse_path 做出来，跑通 PathTest.ParsePathSegmentsShape；
// 再做 evaluate_path，让其余测试全部变绿。
//
// 支持的路径语法（子集）：
//   "."                  -> 整个文档
//   ".name"              -> 对象取成员 name
//   ".a.b.c"             -> 连续取成员
//   ".items[0]"          -> 数组取下标
//   ".[1]"               -> 对根数组取下标
//   ".users[1].id"       -> 混合
//
// 键名规则：字母或下划线开头，后面可以跟字母、数字、下划线。
// 下标规则：[ + 非负十进制整数 + ]，不允许前导 0 以外的花活（"[]"、"[abc]" 都是错）。
//
// 提示：
//   - parse_path 用下标 i 扫描 string_view，遇到 '.' 解析一个 key，遇到 '[' 解析一个 index。
//   - 判断字符：自己写 is_alpha / is_digit 小函数，别用 std::isalpha（char 有符号会 UB）。
//   - 解析 key 时记 start，扫到非字母数字下划线为止，substr(start, end - start) 得到 key。
//   - 解析数字用循环 idx = idx * 10 + (c - '0')，注意溢出可以先不管（测试用例不会太大）。
//   - 语法错误统一 throw QueryError("...")，错误信息要能看懂（例如 "path must start with '.'"）。
//   - evaluate_path 先调 parse_path，再拿 `Value current` 逐段前进：
//       * 取 key：current 必须是对象，否则 throw QueryError("... is not an object")；
//         用 as_object().find(key)，找不到 throw QueryError("key not found: " + key)。
//       * 取下标：current 必须是数组，否则 throw QueryError("... is not an array")；
//         index >= size() 时 throw QueryError("index out of range: ...")。
//   - 返回类型是 Value（值语义），所以最后 return current; 即可。
#include "jq/path.hpp"
#include <stdexcept>
#include <string>

namespace jq {

QueryError::QueryError(const std::string& message) : std::runtime_error(message) {}

namespace{
bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_key_char(char c) { return is_alpha(c) || is_digit(c) || c == '_'; }
}  // namespace

std::vector<PathSegment> parse_path(std::string_view text) {
    std::vector<PathSegment> segments;

    if (text.empty()) {
        throw QueryError("path must not be empty");
    }
    if (text[0] != '.') {
        throw QueryError("path must start with '.'");
    }

    std::size_t i = 0;
    while (i < text.size()) {
        // 可选的点
        if (text[i] == '.') {
            ++i;
            if (i >= text.size()) {
                break;              // 输入就是 "."，合法，空路径
            }
        }

        if (text[i] == '[') {
            // ---------- index 分支 ----------
            ++i;
            const std::size_t start = i;
            std::size_t value = 0;
            while (i < text.size() && is_digit(text[i])) {
                value = value * 10 + static_cast<std::size_t>(text[i] - '0');
                ++i;
            }
            if (i == start) throw QueryError("expected index after '['");
            if (i >= text.size() || text[i] != ']') {
                throw QueryError("expected ']' at position " + std::to_string(i));
            }
            ++i;
            PathSegment seg;
            seg.is_index = true;
            seg.index = value;
            segments.push_back(seg);
        } else {
            // ---------- key 分支 ----------
            const std::size_t start = i;
            while (i < text.size() && is_key_char(text[i])) ++i;
            if (i == start) {
                throw QueryError(std::string("unexpected character '") + text[i] +
                                 "' at position " + std::to_string(i));
            }
            PathSegment seg;
            seg.is_index = false;
            seg.key = std::string(text.substr(start, i - start));
            segments.push_back(seg);
        }
    }

    return segments;
}

Value evaluate_path(const Value& root, std::string_view path) {
    const std::vector<PathSegment> segments = parse_path(path);
    const Value* current = &root;
    for (const PathSegment& seg : segments){
        if(seg.is_index){
            if(!current->is_array()) throw QueryError("cannot index into a non-array value");
            const Array& arr = current->as_array();
            if(seg.index >= arr.size()){
                throw QueryError("index " + std::to_string(seg.index) +
                                 " out of range (size " + std::to_string(arr.size()) + ")");
            }
            current = &arr[seg.index];  
        } else{
            if(!current->is_object()) throw QueryError("cannot take key '" + seg.key + "' from a non-object value");
            const Object& obj = current->as_object();
            const auto it = obj.find(seg.key);
            if(it == obj.end()) throw QueryError("key '" + seg.key + "' not found");
            current = &it->second;
        }
    }
    return *current;
}

}  // namespace jq
