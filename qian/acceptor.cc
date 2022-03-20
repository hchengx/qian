#include "acceptor.h"
#include "inetaddress.h"

#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace qian {

static int createNonBlocking()
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        printf("create socket failed\n");
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop)
    , acceptSocket_(createNonBlocking())
    , acceptChannel_(std::shared_ptr<EventLoop>(loop), acceptSocket_.fd())
    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            close(connfd);
        }
    } else {
        int savedErrno = errno;
        printf("accept error: %s\n", strerror(savedErrno));
        if (savedErrno == EMFILE) {
            printf("out of file descriptors\n");
        }
    }
}

}