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
#include "event_loop_thread_pool.hh"
#include "tcp_connection.hh"

using std::cout;
using std::endl;

TcpServer::TcpServer(EventLoop *loop, const EndPoint &endpoint)
    : loop_(loop),
      acceptor_(new Acceptor(loop_, endpoint)),
      threadPool_(new EventLoopThreadPool(loop_))
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

void TcpServer::SetThreadNum(int numThreads)
{
    threadPool_->SetThreadNum(numThreads);
}

void TcpServer::NewConnection(int sockfd, const EndPoint &endpoint)
{
    EventLoop *ioLoop = threadPool_->GetNextLoop();
    auto tcpConnection = std::make_shared<TcpConnection>(ioLoop, sockfd);
    connections_[sockfd] = tcpConnection;
    tcpConnection->SetConnectionCallback(connectionCallback_);
    tcpConnection->SetMessageCallback(messageCallback_);
    tcpConnection->SetWriteCompleteCallback(writeCompleteCallback_);
    tcpConnection->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    ioLoop->RunInLoop([&tcpConnection] { tcpConnection->ConnectEstablished(); });
    //cout << "Main Loop thread: " << std::this_thread::get_id() << endl;
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn)
{
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn)
{
    connections_.erase(conn->GetSockfd());
    EventLoop *ioLoop = conn->GetLoop();
    ioLoop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}

void TcpServer::Start()
{
    threadPool_->Start(threadInitCallback_);
    loop_->RunInLoop([this] { acceptor_->Listen(); });
}