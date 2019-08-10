#include "acceptor.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

using std::cout;
using std::endl;

Acceptor::Acceptor(EventLoop *loop, const EndPoint &endpoint)
    : loop_{loop},
      listenfd_{socket(AF_INET, SOCK_STREAM, 0)},
      acceptChannel_{loop_, listenfd_.GetFd()}
{
    listenfd_.SetNonBlock();
    listenfd_.SetReuseAddr();
    listenfd_.Bind(endpoint);
    acceptChannel_.SetReadCallback([this] { HandleRead(); });
    acceptChannel_.Add();
}

Acceptor::~Acceptor()
{
    acceptChannel_.Remove();
}

void Acceptor::HandleRead() const
{
    EndPoint endpoint;
    auto connfd = listenfd_.Accept(&endpoint);
    //TODO: 添加错误处理，记录日志
    connfd.SetNonBlock();

    if (newConnectionCallback_) {
        newConnectionCallback_(std::move(connfd), endpoint);
    }
}

void Acceptor::Listen()
{
    listenfd_.Listen();
    acceptChannel_.EnableReading();
}