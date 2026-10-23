// 这是你要实现的文件。现在大部分函数都直接抛异常，测试会全部失败。
//
// 练习顺序建议：
//   1. 先实现两个构造函数和 resize 之前的空壳，跑通前几个测试；
//   2. 再实现拷贝构造 / 拷贝赋值（深拷贝），注意自赋值和强异常安全；
//   3. 然后移动构造 / 移动赋值（窃取指针），注意把 other 置空；
//   4. 最后 resize。
//
// 提示：
//   - 约定：size() == 0 时 data() == nullptr，不要用 new char[0] 来表示空。
//   - 拷贝赋值要先分配新内存、拷贝成功，再释放旧内存；中途失败 *this 不能变。
//   - 移动函数是 noexcept，里面不能抛异常，也不能留 todo()（会 terminate）。
//   - 所有实现写完后，把匿名命名空间里的 todo() 删掉，否则 -Wunused-function + -Werror 会报错。
#include "raii/buffer.hpp"

#include <cstring>
#include <stdexcept>
#include <string>

namespace raii {

Buffer::Buffer(std::size_t size) {
    if(size == 0) return;
    data_ = new char[size]{};
    size_ = size;
}

Buffer::Buffer(const char* data, std::size_t size) {
    if (size == 0) {
        return;
    }
    if (data == nullptr) {
        throw std::invalid_argument("Buffer: null data with non-zero size");
    }
    data_ = new char[size]{};
    std::memcpy(data_, data, size);
    size_ = size;
}

Buffer::Buffer(const Buffer& other) {
    if(other.size_ == 0) return;
    size_ = other.size_;
    data_ = new char[size_]{};
    memcpy(data_ , other.data_ , size_);
}

Buffer& Buffer::operator=(const Buffer& other) {
     if (this == &other) {
        return *this;             
    }
    Buffer tmp(other);             
    swap(tmp);                    
    return *this;
}

Buffer::Buffer(Buffer&& other) noexcept 
    : data_(other.data_), size_(other.size_){
    other.data_ = nullptr;
    other.size_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if(this == &other) return *this;
    delete[] data_;
    data_ = other.data_;
    size_ = other.size_;
    other.data_ = nullptr;
    other.size_ = 0;
    return *this;
}

Buffer::~Buffer() noexcept {
    delete[] data_;  // data_ 初始是 nullptr，delete[] nullptr 是安全的
}

void Buffer::resize(std::size_t new_size) {
    if (new_size == size_) {
        return;
    }
    if (new_size == 0) {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
        return;
    }
    char* fresh = new char[new_size]{};        
    const std::size_t keep = (size_ < new_size) ? size_ : new_size;
    if (keep > 0) std::memcpy(fresh, data_, keep);        
    delete[] data_;                              
    data_ = fresh;                              
    size_ = new_size;
    return;
}

}  // namespace raii
