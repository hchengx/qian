#pragma once
#include <sys/types.h>
#include <unistd.h>

namespace qian {
    class CurrentThread {
    public:
        static pid_t tid();
    };


    class Noncopyable {
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;

    private:
        Noncopyable(const Noncopyable &) = delete;
        Noncopyable &operator=(const Noncopyable &) = delete;
    };
}// namespace qian