#ifndef TCPCONNECTION_HH
#define TCPCONNECTION_HH

#include <memory>

#include "channel.hh"

class TcpConnection
{
public:
    TcpConnection(int epollfd, int sockfd);
    ~TcpConnection();

    void OnIn(int sockfd);

private:
    int epollfd_;
    int sockfd_;
    std::shared_ptr<Channel> channel_ = nullptr;
};
#endif