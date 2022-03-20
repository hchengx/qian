#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "buffer.h"
#include "inetaddress.h"
#include "noncopyable.h"
#include "timestamp.h"

namespace qian {

class Channel;
class EventLoop;
class Socket;

class TcpConnection : Noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    typedef std::shared_ptr<TcpConnection> Ptr;
    typedef std::function<void(const Ptr&)> ConnectionCallback;
    typedef std::function<void(const Ptr&)> CloseCallback;
    typedef std::function<void(const Ptr&)> WriteCompleteCallback;
    typedef std::function<void(const Ptr&, size_t)> HighWatermarkCallback;
    typedef std::function<void(const Ptr&, Buffer*, Timestamp)> MessageCallback;

    TcpConnection(EventLoop* loop,
        const std::string& name,
        int sockfd,
        const InetAddress& localAddr,
        const InetAddress& peerAddr);
    ~TcpConnection();

    void send(const std::string& buf);
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& callback)
    {
        connection_callback_ = callback;
    }
    void setCloseCallback(const CloseCallback& callback)
    {
        close_callback_ = callback;
    }

    void setMessageCallback(const MessageCallback& callback)
    {
        message_callback_ = callback;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& callback)
    {
        write_complete_callback_ = callback;
    }
    void setHighWatermarkCallback(const HighWatermarkCallback& callback)
    {
        high_watermark_callback_ = callback;
    }

private:
    enum StateE {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected,
    };
    void setState(StateE state) { state_ = state; }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t length);
    void shutdownInLoop();

    void connectionEstablished();
    void connectionDestroyed();

private:
    EventLoop* loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localaddr_;
    const InetAddress peeraddr_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWatermarkCallback high_watermark_callback_;
    CloseCallback close_callback_;

    size_t high_watermark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

} // namespace qian