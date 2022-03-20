#pragma once
#include "noncopyable.h"
#include "timestamp.h"
#include <functional>
#include <memory>

namespace qian {

class EventLoop;

class Channel : Noncopyable, public std::enable_shared_from_this<Channel> {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(std::shared_ptr<EventLoop> loop, int fd);
    ~Channel() = default;

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { read_callback_ = cb; }
    void setWriteCallback(EventCallback cb) { write_callback_ = cb; }
    void setCloseCallback(EventCallback cb) { close_callback_ = cb; }
    void setErrorCallback(EventCallback cb) { error_callback_ = cb; }

    /**
     * TcpConnection and Channel lifetime problem
     * @param obj the owner object
     * @return
     */
    void tie(const std::shared_ptr<void>& obj);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWrite()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() const { return index_; }
    void set_index(int idx) { index_ = idx; }

    std::shared_ptr<EventLoop> ownerLoop() { return loop_; }

    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

private:
    std::shared_ptr<EventLoop> loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;
    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};
}; // namespace qian