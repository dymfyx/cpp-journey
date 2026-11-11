// 第 4 周：fuzz 测试。
//
// 这是一个独立的 fuzz 驱动（不依赖 GoogleTest），做两件事：
//   1. 随机生成【合法】JSON → parse → dump → 再 parse，断言两次的 Value 完全相等
//      （parse/dump 必须互为逆运算，这条能抓出序列化的所有边角 bug）；
//   2. 对同一份合法 JSON 做随机字节破坏（改字节/删字节/插入/截断）→ 再 parse，
//      只允许抛 ParseError，不允许崩溃、UB、ASan 报错、死循环。
//
// 用法：
//   ./build/jq_fuzz <迭代次数> [随机种子]
//   ./build/jq_fuzz            # 默认 20000 次，种子 0xC0FFEE
//
// ctest 会把 FuzzParser 注册成一个测试（5000 次，约 1 秒）。
// 这个程序编译时带 ASan + UBSan（见 CMakeLists 的 ENABLE_SANITIZERS），
// 所以「越界一次」就会直接以非 0 退出，并打印出具体的越界位置。

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include "jq/error.hpp"
#include "jq/parser.hpp"
#include "jq/value.hpp"

namespace {

using Rng = std::mt19937_64;

// ---------------------------------------------------------------------------
// 随机生成【合法】JSON
// ---------------------------------------------------------------------------

// 随机字符串生成：从一组安全字符里抽，避免故意生成非法控制字符
std::string random_json_string(Rng& rng, std::size_t max_len) {
    static const char alphabet[] =
        "abcXYZ0123456789_- .:;/\\\"'[]{}|<>,!@#$%^&*()+=";
    const std::size_t len = static_cast<std::size_t>(rng() % (max_len + 1));
    std::string out;
    out.reserve(len);
    for (std::size_t i = 0; i < len; ++i) {
        const char c = alphabet[rng() % (sizeof(alphabet) - 1)];
        if (c == '"' || c == '\\') {
            out += '\\';
        }
        out += c;
    }
    return out;
}

std::string random_number(Rng& rng) {
    const int kind = static_cast<int>(rng() % 6);
    switch (kind) {
        case 0: return "0";
        case 1: return "-0";
        case 2: return std::to_string(rng() % 100000);
        case 3: return "-" + std::to_string(rng() % 100000);
        case 4: {
            // 小数
            const int frac = static_cast<int>(rng() % 1000000);
            return std::to_string(rng() % 100) + "." + std::to_string(frac);
        }
        default: {
            // 科学计数法
            const int exp = static_cast<int>(rng() % 300) - 150;
            return std::to_string(rng() % 10) + "e" + (exp < 0 ? "-" : "+") + std::to_string(std::abs(exp));
        }
    }
}

std::string gen_value(Rng& rng, int depth);

std::string gen_array(Rng& rng, int depth) {
    const std::size_t n = static_cast<std::size_t>(rng() % 4);
    std::string out = "[";
    for (std::size_t i = 0; i < n; ++i) {
        if (i != 0) out += ',';
        out += gen_value(rng, depth - 1);
    }
    out += ']';
    return out;
}

std::string gen_object(Rng& rng, int depth) {
    const std::size_t n = static_cast<std::size_t>(rng() % 4);
    std::string out = "{";
    for (std::size_t i = 0; i < n; ++i) {
        if (i != 0) out += ',';
        out += '"' + random_json_string(rng, 8) + "\":";
        out += gen_value(rng, depth - 1);
    }
    out += '}';
    return out;
}

std::string gen_value(Rng& rng, int depth) {
    if (depth <= 0) {
        // 叶子：50% 标量、50% 空容器
        switch (rng() % 6) {
            case 0: return "null";
            case 1: return "true";
            case 2: return "false";
            case 3: return random_number(rng);
            case 4: return '"' + random_json_string(rng, 10) + '"';
            default: return (rng() % 2 == 0) ? "[]" : "{}";
        }
    }
    switch (rng() % 8) {
        case 0: return "null";
        case 1: return "true";
        case 2: return "false";
        case 3: return random_number(rng);
        case 4: return '"' + random_json_string(rng, 12) + '"';
        case 5: return gen_array(rng, depth);
        case 6: return gen_object(rng, depth);
        default: return (rng() % 2 == 0) ? "[]" : "{}";
    }
}

// ---------------------------------------------------------------------------
// 随机破坏一段合法 JSON
// ---------------------------------------------------------------------------

std::string mutate(std::string text, Rng& rng) {
    switch (rng() % 5) {
        case 0:  // 改一个字节
            if (!text.empty()) {
                text[rng() % text.size()] = static_cast<char>(rng() % 256);
            }
            break;
        case 1:  // 删一段
            if (!text.empty()) {
                const std::size_t pos = static_cast<std::size_t>(rng() % text.size());
                const std::size_t len = 1 + static_cast<std::size_t>(rng() % 4);
                text.erase(pos, len);
            }
            break;
        case 2:  // 插一个随机字节
            text.insert(text.begin() + static_cast<std::ptrdiff_t>(rng() % (text.size() + 1)),
                        static_cast<char>(rng() % 256));
            break;
        case 3:  // 截断
            if (text.size() > 1) {
                text.resize(static_cast<std::size_t>(rng() % text.size()));
            }
            break;
        default:  // 复制一段插入
            if (!text.empty()) {
                const std::size_t pos = static_cast<std::size_t>(rng() % text.size());
                const std::size_t len = 1 + static_cast<std::size_t>(rng() % 8);
                text.insert(pos, text.substr(pos, len));
            }
            break;
    }
    return text;
}

}  // namespace

