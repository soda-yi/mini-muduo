#ifndef TCPSERVER_HH
#define TCPSERVER_HH

#include <sys/epoll.h>

#include <map>
#include <memory>

#include "channel.hh"

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();
    void Start();
    void OnIn(int sockfd);

private:
    int CreateAndListen();
    void Update(std::shared_ptr<Channel> pChannel, int op);

    static constexpr int kMaxEvents = 500;

    int epollfd_ = -1;
    int listenfd_ = -1;
    struct epoll_event events_[kMaxEvents];
    std::map<int, std::shared_ptr<Channel>> channels_;
};

#endif