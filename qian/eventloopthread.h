#pragma once
#include "noncopyable.h"
#include "thread.h"
#include <condition_variable>
#include <functional>
#include <mutex>

namespace qian {

class EventLoop;

class EventLoopThread : Noncopyable {
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

private:
    EventLoop* loop_;
    ThreadInitCallback callback_;

    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

} // namespace qian