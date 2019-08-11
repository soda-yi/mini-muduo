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

/**
 * @brief TcpServer类
 * 
 * 管理所有的连接，处理连接的新建移除
 */
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

    /**
     * @brief 启动一个TcpServer
     * 
     * 调用Acceptor的Listen方法，使listenfd_处于LISTEN状态，以启动一个Server
     */
    void Start();

    void SetConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void SetMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void SetThreadInitCallback(ThreadInitCallback cb) { threadInitCallback_ = std::move(cb); }
    void SetThreadNum(int numThreads);

private:
    /**
     * @brief 新建一条连接
     * 
     * 通常作为acceptor_的NewConnectionCallback
     */
    void NewConnection(sockets::Socket socket, const EndPoint &endpoint);
    /**
     * @brief 移除一条连接
     * 
     * @param conn 
     * 
     * 通常作为conn的NewConnectionCallback
     */
    void RemoveConnection(const TcpConnectionPtr &conn);
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

    EventLoop *loop_; /**< Server的主Loop，acceptor_也共用此loop */
    Acceptor acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    std::map<int, TcpConnectionPtr> connections_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    ThreadInitCallback threadInitCallback_;
};

#endif