#include "channel.h"
#include "eventloop.h"
#include <sys/epoll.h>

namespace qian {

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(std::shared_ptr<EventLoop> loop, int fd):
	loop_(loop),
	fd_(fd),
	events_(0),
	revents_(0),
	index_(-1),
	tied_(false)
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
	tie_ = obj;
	tied_ = true;
}

void Channel::update()
{
	loop_->updateChannel(shared_from_this());
}

void Channel::remove()
{
	loop_->removeChannel(shared_from_this());
}

void Channel::handleEvent(Timestamp receiveTime)
{
	if(tied_) {
		std::shared_ptr<void> guard = tie_.lock();
		if(guard) {
			handleEventWithGuard(receiveTime);
		}
	}
	else {
		handleEventWithGuard(receiveTime);
	}
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	if((revents_& EPOLLHUP) && !(revents_ & EPOLLIN)) {
		if(close_callback_) {
			close_callback_();
		}
	}

	if(revents_ & EPOLLERR) {
		if(error_callback_) {
			error_callback_();
		}
	}

	if(revents_ & (EPOLLIN | EPOLLPRI)) {
		if(read_callback_) {
			read_callback_(receiveTime);
		}
	}

	if(revents_ & EPOLLOUT) {
		if(write_callback_) {
			write_callback_();
		}
	}
}


} // namespace qian