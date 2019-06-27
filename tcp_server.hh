#ifndef TCPSERVER_HH
#define TCPSERVER_HH

#include <sys/epoll.h>

#include <map>
#include <memory>

#include "callback.hh"

class Acceptor;
class Channel;
class EventLoop;
class TcpConnection;

class TcpServer
{
public:
    TcpServer(EventLoop *loop);
    ~TcpServer();

    void Start();
    void NewConnection(int sockfd);

    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

private:
    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_;
    std::map<int, TcpConnectionPtr> connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};

#endif