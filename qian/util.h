#pragma once
#include <sys/types.h>
#include <unistd.h>

namespace qian {
    class CurrentThread {
    public:
        static pid_t tid();
    };
}// namespace qian