int main(int argc, char** argv) {
    std::size_t iterations = 20000;
    std::uint64_t seed = 0xC0FFEEULL;
    if (argc > 1) {
        iterations = static_cast<std::size_t>(std::strtoull(argv[1], nullptr, 10));
    }
    if (argc > 2) {
        seed = std::strtoull(argv[2], nullptr, 10);
    }

    Rng rng(seed);
    std::size_t accepted = 0;
    std::size_t rejected = 0;

    for (std::size_t n = 0; n < iterations; ++n) {
        const std::string doc = gen_value(rng, static_cast<int>(rng() % 4));

        // ---- 1. 合法文档必须能 parse，且 parse(dump(v)) == v ----
        const int indent = (rng() % 2 == 0) ? -1 : 2;
        jq::Value value;
        try {
            value = jq::parse(doc);
        } catch (const std::exception& error) {
            std::cerr << "generated valid JSON rejected!\n  input : " << doc << "\n  error : "
                      << error.what() << '\n';
            return 1;
        }

        const std::string dumped = value.dump(indent);
        try {
            const jq::Value roundtrip = jq::parse(dumped);
            if (!(value == roundtrip)) {
                std::cerr << "roundtrip mismatch!\n  input : " << doc << "\n  dump  : " << dumped
                          << '\n';
                return 1;
            }
        } catch (const std::exception& error) {
            std::cerr << "dump output is not valid JSON!\n  input : " << doc << "\n  dump  : "
                      << dumped << "\n  error : " << error.what() << '\n';
            return 1;
        }
        ++accepted;

        // ---- 2. 破坏后的输入只允许抛 ParseError，不允许崩溃 ----
        const std::string broken = mutate(doc, rng);
        try {
            const jq::Value broken_value = jq::parse(broken);
            // 少数破坏恰好还是合法 JSON，那就顺便验证它也能 roundtrip
            (void)jq::parse(broken_value.dump(-1));
        } catch (const jq::ParseError&) {
            ++rejected;
        } catch (const std::exception& error) {
            std::cerr << "unexpected exception type!\n  input : " << broken << "\n  error : "
                      << error.what() << '\n';
            return 1;
        }
    }

    std::cout << "fuzz done: " << iterations << " iterations, " << accepted << " valid docs, "
              << rejected << " rejected mutations\n";
    return 0;
}
