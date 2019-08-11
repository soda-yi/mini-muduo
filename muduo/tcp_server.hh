#ifndef TCPSERVER_HH
#define TCPSERVER_HH

#include <sys/epoll.h>

#include <map>
#include <memory>

#include "acceptor.hh"
#include "callbacks.hh"
#include "event_loop.hh"
#include "network.hh"
#include "tcp_connection.hh"

class EventLoopThreadPool;

class TcpServer
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    TcpServer(EventLoop *loop, const EndPoint &endpoint);
    TcpServer(const TcpServer &) = delete;
    TcpServer &operator=(const TcpServer &) = delete;
    TcpServer(TcpServer &&) = default;
    TcpServer &operator=(TcpServer &&) = default;
    ~TcpServer();

    void Start();

    void SetConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void SetMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void SetThreadInitCallback(ThreadInitCallback cb) { threadInitCallback_ = std::move(cb); }
    void SetThreadNum(int numThreads);

private:
    void NewConnection(sockets::Socket socket, const EndPoint &endpoint);
    void RemoveConnection(const TcpConnectionPtr &conn);
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

    EventLoop *loop_;
    Acceptor acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    std::map<int, TcpConnectionPtr> connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    ThreadInitCallback threadInitCallback_;
};

#endif