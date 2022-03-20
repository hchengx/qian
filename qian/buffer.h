#pragma once

/***************************************************************
 * File: buffer.h
 * Details:
 * 在栈上准备一个 extrabuf 缓冲区。如果读的数据不多，则全部读入buffer_，
 * 否则剩余的读入 extrabuf 缓冲区，再 append 到 buffer_ 中。
 * data struct:
 *	 | kCheapPrepend     |    xxx   | readable        | writable        |
 *	 | prependableBytes()|          | readableBytes() | writableBytes() |
 * use example:
 *
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

namespace qian {

class Buffer {
public:
    /// leave for data header
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kCheapPrepend)
        : buffer_(kCheapPrepend + kInitialSize)
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writableBytes() == kInitialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    /// return header of readable bytes
    const char* peek() const { return begin() + readerIndex_; }

    /// move reader index forward
    void retrieve(size_t len)
    {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }
    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    /// check if there is enough space for len bytes
    /// if not, make space for len bytes
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite() { return (char*)begin() + writerIndex_; }
    const char* beginWrite() const { return begin() + writerIndex_; }

    /// read data in fd to buffer
    ssize_t readFd(int fd, int* savedErrno);

private:
    char* begin() { return (char*)&*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    void makeSpace(size_t len);

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

}; // namespace qian