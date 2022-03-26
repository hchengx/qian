#include "eventloop.h"
#include <cassert>
#include <poll.h>
#include <thread>

namespace qian {
    __thread EventLoop *t_loopInThisThread = nullptr;
    EventLoop::EventLoop() : looping_(false),
                             thread_id_(CurrentThread::tid()) {
        if (t_loopInThisThread) {
            perror("Another EventLoop exists in this thread");
        } else {
            t_loopInThisThreadead = this;
        }
    }
    EventLoop::~EventLoop() {
        t_loopInThisThread = nullptr;
    }
    void EventLoop::loop() {
        assert(!looping_);
        assertInLoopThread();
        ::poll(nullptr, 0, 5);
        printf("EventLoop::loop() stopped\n");
        looping_ = false;
    }
}// namespace qian