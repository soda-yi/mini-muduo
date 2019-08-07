#include "acceptor.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

using std::cout;
using std::endl;

Acceptor::Acceptor(EventLoop *loop, const EndPoint &endpoint) noexcept
    : loop_{loop},
      listenfd_{socket(AF_INET, SOCK_STREAM, 0)},
      acceptChannel_{loop_, listenfd_}
{
    int on = 1;
    struct sockaddr_in servaddr;
    fcntl(listenfd_, F_SETFL, O_NONBLOCK); //no-block io
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(endpoint.ip_addr.c_str());
    servaddr.sin_port = htons(endpoint.port);

    if (-1 == bind(listenfd_, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        cout << "bind error, errno:" << errno << endl;
    }

    acceptChannel_.SetReadCallback([this] { HandleRead(); });
}

Acceptor::~Acceptor()
{
    acceptChannel_.Remove();
    ::close(listenfd_);
}

void Acceptor::HandleRead() const
{
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(struct sockaddr_in);
    int connfd = accept(listenfd_, (sockaddr *)&cliaddr, (socklen_t *)&clilen);
    if (connfd > 0) {
        cout << "new connection from "
             << "[" << inet_ntoa(cliaddr.sin_addr)
             << ":" << ntohs(cliaddr.sin_port) << "]"
             << " new socket fd:" << connfd
             << endl;
    } else {
        cout << "accept error, connfd:" << connfd
             << " errno:" << errno << endl;
    }
    fcntl(connfd, F_SETFL, O_NONBLOCK); //no-block io

    EndPoint endpoint{inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port)};
    if (newConnectionCallback_) {
        newConnectionCallback_(connfd, endpoint);
    }
}

void Acceptor::Listen()
{
    constexpr int backlog = 10;
    if (-1 == listen(listenfd_, backlog)) {
        cout << "listen error, errno:" << errno << endl;
    }
    acceptChannel_.EnableReading();
}