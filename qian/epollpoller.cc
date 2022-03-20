#include "epollpoller.h"
#include"channel.h"
#include <cstring>
#include<unistd.h>
#include <cassert>

namespace qian {

const int kNew = -1;
const int kAdded = 1;
const int kRemoved = 2;

EpollPoller::EpollPoller(EventLoop *loop)
	: Poller(loop),
	  epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	  events_(kInitEventListSize)
{
	if (epollfd_ < 0) {
		::fprintf(stderr, "EpollPoller::EpollPoller() failed: %s\n", ::strerror(errno));
	}
}

EpollPoller::~EpollPoller()
{
	::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeout_ms, ChannelList *activeChannels)
{
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeout_ms);
	int savedErrno = errno;
	Timestamp now(Timestamp::now());
	if (numEvents > 0) {
		fillActiveChannels(numEvents, activeChannels);
		if (numEvents == static_cast<int>(events_.size())) {
			events_.resize(events_.size() * 2);
		}
	} else if (numEvents == 0) {
		// nothing happend
	} else {
		if (savedErrno != EINTR) {
			::fprintf(stderr, "EpollPoller::poll() failed: %s\n", ::strerror(savedErrno));
		}
	}
	return now;
}

void EpollPoller::updateChannel(Channel *channel)
{
	const int index = channel->index();
	if(index == kNew || index == kRemoved)
	{
		if(index == kNew)
		{
			int fd = channel->fd();
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}else{
			assert(channels_.find(channel->fd()) != channels_.end());
			assert(channels_[channel->fd()] == channel);
		}
	}else {
		int fd = channel->fd();
		if (channel->isNoneEvent()) {
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kRemoved);
		} else {
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EpollPoller::removeChannel(Channel *channel)
{
	int fd = channel->fd();
	channels_.erase(fd);
	int index = channel->index();
	if(index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->set_index(kNew);
}

void EpollPoller::fillActiveChannels(int num_events, ChannelList *active_channels)
{
	assert(static_cast<size_t>(num_events) <= events_.size());
	for (int i = 0; i < num_events; ++i) {
		Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		active_channels->push_back(channel);
	}
}

void EpollPoller::update(int op, Channel *channel)
{
	epoll_event ev;
	bzero(&ev, sizeof(ev));
	int fd = channel->fd();
	ev.events = channel->events();
	ev.data.ptr = channel;
	if(::epoll_ctl(epollfd_, op, fd, &ev) < 0)
	{
		::fprintf(stderr, "EpollPoller::updateChannel() failed: %s\n", ::strerror(errno));
	}
}


} // namespace qian