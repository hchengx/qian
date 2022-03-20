#include "tcpconnection.h"
#include "channel.h"
#include "eventloop.h"
#include "socket.h"

#include <errno.h>
#include <functional>
#include <netinet/tcp.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace qian {

static EventLoop* checkLoopNotNull(EventLoop* loop)
{
    if (loop == nullptr) {
        printf("loop is nullptr\n");
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,
    const std::string& name_arg,
    int sockfd,
    const InetAddress& localAddr,
    const InetAddress& peerAddr)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    printf("TcpConnection::~TcpConnection()\n");
}

void TcpConnection::send(const std::string& buf)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) {
        printf("disconnected, give up writing\n");
        return;
    }

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_callback_) {
                loop_->queueInLoop(std::bind(write_complete_callback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                printf("TcpConnection::sendInLoop(): write error, fd = %d, errno = %d\n", channel_->fd(), errno);
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= high_watermark_ && oldLen < high_watermark_ && high_watermark_callback_) {
            loop_->queueInLoop(std::bind(high_watermark_callback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectionEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connection_callback_(shared_from_this());
}

void TcpConnection::connectionDestroyed()
{
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connection_callback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        message_callback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        printf("TcpConnection::handleRead()\n");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWrite();
                if (write_complete_callback_) {
                    loop_->queueInLoop(std::bind(write_complete_callback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                printf("TcpConnection::handleWrite()\n");
            }
        } else {
            printf("TcpConnection::handleWrite()\n");
        }
    } else {
        printf("TcpConnection::handleWrite()\n");
    }
}

void TcpConnection::handleClose()
{
    printf("TcpConnection::handleClose()\n");
    setState(kDisconnected);
    channel_->disableAll();
    Ptr conn_ptr(shared_from_this());
    connection_callback_(conn_ptr);
    close_callback_(conn_ptr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) == 0) {
        err = optval;
    } else {
        err = errno;
    }
    printf("TcpConnection::handleError() name:%s - SO_ERROR:%d\n", name_.c_str(), err);
}

} // namespace qian