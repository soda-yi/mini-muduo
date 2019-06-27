#ifndef TCPSERVER_HH
#define TCPSERVER_HH

#include <sys/epoll.h>

#include <map>
#include <memory>

#include "acceptor.hh"
#include "channel.hh"
#include "tcp_connection.hh"

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();
    void Start();
    void NewConnection(int sockfd);

private:
    void Update(std::shared_ptr<Channel> pChannel, int op);

    static constexpr int kMaxEvents = 500;

    int epollfd_ = -1;
    struct epoll_event events_[kMaxEvents];
    std::map<int, std::shared_ptr<TcpConnection>> connections_;
    std::shared_ptr<Acceptor> acceptor_ = nullptr;
};

#endif