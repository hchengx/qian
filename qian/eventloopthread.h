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
    typedef std::function<void(EventLoop*)> ThreadCallback;
    EventLoopThread(const ThreadCallback& cb = ThreadCallback(), const std::string& name = std::string());
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadCallback callback_;
};

} // namespace qian