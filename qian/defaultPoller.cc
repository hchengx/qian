#include "epollpoller.h"
#include "poller.h"

namespace qian {

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    return new EpollPoller(loop);
}

}; // namespace qian