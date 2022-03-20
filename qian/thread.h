#pragma once
#include "noncopyable.h"
#include <atomic>
#include <functional>
#include <thread>
#include <unistd.h>

namespace qian {

class Thread : Noncopyable {
public:
    typedef std::function<void()> Function;

    /// nothing but saving information about thread
    explicit Thread(Function, const std::string& name = std::string());
    ~Thread();

    /// start the thread
    void start();
    void join();

    bool started() const { return started_; }

    /// get the current thread's id
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

    static int numCreated() { return num_created_; }

private:
    void setDefaultName();
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pthread_t pthread_;
    std::string name_;
    Function func_;
    pid_t tid_;
    int num_;
    static std::atomic_int num_created_;
};

} // namespace qian