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

    Acceptor(EventLoop *loop, const EndPoint &endpoint) noexcept;
    Acceptor(const Acceptor &) = delete;
    Acceptor &operator=(const Acceptor &) = delete;
    Acceptor(Acceptor &&) noexcept;
    Acceptor &operator=(Acceptor &&) noexcept;
    ~Acceptor();

    void SetNewConnectionCallback(NewConnectionCallback cb) noexcept { newConnectionCallback_ = std::move(cb); }
    void Listen();

private:
    void HandleRead() const;

    EventLoop *loop_;
    int listenfd_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_ = nullptr;
};
#endif