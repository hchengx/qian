#pragma once

#include "noncopyable.h"
#include <functional>
#include <memory>
#include <vector>

namespace qian {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int num_threads) { num_threads_ = num_threads; }

    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }
    const std::string& name() const { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int num_threads_;
    int next_; /// round idx

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

} // namespace qian