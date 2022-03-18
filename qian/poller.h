#pragma once

#include <vector>
#include <unordered_map>

#include "noncopyable.h"
#include "timestamp.h"


namespace qian {

class Channel;
class EventLoop;

class Poller
{
public:
	using ChannelList = std::vector<Channel*>;
	Poller(EventLoop *loop);
	virtual ~Poller() = default;

	virtual Timestamp poll(int timeout_ms, ChannelList *activeChannels) = 0;
	virtual void updateChannel(Channel *channel) = 0;
	virtual void removeChannel(Channel *channel) = 0;

	bool hasChannel(Channel *channel) const;

	static Poller *newDefaultPoller(EventLoop *loop);
protected:
	using ChannelMap = std::unordered_map<int, Channel*>;
	ChannelMap channels_;

private:
	EventLoop *loop_;
};

} // namespace qian