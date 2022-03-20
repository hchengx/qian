#pragma once

#include "channel.h"
#include "noncopyable.h"
#include "socket.h"
#include <functional>

namespace qian {
class EventLoop;
class InetAddress;

class Acceptor : Noncopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool request);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    bool listenning_;

    NewConnectionCallback newConnectionCallback_;
};

}