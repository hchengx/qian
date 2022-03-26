#include "eventloop.h"
#include "channel.h"
#include "poller.h"
#include "timestamp.h"
#include <cassert>
#include <poll.h>
#include <thread>

namespace qian {
    __thread EventLoop *t_loopInThisThread = nullptr;
    const int kPollTimeMs = 10000;// 10 sec
    EventLoop::EventLoop() : looping_(false),
                             thread_id_(CurrentThread::tid()),
                             quit_(true),
                             poller_(std::make_shared<Poller>(this))
    {
        if (t_loopInThisThread) {
            fprintf(stderr, "Another EventLoop exists in this thread");
        } else {
            t_loopInThisThread = this;
        }
    }

    EventLoop::~EventLoop()
    {
        t_loopInThisThread = nullptr;
    }

    void EventLoop::quit()
    {
        quit_ = true;
        // wakeup();
    }

    void EventLoop::updateChannel(Channel *channel)
    {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        poller_->updateChannel(channel);
    }

    void EventLoop::loop()
    {
        assert(!looping_);
        assertInLoopThread();
        looping_ = true;
        quit_ = false;
        while (!quit_) {
            activeChannels_.clear();
            poller_->poll(kPollTimeMs, &activeChannels_);
            for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it) {
                (*it)->handleEvent();
            }
        }
        printf("EventLoop::loop() exit\n");
        looping_ = false;
    }
}// namespace qian