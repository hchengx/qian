#pragma once

namespace qian {
class Noncopyable {
public:
    Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete; // copy constructor
    Noncopyable(const Noncopyable&&) = delete; // move constructor
    Noncopyable& operator=(const Noncopyable&) = delete; // copy assignment operator
};

} // namespace qian