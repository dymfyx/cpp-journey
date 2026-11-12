// JSONTestSuite 官方解析测试集驱动（第 6 周）。
//
// 用法：
//   ./build/jq_jsontestsuite                      # 用编译期默认目录 tests/jsonsuite
//   ./build/jq_jsontestsuite /path/to/jsonsuite   # 指定目录
//
// 规则（来自 JSONTestSuite 的约定）：
//   y_*.json  必须解析成功
//   n_*.json  必须解析失败，且只能抛 jq::ParseError
//   i_*.json  实现自由决定（接受或拒绝都算对），只统计不判错
//
// 退出码：0 全部符合预期；1 有 y_ 被拒、n_ 被接受、或抛了非 ParseError 的异常。

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <jq/error.hpp>
#include <jq/parser.hpp>

namespace fs = std::filesystem;

namespace {

std::string read_file(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    std::string text;
    in.seekg(0, std::ios::end);
    const auto size = in.tellg();
    if (size > 0) {
        text.resize(static_cast<std::size_t>(size));
        in.seekg(0, std::ios::beg);
        in.read(text.data(), static_cast<std::streamsize>(size));
    }
    return text;
}

std::string_view classify(const std::string& name) {
    if (name.size() >= 2 && name[1] == '_') {
        return std::string_view(name.data(), 2);
    }
    return {};
}

}  // namespace

int main(int argc, char** argv) {
    const fs::path dir = (argc > 1) ? fs::path(argv[1]) : fs::path(JSONSUITE_DIR);

    if (!fs::is_directory(dir)) {
        std::cerr << "JSONTestSuite directory not found: " << dir << '\n';
        std::cerr << "hint: scripts/fetch-jsontestsuite.ps1 (Windows) 或手动解压到 tests/jsonsuite\n";
        return 2;
    }

    int y_total = 0;
    int y_ok = 0;
    int n_total = 0;
    int n_ok = 0;
    int i_total = 0;
    int i_accepted = 0;
    std::vector<std::string> failures;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        const std::string_view kind = classify(name);
        if (kind != "y_" && kind != "n_" && kind != "i_") {
            continue;
        }

        const std::string text = read_file(entry.path());
        bool accepted = false;
        std::string error_message;

        try {
            (void)jq::parse(text);
            accepted = true;
        } catch (const jq::ParseError& e) {
            error_message = e.what();
        } catch (const std::exception& e) {
            failures.push_back(name + ": 抛了非 ParseError 异常: " + e.what());
            continue;
        }

        if (kind == "y_") {
            ++y_total;
            if (accepted) {
                ++y_ok;
            } else {
                failures.push_back(name + ": 合法 JSON 被拒绝: " + error_message);
            }
        } else if (kind == "n_") {
            ++n_total;
            if (!accepted) {
                ++n_ok;
            } else {
                failures.push_back(name + ": 非法 JSON 被接受");
            }
        } else {
            ++i_total;
            if (accepted) {
                ++i_accepted;
            }
        }
    }

    std::cout << "y_ (必须接受): " << y_ok << '/' << y_total << '\n';
    std::cout << "n_ (必须拒绝): " << n_ok << '/' << n_total << '\n';
    std::cout << "i_ (自由): 接受 " << i_accepted << " / 拒绝 " << (i_total - i_accepted) << " (共 "
              << i_total << ")\n";

    if (!failures.empty()) {
        std::cout << "\n失败用例 (" << failures.size() << "):\n";
        for (const auto& message : failures) {
            std::cout << "  " << message << '\n';
        }
        return 1;
    }
    std::cout << "\n全部符合预期\n";
    return 0;
}
