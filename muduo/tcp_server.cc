#include "tcp_server.hh"

#include <vector>

TcpServer::TcpServer(EventLoop *loop, const EndPoint &endpoint)
    : loop_{loop},
      acceptor_{loop_, endpoint},
      threadPool_{std::make_shared<EventLoopThreadPool>(loop_, 4)}
{
    acceptor_.SetNewConnectionCallback(
        [this](sockets::Socket socket, const EndPoint &newEndpoint) { NewConnection(std::move(socket), newEndpoint); });
}

TcpServer::~TcpServer()
{
    for (auto &item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->GetLoop()->RunInLoop(
            [conn] { conn->ConnectDestroyed(); });
    }
}

void TcpServer::NewConnection(sockets::Socket socket, const EndPoint &endpoint)
{
    EventLoop *ioLoop = threadPool_->GetNextLoop();
    auto tcpConnection = std::make_shared<TcpConnection>(ioLoop, std::move(socket));
    connections_[tcpConnection->GetFd()] = tcpConnection;
    tcpConnection->SetConnectionCallback(connectionCallback_);
    tcpConnection->SetMessageCallback(messageCallback_);
    tcpConnection->SetCloseCallback([this](const TcpConnectionPtr &conn) { RemoveConnection(conn); });
    tcpConnection->SetPeerEndPoint(endpoint);
    ioLoop->RunInLoop([tcpConnection] { tcpConnection->ConnectEstablished(); });
    //cout << "Main Loop thread: " << std::this_thread::get_id() << endl;
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn)
{
    loop_->RunInLoop([this, conn] { RemoveConnectionInLoop(conn); });
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn)
{
    connections_.erase(conn->GetFd());
    EventLoop *ioLoop = conn->GetLoop();
    ioLoop->QueueInLoop([conn] { conn->ConnectDestroyed(); });
}

void TcpServer::Start()
{
    loop_->RunInLoop([this] { acceptor_.Listen(); });
}