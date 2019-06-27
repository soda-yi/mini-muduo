#include "tcp_server.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

using std::cout;
using std::endl;

TcpServer::TcpServer()
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::NewConnection(int sockfd)
{
    auto tcp = std::make_shared<TcpConnection>(epollfd_, sockfd);
    connections_[sockfd] = tcp;
}

void TcpServer::Start()
{
    epollfd_ = epoll_create1(1);
    if (epollfd_ < 0) {
        cout << "epoll_create error, error:" << epollfd_ << endl;
    }

    acceptor_ = std::make_shared<Acceptor>(epollfd_);
    acceptor_->SetCallBack(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
    acceptor_->Start();

    for (;;) {
        std::vector<Channel *> rchannels;
        int fds = epoll_wait(epollfd_, events_, kMaxEvents, -1);
        if (fds == -1) {
            cout << "epoll_wait error, errno:" << errno << endl;
            break;
        }
        for (int i = 0; i < fds; i++) {
            Channel *pChannel = static_cast<Channel *>(events_[i].data.ptr);
            pChannel->SetRevents(events_[i].events);
            rchannels.push_back(pChannel);
        }
        for (auto &channel : rchannels) {
            channel->HandleEvent();
        }
    }
}