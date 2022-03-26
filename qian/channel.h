#pragma once
#include "util.h"
#include <functional>

namespace qian {
    class EventLoop;
    class Channel : Noncopyable {
    public:
        typedef std::function<void()> EventCallback;

        Channel(EventLoop *loop, int fd);
        ~Channel();
        void handleEvent();

        void setReadCallback(const EventCallback &cb) { readCallback_ = cb; }
        void setWriteCallback(const EventCallback &cb) { writeCallback_ = cb; }
        void setErrorCallback(const EventCallback &cb) { errorCallback_ = cb; }

        int fd() const { return fd_; }
        int events() const { return events_; }
        /// called by Poller
        void set_revents(int revt) { revents_ = revt; }
        bool isNoneEvent() const { return events_ == kNoneEvent; }
        void enableReading() {
            events_ |= kReadEvent;
            update();
        }

        int index() { return index_; }
        void set_index(int index) { index_ = index; }

        EventLoop *ownerLoop() { return loop_; }

    private:
        void update();

    private:
        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;

        /// 用户关心的事件
        int events_;
        /// 活动事件
        int revents_;
        int index_;


        EventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback errorCallback_;
    };

}// namespace qian