#pragma once
#include <unistd.h>
#include <sys/types.h>

namespace qian {
    class CurrentThread {
    public:
        static pid_t tid();
    };
}