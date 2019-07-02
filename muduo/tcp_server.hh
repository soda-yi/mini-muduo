#ifndef TCPSERVER_HH
#define TCPSERVER_HH

#include <sys/epoll.h>

#include <map>
#include <memory>

#include "callbacks.hh"
#include "network.hh"

class Acceptor;
class Channel;
class EventLoop;
class TcpConnection;
class EventLoopThreadPool;

class TcpServer
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    TcpServer(EventLoop *loop, const EndPoint &endpoint);
    ~TcpServer();

    void Start();
    void NewConnection(int sockfd, const EndPoint &endpoint);
    void RemoveConnection(const TcpConnectionPtr &conn);
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void SetWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
    void SetThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void SetThreadNum(int numThreads);

private:
    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    std::map<int, TcpConnectionPtr> connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    ThreadInitCallback threadInitCallback_;
};

#endif