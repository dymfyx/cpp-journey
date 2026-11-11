// 外部项目使用 jq-lite 的最小示例（第 5 周验收用）。
//
// 它不在本仓库的构建树里被编译，而是：
//   1. 先把 jq-lite 安装到某个前缀（cmake --install build --prefix /tmp/jq-stage）
//   2. 再独立 configure 本目录，用 find_package(jq-lite) 找到它
// 能编过、能跑出结果，就说明 install/export 配置是对的。

#include <iostream>
#include <string>

#include <jq/parser.hpp>
#include <jq/path.hpp>

int main() {
    const std::string text = R"({"name":"dym","skills":["c++","python"]})";

    const jq::Value doc = jq::parse(text);
    std::cout << "dump(2):\n" << doc.dump(2) << '\n';
    std::cout << "skills[1] = "
              << jq::evaluate_path(doc, ".skills[1]").as_string() << '\n';
    return 0;
}
