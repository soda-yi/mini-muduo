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

void TcpServer::Start()
{
    epollfd_ = epoll_create1(1);
    if (epollfd_ < 0) {
        cout << "epoll_create error, error:" << epollfd_ << endl;
    }

    listenfd_ = CreateAndListen();
    auto pChannel = std::make_shared<Channel>(epollfd_, listenfd_);
    pChannel->SetCallback(std::bind(&TcpServer::OnIn, this, std::placeholders::_1));
    pChannel->EnableReading();
    channels_.insert({pChannel->GetSockfd(), pChannel});

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

int TcpServer::CreateAndListen()
{
    int on = 1;
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    fcntl(listenfd, F_SETFL, O_NONBLOCK); //no-block io
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(11111);

    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        cout << "bind error, errno:" << errno << endl;
    }

    constexpr int backlog = 10;
    if (-1 == listen(listenfd, backlog)) {
        cout << "listen error, errno:" << errno << endl;
    }

    return listenfd;
}

void TcpServer::OnIn(int sockfd)
{
    constexpr int kMaxLength = 100;
    cout << "OnIn:" << sockfd << endl;
    if (sockfd == listenfd_) {
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

        auto pChannel = std::make_shared<Channel>(epollfd_, connfd);
        pChannel->SetCallback(std::bind(&TcpServer::OnIn, this, std::placeholders::_1));
        pChannel->EnableReading();
        channels_.insert({pChannel->GetSockfd(), pChannel});
    } else {
        int readlength;
        char line[kMaxLength];
        if (sockfd < 0) {
            cout << "EPOLLIN sockfd < 0 error " << endl;
            return;
        }
        bzero(line, kMaxLength);
        if ((readlength = read(sockfd, line, kMaxLength)) < 0) {
            if (errno == ECONNRESET) {
                cout << "ECONNREST closed socket fd:" << sockfd << endl;
                close(sockfd);
            }
        } else if (readlength == 0) {
            cout << "read 0 closed socket fd:" << sockfd << endl;
            close(sockfd);
        } else {
            if (write(sockfd, line, readlength) != readlength) {
                cout << "error: not finished one time" << endl;
            }
        }
    }
}