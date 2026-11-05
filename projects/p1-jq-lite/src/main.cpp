// jq_lite —— 一个极简的命令行 JSON 查询工具（jq 子集）。
//
// 用法：
//   jq_lite <路径> [文件]         从文件读取 JSON（省略文件则读标准输入），按路径取值并打印
//   jq_lite -c <路径> [文件]      紧凑输出（单行）
//   jq_lite -r <路径> [文件]      结果是字符串时不加引号
//
// 例子：
//   echo '{"name":"dym","age":19}' | ./jq_lite .name
//   ./jq_lite '.users[0].id' data.json
//   cat data.json | ./jq_lite -c '.items'
//
// 退出码：0 成功；1 用法/文件错误；2 JSON 解析失败或路径查询失败。
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "jq/error.hpp"
#include "jq/parser.hpp"
#include "jq/path.hpp"
#include "jq/value.hpp"

namespace {

void print_usage() {
    std::cerr << "用法: jq_lite [-c] [-r] <路径> [文件]\n"
                 "  路径示例: .  .name  .users[0].id\n"
                 "  -c  紧凑输出（默认是缩进两格的格式化输出）\n"
                 "  -r  结果是字符串时直接输出内容，不加引号\n"
                 "不指定文件时从标准输入读取 JSON。\n";
}

// 一次性读完整个流（文件或 stdin）。
std::string read_all(std::istream& in) {
    return std::string(std::istreambuf_iterator<char>(in),
                       std::istreambuf_iterator<char>());
}

}  // namespace

int main(int argc, char** argv) {
    bool compact = false;
    bool raw = false;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "-c") {
            compact = true;
        } else if (arg == "-r") {
            raw = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage();
            return 0;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "未知选项: " << arg << '\n';
            print_usage();
            return 1;
        } else {
            positional.emplace_back(arg);
        }
    }

    if (positional.empty()) {
        print_usage();
        return 1;
    }

    const std::string& path = positional[0];
    std::string text;
    if (positional.size() >= 2) {
        std::ifstream file(positional[1], std::ios::binary);
        if (!file) {
            std::cerr << "无法打开文件: " << positional[1] << '\n';
            return 1;
        }
        text = read_all(file);
    } else {
        text = read_all(std::cin);
    }

    try {
        const jq::Value root = jq::parse(text);
        const jq::Value result = jq::evaluate_path(root, path);

        if (raw && result.type() == jq::Type::String) {
            std::cout << result.as_string();
        } else {
            std::cout << result.dump(compact ? -1 : 2);
        }
        std::cout << '\n';
    } catch (const jq::ParseError& error) {
        std::cerr << "JSON 解析失败: " << error.what() << '\n';
        return 2;
    } catch (const jq::QueryError& error) {
        std::cerr << "查询失败: " << error.what() << '\n';
        return 2;
    }

    return 0;
}
