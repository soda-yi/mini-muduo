#include "tcp_connection.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>

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
        //cout << "read " << readlength << " bytes data" << endl;
        std::string linestr(line, readlength);
        inBuf_.Append(linestr);
        messageCallback_(shared_from_this(), &inBuf_);
    } catch (const FdException &e) {
        // TODO: 处理异常
        if (e.GetErrno() == 0) {
            HandleClose();
        } else {
            throw;
        }
    }
}

void TcpConnection::HandleWrite()
{
    //cout << "IO Loop thread: " << std::this_thread::get_id() << endl;
    if (outBufs_.empty()) {
        return;
    }
    try {
        auto &[buf, callback] = outBufs_.front();
        auto temCb = std::move(callback);
        int n = socket_.Write(buf.Peek(), buf.ReadableBytes());
        //cout << "write " << n << " bytes data" << endl;
        buf.Retrieve(n);
        if (buf.ReadableBytes() == 0) {
            outBufs_.pop();
            loop_->QueueInLoop([this, callback = std::move(temCb)] { 
            if(callback) { 
                callback(shared_from_this());
            } });
        }
    } catch (const FdException &e) {
        // TODO: 添加错误处理
        throw;
    }
    if (outBufs_.empty()) {
        channel_.DisableWriting();
    }
}

void TcpConnection::HandleClose()
{
    state_ = StateE::kDisconnected;
    // 必须在HandleClose中而不是ConnectDestroyed中，因为放在ConnectDestroyed中会多次触发EPOLLRDHUP事件直至DisableAll
    channel_.DisableAll();

    if (closeCallback_) {
        closeCallback_(shared_from_this());
    }
}

void TcpConnection::Send(const std::string &message, WriteCompleteCallback callback)
{
    static std::mutex mutex;
    {
        std::scoped_lock lock{mutex};
        outBufs_.emplace(message, std::move(callback));
    }
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
    state_ = StateE::kConnected;
    channel_.EnableReading();

    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::ConnectDestroyed()
{
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }

    channel_.Remove();
    socket_.Close();
}