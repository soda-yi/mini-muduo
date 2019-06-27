#ifndef ACCEPTOR_HH
#define ACCEPTOR_HH

#include <functional>
#include <memory>

#include "channel.hh"

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Acceptor(int epollfd);
    ~Acceptor();

    void OnIn(int socket);
    void SetCallBack(NewConnectionCallback callback) { callback_ = std::move(callback); }
    void Start();

private:
    int CreateAndListen();
    int epollfd_;
    int listenfd_ = -1;
    std::shared_ptr<Channel> channel_ = nullptr;
    NewConnectionCallback callback_ = nullptr;
};
#endif