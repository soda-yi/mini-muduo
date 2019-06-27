#include "acceptor.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <iostream>

using std::cout;
using std::endl;

Acceptor::Acceptor(int epollfd)
    : epollfd_(epollfd)
{
}

Acceptor::~Acceptor()
{
}

int Acceptor::CreateAndListen()
{
    int on = 1;
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    fcntl(listenfd_, F_SETFL, O_NONBLOCK); //no-block io
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(11111);

    if (-1 == bind(listenfd_, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        cout << "bind error, errno:" << errno << endl;
    }

    constexpr int backlog = 10;
    if (-1 == listen(listenfd_, backlog)) {
        cout << "listen error, errno:" << errno << endl;
    }

    return listenfd_;
}

void Acceptor::OnIn(int socket)
{
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(struct sockaddr_in);
    connfd = accept(listenfd_, (sockaddr *)&cliaddr, (socklen_t *)&clilen);
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

    callback_(connfd);
}

void Acceptor::Start()
{
    listenfd_ = CreateAndListen();
    channel_ = std::make_shared<Channel>(epollfd_, listenfd_);
    channel_->SetCallback(std::bind(&Acceptor::OnIn, this, std::placeholders::_1));
    channel_->EnableReading();
}