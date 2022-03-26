#include "poller.h"
#include "channel.h"
#include "eventloop.h"
#include "timestamp.h"
#include <cassert>
#include <poll.h>

namespace qian {
    Poller::Poller(EventLoop *loop)
        : ownerloop_(loop),
          pollfds_(),
          channels_() {
    }
    Poller::~Poller() {
    }

    Timestamp Poller::poll(int timeout, ChannelList *activeChannels) {
        int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
        Timestamp now(Timestamp::now());
        if (numEvents > 0) {
            fillActiveChannels(numEvents, activeChannels);
        } else if (numEvents == 0) {
            printf("Poller::poll() timeout\n");
        } else {
            printf("Poller::poll() error\n");
        }
        return now;
    }

    void Poller::fillActiveChannels(int numEvents, ChannelList *activeChannels) {
        for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
            if (pfd->revents > 0) {
                --numEvents;
                ChannelMap::iterator ch = channels_.find(pfd->fd);
                assert(ch != channels_.end());
                Channel *channel = ch->second;
                assert(channel->fd() == pfd->fd);
                channel->set_revents(pfd->revents);
                activeChannels->push_back(channel);
            }
        }
    }

    void Poller::updateChannel(Channel *channel) {
        assertInLoopThread();
        if (channel->index() < 0) {
            assert(channels_.find(channel->fd()) == channels_.end());

            struct pollfd pfd;
            pfd.fd = channel->fd();
            pfd.events = static_cast<short>(channel->events());
            pfd.revents = 0;
            pollfds_.push_back(pfd);

            int idx = static_cast<int>(pollfds_.size()) - 1;
            channel->set_index(idx);
            channels_[pfd.fd] = channel;
        } else {
            // update existing one
            assert(channels_.find(channel->fd()) != channels_.end());
            assert(channels_[channel->fd()] == channel);
            int idx = channel->index();
            assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
            struct pollfd &pfd = pollfds_[idx];
            assert(pfd.fd == channel->fd() || pfd.fd == -1);
            pfd.events = static_cast<short>(channel->events());
            pfd.revents = 0;
            if (channel->isNoneEvent()) {
                pfd.fd = -1;
            }
        }
    }

    void Poller::assertInLoopThread() {
        ownerloop_->assertInLoopThread();
    }

}// namespace qian