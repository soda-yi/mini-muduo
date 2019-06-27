#ifndef ACCEPTOR_HH
#define ACCEPTOR_HH

#include <functional>
#include <memory>

#include "channel.hh"

class EventLoop;

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Acceptor(EventLoop *loop);
    ~Acceptor();

    void OnIn(int socket);
    void SetNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = cb; }
    void Listen();

private:
    EventLoop *loop_;
    int listenfd_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_ = nullptr;
};
#endif