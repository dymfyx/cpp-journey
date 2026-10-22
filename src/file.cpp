// 这是你要实现的文件。现在每个函数都直接抛异常，测试会全部失败。
//
// 练习顺序建议：
//   1. 先把构造 / 析构 / is_open / close 做对，跑通前几个测试；
//   2. 再实现 write / read_all / read_into；
//   3. 最后处理移动构造、移动赋值、自赋值、close 幂等。
//
// 提示：
//   - 移动构造/赋值不需要 reset：直接把 other.fp_ 取走，再把 other.fp_ 置空。
//   - 移动赋值先检查 this != &other，再 close() 自己，然后接管。
//   - 错误信息用 std::strerror(errno) 拼一段可读的文本。
//   - 用 std::fwrite / std::fread / std::fopen / std::fclose。
#include "raii/file.hpp"

#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
namespace raii {

File::File(const char* path, const char* mode) {
    if (path == nullptr || mode == nullptr) {
        throw std::runtime_error("File: path or mode is null");
    }
    fp_ = std::fopen(path, mode);
    if (fp_ == nullptr) {
        throw std::runtime_error(std::string("File: failed to open ") + path + ": " + std::strerror(errno));
    }
}

File::~File() noexcept {
    close();    
}

File::File(File&& other) noexcept : fp_(other.fp_) {
    other.fp_ = nullptr;
}

File& File::operator=(File&& other) noexcept {
    if (this != &other) {
        close();
        fp_ = other.fp_;
        other.fp_ = nullptr;
    }
    return *this;
}

bool File::is_open() const noexcept {
    return fp_ != nullptr;
}

void File::close() noexcept {
    // TODO: 关闭并置空，重复调用安全。
    if (fp_ != nullptr) {
        std::fclose(fp_);
        fp_ = nullptr;
    }
}

void File::write(std::string_view data) {
    if (!is_open()) {
        throw std::runtime_error("File: write on closed file");
    }
    std::size_t written = std::fwrite(data.data(), 1, data.size(), fp_);
    if (written != data.size()) {
        throw std::runtime_error(std::string("File: failed to write: ") + std::strerror(errno));
    }
}

std::string File::read_all() {
    if (fp_ == nullptr) {
        throw std::runtime_error("File: read on closed file");
    }
    std::string out;
    char buf[4096];
    while (true) {
        std::size_t n = std::fread(buf, 1, sizeof(buf), fp_);
        if (n > 0) {
            out.append(buf, n);
        } else {
            break;
        }
    }
    if (std::ferror(fp_)) {
        throw std::runtime_error("File: read error");
    }
    return out;
}

std::size_t File::read_into(char* buffer, std::size_t size) {
    if (fp_ == nullptr) {
        throw std::runtime_error("File: read on closed file");
    }
    if (buffer == nullptr && size > 0) {
        throw std::runtime_error("File: null buffer");
    }
    std::size_t n = std::fread(buffer, 1, size, fp_);
    if (std::ferror(fp_)) {
        throw std::runtime_error("File: read error");
    }
    return n;

}

}  // namespace raii
