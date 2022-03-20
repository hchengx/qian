#pragma once
#include "poller.h"

#include <sys/epoll.h>
#include <vector>

namespace qian {

// forward declaration
class Channel;

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller();

    Timestamp poll(int timeout_ms, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;
    void fillActiveChannels(int numEvents, ChannelList* activeChannels);
    void update(int op, Channel* channel);
    typedef std::vector<epoll_event> EventList;

private:
    int epollfd_;
    EventList events_;
};

} // namespace qian
