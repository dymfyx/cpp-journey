// p0-01 RAII File: 你唯一需要实现的是 src/file.cpp。这个头文件的接口不要改。
#pragma once

#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>

namespace raii {

// File 持有一个 std::FILE*（C 标准库文件句柄）。
// 规则：构造时打开，析构时一定关闭；不允许拷贝；只允许移动。
class File {
public:
    // 未打开的 File（空句柄）。
    File() noexcept = default;

    // 打开文件。失败时抛 std::runtime_error，消息里要带 errno 对应的原因。
    // path 为 nullptr / 空字符串 / mode 为空也算失败。
    explicit File(const char* path, const char* mode);

    // 析构：如果还持有句柄就关闭它。必须是 noexcept。
    ~File() noexcept;

    // 禁止拷贝：两份对象共用同一个 FILE* 会 double-close。
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    // 移动构造：接管 other 的句柄，并且让 other 不再持有任何句柄。
    // 必须是 noexcept（GTest 与标准库容器都依赖这一点）。
    File(File&& other) noexcept;

    // 移动赋值：先释放自己的句柄，再接管 other 的。必须处理自赋值（f = std::move(f)）。
    File& operator=(File&& other) noexcept;

    // 是否持有打开的句柄。
    bool is_open() const noexcept;

    // 主动关闭。重复调用必须安全。
    void close() noexcept;

    // 写入全部字节。失败（含对已关闭的文件调用）抛 std::runtime_error。
    void write(std::string_view data);

    // 从当前位置读到文件末尾，返回读到的全部字节。对已关闭的文件调用抛异常。
    std::string read_all();

    // 读最多 size 个字节到 buffer，返回实际读到的字节数（文件末尾返回 0）。
    // 对已关闭的文件调用抛异常。
    std::size_t read_into(char* buffer, std::size_t size);

private:
    std::FILE* fp_ = nullptr;
};

}  // namespace raii
