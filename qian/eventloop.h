#pragma once
#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <unistd.h>
#include <vector>

namespace qian {
    class Channel;
    class Poller;
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
        void updateChannel(Channel *channel);
        void quit();

    private:
        bool isInLoopThread() const {
            return thread_id_ == CurrentThread::tid();
        }

    private:
        const pid_t thread_id_;
        bool looping_;
        bool quit_;
        std::shared_ptr<Poller> poller_;
        typedef std::vector<Channel *> ChannelList;
        ChannelList activeChannels_;
    };

}// namespace qian