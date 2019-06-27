#ifndef TCPCONNECTION_HH
#define TCPCONNECTION_HH

#include <memory>
#include <string>

#include "callback.hh"

class EventLoop;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, int sockfd);
    ~TcpConnection();

    /* 对外提供的发送接口，用户调用后不用关心是否发送完毕 */
    void Send(const std::string &message);

    void HandleRead();
    /* 用来发送输出缓冲区中剩余的字节 */
    void HandleWrite();
    void ConnectEstablished();

    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

private:
    EventLoop *loop_;
    int sockfd_;
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    std::string inBuf_;
    std::string outBuf_;
};
#endif