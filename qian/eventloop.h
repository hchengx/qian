#pragma once
#include "noncopyable.h"
#include "timestamp.h"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <unistd.h>
#include <vector>

class Channel;
class Poller;

namespace qian {

class EventLoop : Noncopyable {
public:
	typedef std::shared_ptr<EventLoop> Ptr;
    using Function = std::function<void()>;
    EventLoop();
    ~EventLoop();
    void loop();
    void quit();
    Timestamp pollReturnTime() const { return poll_return_time_; }
    void runInLoop(Function cb);
    void queueInLoop(Function cb);
    void wakeup();
    void updateChannel(std::shared_ptr<Channel> channel);
    void removeChannel(std::shared_ptr<Channel> channel);
    void hasChannel(std::shared_ptr<Channel> channel);
	/// TODO: replace getpid() with local thread variable
    bool isInLoopThread() const { return thread_id_ == getpid(); }

private:
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    const pid_t thread_id_;
    Timestamp poll_return_time_;
    std::unique_ptr<Poller> poller_;
    int wakeup_fd_;
    std::unique_ptr<Channel> channel_;
    std::mutex mutex_;
};

}