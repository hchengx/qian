#include "poller.h"
#include "channel.h"

namespace qian {
Poller::Poller(EventLoop* loop)
    : loop_(loop)
{
}

bool Poller::hasChannel(Channel* channel) const
{
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

} // namespace qian