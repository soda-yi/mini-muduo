#include "tcp_connection.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>

#include "channel.hh"
#include "event_loop.hh"

using std::cout;
using std::endl;

TcpConnection::TcpConnection(EventLoop *loop, int sockfd)
    : loop_(loop),
      state_(kConnecting),
      sockfd_(sockfd),
      channel_(new Channel(loop_, sockfd_))
{
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this));
    channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    channel_->EnableReading();
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::HandleRead()
{
    int sockfd = channel_->GetFd();
    constexpr int kMaxLength = 100;
    int readlength;
    char line[kMaxLength];
    if (sockfd < 0) {
        cout << "EPOLLIN sockfd < 0 error " << endl;
        return;
    }
    bzero(line, kMaxLength);
    if ((readlength = read(sockfd, line, kMaxLength)) < 0) {
        if (errno == ECONNRESET) {
            cout << "ECONNREST closed socket fd:" << sockfd << endl;
            close(sockfd);
        }
    } else if (readlength == 0) {
        cout << "read 0 closed socket fd:" << sockfd << endl;
        close(sockfd);
    } else {
        std::string linestr(line, readlength);
        inBuf_.Append(linestr);
        messageCallback_(shared_from_this(), &inBuf_);
    }
}

void TcpConnection::HandleWrite()
{
    int sockfd = channel_->GetFd();
    if (channel_->IsWriting()) {
        int n = ::write(sockfd, outBuf_.Peek(), outBuf_.ReadableBytes());
        if (n > 0) {
            cout << "write " << n << " bytes data again" << endl;
            outBuf_.Retrieve(n);
            if (outBuf_.ReadableBytes() == 0) {
                channel_->DisableWriting();
                loop_->QueueInLoop([this] { if(writeCompleteCallback_) {writeCompleteCallback_(shared_from_this());} });
            }
        }
    }
}

void TcpConnection::HandleClose()
{
    state_ = kDisconnected;
    channel_->DisableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    if (connectionCallback_) {
        connectionCallback_(guardThis);
    }
    // must be the last line
    if (closeCallback_) {
        closeCallback_(guardThis);
    }
}

void TcpConnection::Send(const std::string &message)
{
    loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, message));
}

void TcpConnection::SendInLoop(const std::string &message)
{
    //cout << "IO Loop thread: " << std::this_thread::get_id() << endl;
    int n = 0;
    if (outBuf_.ReadableBytes() == 0) {
        n = ::write(sockfd_, message.c_str(), message.size());
        if (n < 0) {
            cout << "write error" << endl;
        } else if (n == static_cast<int>(message.size())) {
            loop_->QueueInLoop([this] { if(writeCompleteCallback_) {writeCompleteCallback_(shared_from_this());} });
        }
    }
    if (n < static_cast<int>(message.size())) {
        outBuf_.Append(message.substr(n, message.size()));
        if (!channel_->IsWriting()) {
            channel_->EnableWriting();
        }
    }
}

void TcpConnection::Shutdown()
{
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        loop_->RunInLoop([this] {
            if (!channel_->IsWriting()) {
                ::shutdown(sockfd_, SHUT_WR);
            }
        });
    }
}

void TcpConnection::ForceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting) {
        state_ = kDisconnecting;
        loop_->QueueInLoop([this] { HandleClose(); });
    }
}

void TcpConnection::ConnectEstablished()
{
    if (connectionCallback_) {
        state_ = kConnected;
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::ConnectDestroyed()
{
    if (state_ == kConnected) {
        state_ = kDisconnected;
        channel_->DisableAll();

        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    channel_->Remove();
}