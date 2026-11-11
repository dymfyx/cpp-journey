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

int main() {
    // TODO(练习 2)：在这里实现
    std::cout << "TODO: 最小 fuzz 还没写\n";
    return 0;
}
