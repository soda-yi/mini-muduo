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
#include "event_loop.hh"
#include "tcp_connection.hh"

using std::cout;
using std::endl;

TcpServer::TcpServer(EventLoop *loop, const EndPoint &endpoint)
    : loop_(loop),
      acceptor_(new Acceptor(loop_, endpoint))
{
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for (auto &item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->GetLoop()->RunInLoop(
            std::bind(&TcpConnection::ConnectDestroyed, conn));
    }
}

void TcpServer::NewConnection(int sockfd, const EndPoint &endpoint)
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