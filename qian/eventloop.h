#pragma once
#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

namespace qian {
    class EventLoop {
    public:
        EventLoop();
        ~EventLoop();
        void loop();
        void assertInLoopThread() {
            if (!isInLoopThread()) {
                fprintf(stderr, "EventLoop::assertInLoopThread() failed\n");
                abort();
            }
        }

    private:
        bool isInLoopThread() const {
            return thread_id_ == CurrentThread::tid();
        }

    private:
        const pid_t thread_id_;
        bool looping_;
    };

}// namespace qian