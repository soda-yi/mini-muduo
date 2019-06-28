#ifndef ACCEPTOR_HH
#define ACCEPTOR_HH

#include <functional>
#include <memory>

#include "channel.hh"
#include "network.hh"

class EventLoop;

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const EndPoint &endpoint)>;

    Acceptor(EventLoop *loop, const EndPoint &endpoint);
    ~Acceptor();

    void HandleRead();
    void SetNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }
    void Listen();

private:
    EventLoop *loop_;
    int listenfd_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_ = nullptr;
};
#endif