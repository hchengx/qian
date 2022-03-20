#include "thread.h"
#include <semaphore.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace qian {

std::atomic_int Thread::num_created_(0);

Thread::Thread(Function func, const std::string& name)
    : started_(false)
    , joined_(false)
    , func_(std::move(func))
    , name_(name)
    , tid_(0)
{
    num_ = ++num_created_;
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_) {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread(
        [&]() { // new thread
            tid_ = static_cast<pid_t>(::syscall(SYS_gettid));
            sem_post(&sem);
            func_();
        }));
    sem_wait(&sem); // wait for tid_ valid
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    if (name_.empty()) {
        char buf[32] = { 0 };
        snprintf(buf, sizeof(buf), "Thread%d", num_);
        name_ = buf;
    }
}

} // namespace qian
