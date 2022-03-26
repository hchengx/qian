#include "eventloop.h"
#include <thread>
#include <cassert>
#include <poll.h>

namespace qian {
    __thread EventLoop* t_loopInThisThread = nullptr;
    EventLoop::EventLoop():
        looping_(false),
        thread_id_(CurrentThread::tid()) // TODO: get thread id
    {
        if (t_loopInThisThread)
        {
            perror("Another EventLoop exists in this thread");
        } else {
            t_loopInThisThread = this;
        }
    }
    EventLoop::~EventLoop() {
        t_loopInThisThread = nullptr;
    }
    void EventLoop::loop() {
        assert(!looping_);
        assertInLoopThread();
        ::poll(nullptr, 0, 5); // TODO: poll timeout
        printf("EventLoop::loop() stopped\n");
        looping_ = false;
    }
}