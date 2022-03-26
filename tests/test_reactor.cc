#include "../qian/qian.h"
#include <cstring>
#include <sys/timerfd.h>

qian::EventLoop *g_loop;

void timeout() {
    printf("timeout()\n");
    g_loop->quit();
}

int main(int argc, char *argv[]) {
    qian::EventLoop loop;
    g_loop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    qian::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(struct itimerspec));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);
    loop.loop();
    ::close(timerfd);
}