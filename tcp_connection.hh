#ifndef TCPCONNECTION_HH
#define TCPCONNECTION_HH

#include <memory>

class EventLoop;
class Channel;

class TcpConnection
{
public:
    TcpConnection(EventLoop *loop, int sockfd);
    ~TcpConnection();

    void OnIn(int sockfd);

private:
    EventLoop *loop_;
    int sockfd_;
    std::unique_ptr<Channel> channel_;
};
#endif