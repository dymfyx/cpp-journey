// ============================================================================
// 第 8 周扩展：RFC 6901 JSON Pointer（你要实现的部分）
//
// 目标：让 20 个 PointerTest 全绿（ctest 63/63）。
//
// 输入 / 输出：
//   parse_pointer("")            -> {}                      （空指针 = 整个文档）
//   parse_pointer("/")           -> {""}                    （一个空 key）
//   parse_pointer("/a/b")        -> {"a", "b"}
//   parse_pointer("/a~1b")       -> {"a/b"}                 （~1 解码成 '/'）
//   parse_pointer("/a~0b")       -> {"a~b"}                 （~0 解码成 '~'）
//   parse_pointer("a/b")         -> 抛 QueryError            （必须以 '/' 开头）
//   parse_pointer("/a~2b")       -> 抛 QueryError            （非法转义）
//
//   evaluate_pointer(doc, "")        -> 整个文档
//   evaluate_pointer(doc, "/a/b")    -> 对象按键查
//   evaluate_pointer(doc, "/arr/1")  -> 数组按下标查
//   evaluate_pointer(doc, "/arr/01") -> 抛 QueryError（数组下标不允许前导 0）
//
// 实现提示：
//   1. parse_pointer：先判空串，再判首字符 '/'；
//      用一个游标扫，遇到 '/' 就切一段 token，最后别忘收尾那一段；
//      每段解码：先处理 ~1 再处理 ~0（顺序错了 "~01" 会解析成 '/'）。
//   2. evaluate_pointer：拿到 token 序列后循环，跟随当前值走：
//         const Value* current = &root;
//         对象 → find(token)；数组 → 校验下标字符串后转换；
//         标量 → 报错；最后 return *current;
//   3. 错误类型统一用 QueryError（见 include/jq/path.hpp），std::runtime_error 的派生类。
//   4. 不要修改 include/jq/pointer.hpp 里的接口，也不要改 tests/test_pointer.cpp。
//   5. 写完先跑：./build/jq_pointer_tests --gtest_filter='PointerTest.*'
// ============================================================================
#include "jq/pointer.hpp"

#include <stdexcept>
#include <string>

namespace jq {

std::string decode_pointer_token(std::string_view raw) {
    std::string out;
    out.reserve(raw.size());          // 最多和原文一样长
    for (std::size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] != '~') { out.push_back(raw[i]); continue; }
        if (i + 1 >= raw.size()) throw QueryError("'~' at end");
        switch (raw[++i]) {
            case '0': out.push_back('~'); break;
            case '1': out.push_back('/'); break;
            default: throw QueryError("invalid escape");
        }
    }
    return out;
}

std::vector<std::string> parse_pointer(std::string_view pointer) {
    std::vector<std::string> tokens;
    if (pointer.empty()) {
        return {};
    }
    if (pointer[0] != '/') {
        throw QueryError("JSON Pointer must start with '/'");
    }
    std::size_t start = 1; 
    for (std::size_t i = 1; i <= pointer.size(); ++i) {
        if (i == pointer.size() || pointer[i] == '/') {
            std::string raw(pointer.substr(start, i - start)); 
            std::string token = decode_pointer_token(raw);
            tokens.push_back(std::move(token));
            start = i + 1;
        }
    }
    return tokens;
}

Value evaluate_pointer(const Value& root, std::string_view pointer) {
    auto tokens = parse_pointer(pointer);
    const Value* current = &root;
    for (const auto& token : tokens) {
        if (current->is_object()) {
            const Object& obj = current->as_object();
            auto it = obj.find(token);
            if (it == obj.end()) throw QueryError("key not found: " + token);
            current = &it->second;                 // 只下降一次
        } else if (current->is_array()) {
            const Array& arr = current->as_array();
            if (token.empty() || (token.size() > 1 && token[0] == '0')) {
                throw QueryError("invalid array index: " + token);
            }
            std::size_t index = 0;
            for (char c : token) {
                if (c < '0' || c > '9') throw QueryError("invalid array index: " + token);
                index = index * 10 + static_cast<std::size_t>(c - '0');
            }
            if (index >= arr.size()) throw QueryError("array index out of bounds: " + token);   
            current = &arr[index];
        } else {
            throw QueryError("cannot index a scalar with token: " + token);
        }
    }
    return *current;
}

}  // namespace jq
