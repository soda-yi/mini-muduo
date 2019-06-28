#ifndef TCPCONNECTION_HH
#define TCPCONNECTION_HH

#include <memory>
#include <string>

#include "buffer.hh"
#include "callbacks.hh"

class EventLoop;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, int sockfd);
    ~TcpConnection();

    bool IsConnected() const { return state_ == kConnected; }
    bool IsDisConnected() const { return state_ == kDisconnected; }

    /* 对外提供的发送接口，用户调用后不用关心是否发送完毕 */
    void Send(const std::string &message);
    void Shutdown();
    void ForceClose();

    void ConnectEstablished();
    void ConnectDestroyed();

    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void SetWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

    EventLoop *GetLoop() const { return loop_; }
    int GetSockfd() const { return sockfd_; }

private:
    enum StateE { kDisconnected,
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
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    Buffer inBuf_;
    Buffer outBuf_;
};

#endif