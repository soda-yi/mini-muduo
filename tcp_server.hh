#ifndef TCPSERVER_HH
#define TCPSERVER_HH

#include <sys/epoll.h>

#include <map>
#include <memory>

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

private:
    static constexpr int kMaxEvents = 500;

    EventLoop *loop_;
    struct epoll_event events_[kMaxEvents];
    std::map<int, std::shared_ptr<TcpConnection>> connections_;
    std::unique_ptr<Acceptor> acceptor_;
};

#endif