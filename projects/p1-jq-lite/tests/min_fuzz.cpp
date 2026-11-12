// 第 5 周练习 2：手写最小 fuzz（不许用 libFuzzer / 现成的 fuzz_parser.cpp）
//
// 目标：生成 200 个随机输入，喂给 jq::parse，要求：
//   - 只允许抛 jq::ParseError；
//   - 抛别的异常（std::bad_alloc、std::out_of_range…）或崩溃都算 bug；
//   - 全部通过打印 `200/200 ok`。
//
// 提示（先自己写，卡住再看）：
//   1. 随机数：std::mt19937_64 rng(seed); std::uniform_int_distribution<int> dist(0, 255);
//   2. 随机输入有三种来源，选一个起步即可：
//        a. 纯随机字节（大概率立刻异常，适合先跑通流程）
//        b. 从合法模板改几个字节：{"a":[1,2,{"b":"c"}]}
//        c. 随机字符集：从 `{}[]":,0123456789-+.eEtruefalsn ` 里随机取字符
//   3. 用 try { jq::parse(input); } catch (const jq::ParseError&) { /* 合法拒绝 */ }
//      catch (...) { /* 报告：不该出现的异常 */ }
//   4. 失败时打印 hex / 可打印字符两种形式，方便定位。
//
// 验收：./build/min_fuzz 打印 200/200 ok；然后你把 parser.cpp 里的某个边界检查删掉，
//       重跑，看它能不能抓出来（抓不到说明生成器太弱，加针对性字符）。

#include <cstddef>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <string_view>

#include <jq/error.hpp>
#include <jq/parser.hpp>

std::string generate(std::mt19937_64& rng) {
    std::string output;
    static const char alphabet[] = "{}[]\":\\,0123456789-+.eEtruefalsn ";
    std::uniform_int_distribution<int> len_dist(0, 40);
    std::uniform_int_distribution<int> dist(0, sizeof(alphabet) - 2);
    const int len = len_dist(rng);
    for (int i = 0; i < len; ++i) {
        output += alphabet[dist(rng)];
    }
    return output;
}

void report(const std::string& input, const char* msg) {
    std::cerr << "Unexpected exception: " << msg << "\n";
    std::cerr << "Input (hex): ";
    for (char c : input) {
        std::cerr << std::hex << static_cast<int>(static_cast<unsigned char>(c)) << " ";
    }
    std::cerr << "\n";
}

int main() {
    std::mt19937_64 rng(0xC0FFEE);          // 固定种子，失败可复现
    int ok = 0;
    for (int i = 0; i < 200; ++i) {
        std::string input = generate(rng);   // 三种来源选一个起步
        try {
            const auto v = jq::parse(input); // 也可能合法解析成功，也算 ok
            (void)v;
            ++ok;
        } catch (const jq::ParseError&) {
            ++ok;                            // 正确拒绝
        } catch (const std::exception& e) {
            report(input, e.what());         // 不该出现的异常类型
            return 1;
        }
    }
    std::cout << ok << "/200 ok\n";
    return 0;
}
