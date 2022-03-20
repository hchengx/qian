#include "poller.h"
#include "epollpoller.h"

namespace qian {

Poller * Poller::newDefaultPoller(EventLoop *loop)
{
	return new EpollPoller(loop);
}

}; // namespace qian