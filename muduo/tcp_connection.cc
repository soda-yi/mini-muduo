#include "tcp_connection.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>

#include "event_loop.hh"

using std::cout;
using std::endl;

TcpConnection::TcpConnection(EventLoop *loop, sockets::Socket socket)
    : loop_{loop},
      state_{StateE::kConnecting},
      socket_{std::move(socket)},
      channel_{loop_, socket_.GetFd()}
{
    channel_.SetReadCallback([this] { HandleRead(); });
    channel_.SetWriteCallback([this] { HandleWrite(); });
    channel_.SetCloseCallback([this] { HandleClose(); });
    channel_.Add();
    channel_.EnableReading();
}

TcpConnection::~TcpConnection()
{
    state_ = StateE::kDisconnected;
    channel_.Remove();
}

void TcpConnection::HandleRead()
{
    constexpr int kMaxLength = 100;
    char line[kMaxLength];
    ::bzero(line, kMaxLength);

    try {
        auto readlength = socket_.Read(line, sizeof(line));
        std::string linestr(line, readlength);
        inBuf_.Append(linestr);
        messageCallback_(shared_from_this(), &inBuf_);
    } catch (const FdException &e) {
        // TODO: 处理异常
        throw;
    }
}

void TcpConnection::HandleWrite()
{
    //cout << "IO Loop thread: " << std::this_thread::get_id() << endl;
    try {
        int n = socket_.Write(outBuf_.Peek(), outBuf_.ReadableBytes());
        cout << "write " << n << " bytes data again" << endl;
        outBuf_.Retrieve(n);
    } catch (const FdException &e) {
        // TODO: 添加错误处理
        throw;
    }
    if (outBuf_.ReadableBytes() == 0) {
        channel_.DisableWriting();
        loop_->QueueInLoop([this] { 
            if(writeCompleteCallback_) { 
                writeCompleteCallback_(shared_from_this());
            } });
    }
}

void TcpConnection::HandleClose()
{
    state_ = StateE::kDisconnected;
    channel_.DisableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    // must be the last line
    if (closeCallback_) {
        closeCallback_(guardThis);
    }
}

void TcpConnection::Send(const std::string &message)
{
    // FIXME: 不是线程安全的
    outBuf_.Append(message);
    loop_->RunInLoop([this] { if (!channel_.IsWriting()) { channel_.EnableWriting(); } });
}

void TcpConnection::Shutdown()
{
    if (state_ == StateE::kConnected) {
        state_ = StateE::kDisconnecting;
        loop_->RunInLoop([this] {
            if (!channel_.IsWriting()) {
                socket_.ShutdownWrite();
            }
        });
    }
}

void TcpConnection::ForceClose()
{
    if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
        state_ = StateE::kDisconnecting;
        loop_->QueueInLoop([this] { HandleClose(); });
    }
}

void TcpConnection::ConnectEstablished()
{
    if (connectionCallback_) {
        state_ = StateE::kConnected;
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::ConnectDestroyed()
{
    if (state_ == StateE::kConnected) {
        state_ = StateE::kDisconnected;
        channel_.DisableAll();

        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    channel_.Remove();
    socket_.Close();
}