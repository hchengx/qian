#include "../qian/eventloop.h"
#include <thread>
#include <cstdio>

void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), qian::CurrentThread::tid());
    qian::EventLoop loop;
    loop.loop();
}

int main()
{
    qian::EventLoop loop;
    loop.loop();
    std::thread t(threadFunc);
    t.join();
    pthread_exit(NULL);
}