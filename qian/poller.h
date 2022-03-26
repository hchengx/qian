#pragma once
#include "util.h"
#include <map>
#include <vector>
struct pollfd;

namespace qian {
    class Channel;
    class EventLoop;
    class Timestamp;
    class Poller : Noncopyable {
    public:
        typedef std::vector<Channel *> ChannelList;
        Poller(EventLoop *loop);
        ~Poller();
        Timestamp poll(int timeout, ChannelList *activeChannels);
        /// 维护更新Channel的状态
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);
        void assertInLoopThread();

    private:
        void fillActiveChannels(int numEvents, ChannelList *activeChannels);
        typedef std::vector<struct pollfd> PollFdList;
        typedef std::map<int, Channel *> ChannelMap;
        EventLoop *ownerloop_;
        PollFdList pollfds_;
        /// map from fd to channel
        ChannelMap channels_;
    };

}// namespace qian