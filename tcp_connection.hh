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

    void Send(const std::string &message);

    void OnIn(int sockfd);
    void ConnectEstablished();

    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

private:
    EventLoop *loop_;
    int sockfd_;
    std::unique_ptr<Channel> channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};
#endif