#include "tcp_connection.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>

#include "channel.hh"

using std::cout;
using std::endl;

TcpConnection::TcpConnection(EventLoop *loop, int sockfd)
    : loop_(loop),
      sockfd_(sockfd),
      channel_(new Channel(loop_, sockfd_))
{
    channel_->SetCallback(std::bind(&TcpConnection::OnIn, this, std::placeholders::_1));
    channel_->EnableReading();
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::OnIn(int sockfd)
{
    constexpr int kMaxLength = 100;
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
        std::string buf(line, kMaxLength);
        messageCallback_(shared_from_this(), buf);
    }
}

void TcpConnection::Send(const std::string &message)
{
    int n = ::write(sockfd_, message.c_str(), message.size());
    if (n != static_cast<int>(message.size())) {
        cout << "write error ! " << message.size() - n << "bytes left" << endl;
    }
}

void TcpConnection::ConnectEstablished()
{
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}