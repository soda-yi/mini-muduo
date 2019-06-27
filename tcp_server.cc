#include "tcp_server.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

#include "acceptor.hh"
#include "channel.hh"
#include "tcp_connection.hh"

using std::cout;
using std::endl;

TcpServer::TcpServer(EventLoop *loop)
    : loop_(loop),
      acceptor_(new Acceptor(loop_))
{
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
}

void TcpServer::NewConnection(int sockfd)
{
    auto tcpConnection = std::make_shared<TcpConnection>(loop_, sockfd);
    connections_[sockfd] = tcpConnection;
    tcpConnection->SetConnectionCallback(connectionCallback_);
    tcpConnection->SetMessageCallback(messageCallback_);
    tcpConnection->SetWriteCompleteCallback(writeCompleteCallback_);
    tcpConnection->ConnectEstablished();
}

void TcpServer::Start()
{
    acceptor_->Listen();
}