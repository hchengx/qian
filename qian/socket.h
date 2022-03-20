#pragma once
#include "noncopyable.h"

namespace qian {

class InetAddress;

class Socket : Noncopyable {
public:
    explicit Socket(int sockfd);
    ~Socket();
    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peer_addr);

    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};

} // namespace qian