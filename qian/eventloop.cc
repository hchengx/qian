#include "eventloop.h"
#include "channel.h"
#include "poller.h"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>

namespace qian {

thread_local EventLoop::Ptr t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

static int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        std::cerr << "Failed to create eventf";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , thread_id_(getpid())
    , poll_return_time_(Timestamp::now())
    , poller_(Poller::newDefaultPoller(this))
    , channel_(nullptr)
    , wakeup_fd_(createEventfd())
    , mutex_()
{
    // channel_.reset(new Channel(shared_from_this(), wakeup_fd_));
    channel_.reset(new Channel(shared_from_this(), wakeup_fd_));
    channel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    channel_->enableReading();
    ::fcntl(wakeup_fd_, F_SETFL, O_NONBLOCK);
}

void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

EventLoop::~EventLoop()
{
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        std::cerr << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        std::cerr << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::removeChannel(std::shared_ptr<Channel> channel)
{
    poller_->removeChannel(channel.get());
}

void EventLoop::updateChannel(std::shared_ptr<Channel> channel)
{
    poller_->updateChannel(channel.get());
}

} // namespace qian