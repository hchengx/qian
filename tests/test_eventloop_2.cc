/*
 * description: negative test for eventloop
 */
#include "../qian/qian.h"
#include <thread>

void threadFunc(qian::EventLoop *loop) {
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), qian::CurrentThread::tid());
    loop->loop();
}

int main() {
    qian::EventLoop loop;
    std::thread t(threadFunc, &loop);
    t.join();
    return 0;
}