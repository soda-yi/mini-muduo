#ifndef TCPCONNECTION_HH
#define TCPCONNECTION_HH

#include <any>
#include <memory>
#include <string>

#include "buffer.hh"
#include "callbacks.hh"
#include "channel.hh"

class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, int sockfd) noexcept;
    TcpConnection(const TcpConnection &) = delete;
    TcpConnection &operator=(const TcpConnection &) = delete;
    // FIXME 移动构造不能是默认的
    TcpConnection(TcpConnection &&) = default;
    TcpConnection &operator=(TcpConnection &&) = default;
    ~TcpConnection();

    bool IsConnected() const noexcept { return state_ == StateE::kConnected; }
    bool IsDisConnected() const noexcept { return state_ == StateE::kDisconnected; }

    /* 对外提供的发送接口，用户调用后不用关心是否发送完毕 */
    void Send(const std::string &message);
    void Shutdown();
    void ForceClose();

    void ConnectEstablished();
    void ConnectDestroyed();

    void SetConnectionCallback(ConnectionCallback cb) noexcept { connectionCallback_ = std::move(cb); }
    void SetMessageCallback(MessageCallback cb) noexcept { messageCallback_ = std::move(cb); }
    void SetWriteCompleteCallback(WriteCompleteCallback cb) noexcept { writeCompleteCallback_ = std::move(cb); }
    void SetCloseCallback(CloseCallback cb) noexcept { closeCallback_ = std::move(cb); }
    void SetContext(std::any context) noexcept { context_ = std::move(context); }
    std::any &GetContext() noexcept { return context_; }

    EventLoop *GetLoop() const noexcept { return loop_; }
    int GetSockfd() const noexcept { return sockfd_; }

private:
    enum class StateE : char { kDisconnected,
                               kConnecting,
                               kConnected,
                               kDisconnecting };
    void HandleRead();
    /* 用来发送输出缓冲区中剩余的字节 */
    void HandleWrite();
    void HandleClose();
    void SendInLoop(const std::string &message);

    EventLoop *loop_;
    StateE state_;
    int sockfd_;
    Channel channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    Buffer inBuf_;
    Buffer outBuf_;
    std::any context_;
};

#endif