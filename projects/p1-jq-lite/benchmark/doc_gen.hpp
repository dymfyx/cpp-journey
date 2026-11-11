// 基准测试共用的输入文档。
//
// 为什么单独抽一个头文件：jq_bench 和 bench_nlohmann 必须测「完全相同的字节」，
// 否则对比没有意义。文档生成逻辑只写一份，两边 include。

#pragma once

#include <cstddef>
#include <string>

namespace bench_docs {

// 生成一份约 1MB 的 JSON：一个 meta 对象 + records 数组。
inline std::string make_doc(std::size_t records) {
    std::string out;
    out.reserve(records * 220);
    out += R"({"meta":{"name":"bench","count":)";
    out += std::to_string(records);
    out += R"(},"records":[)";
    for (std::size_t i = 0; i < records; ++i) {
        if (i != 0) {
            out += ',';
        }
        out += R"({"id":)";
        out += std::to_string(i);
        out += R"(,"name":"user_)";
        out += std::to_string(i % 1000);
        out += R"(","active":)";
        out += (i % 2 == 0) ? "true" : "false";
        out += R"(,"score":)";
        out += std::to_string((i * 37) % 1000);
        out += R"(.5,"tags":["a","b","c"]})";
    }
    out += "]}";
    return out;
}

// 典型配置文件大小（~80 B）。
inline const std::string& tiny() {
    static const std::string text =
        R"({"name":"jq-lite","version":1,"tags":["json","cli"],"debug":false})";
    return text;
}

// 约 1 MB。
inline const std::string& large() {
    static const std::string text = make_doc(13000);
    return text;
}

}  // namespace bench_docs
