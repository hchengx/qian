#include "socket.h"
#include "inetaddress.h"
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace qian {

Socket::Socket(int sockfd)
    : sockfd_(sockfd)
{
}

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::listen()
{
    if (::listen(sockfd_, 1024) != 0) {
        perror("listen()");
    }
}

int Socket::accept(InetAddress* peer_addr)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    int connfd = ::accept(sockfd_, (struct sockaddr*)&addr, &len);
    if (connfd < 0) {
        perror("accept()");
        return connfd;
    }
    peer_addr->setSockAddr(addr);
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        perror("shutdownWrite()");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

} // namespace qian