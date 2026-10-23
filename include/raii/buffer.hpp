// p0-02 Buffer: 你唯一需要实现的是 src/buffer.cpp。这个头文件的接口不要改。
#pragma once

#include <cstddef>
#include <utility>

namespace raii {

// Buffer 持有一块用 new char[] 分配的字节，并负责释放它。
// 规则：拷贝 = 深拷贝（各管各的内存），移动 = 窃取指针（源变成空缓冲）。
class Buffer {
public:
    // 空缓冲：data() == nullptr，size() == 0。
    Buffer() noexcept = default;

    // 分配 size 个字节，并全部清零。size == 0 时等价于默认构造。
    explicit Buffer(std::size_t size);

    // 从 [data, data + size) 拷贝出 size 个字节。
    // data == nullptr 且 size > 0 时抛 std::invalid_argument（参数错误）。
    Buffer(const char* data, std::size_t size);

    // 深拷贝：新开一块内存，把 other 的内容复制过来。
    Buffer(const Buffer& other);

    // 深拷贝赋值：复制 other 的内容到 *this。
    // 要求强异常安全：如果因为 bad_alloc 失败，*this 必须保持原样且不泄漏。
    // 必须处理自赋值（b = b）。
    Buffer& operator=(const Buffer& other);

    // 移动构造：接管 other 的内存，other 变成空缓冲。必须是 noexcept。
    Buffer(Buffer&& other) noexcept;

    // 移动赋值：先释放自己的内存，再接管 other 的；处理自赋值（b = std::move(b)）。
    Buffer& operator=(Buffer&& other) noexcept;

    // 析构：delete[] 掉自己的内存。必须是 noexcept。
    ~Buffer() noexcept;

    std::size_t size() const noexcept { return size_; }
    bool empty() const noexcept { return size_ == 0; }

    char* data() noexcept { return data_; }
    const char* data() const noexcept { return data_; }

    // 不做越界检查（调用方保证 i < size()）。
    char& operator[](std::size_t i) noexcept { return data_[i]; }
    const char& operator[](std::size_t i) const noexcept { return data_[i]; }

    // 调整大小：保留前 min(size(), new_size) 个字节；
    // 变大的部分清零；new_size == 0 时释放内存。
    // 要求强异常安全（分配失败时 *this 不变）。
    void resize(std::size_t new_size);

    // 交换内容，必须 noexcept（供 copy-and-swap 使用）。
    void swap(Buffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

private:
    char* data_ = nullptr;
    std::size_t size_ = 0;
};

}  // namespace raii
