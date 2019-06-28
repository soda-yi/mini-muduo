#ifndef ECHO_SERVER_HH
#define ECHO_SERVER_HH

#include "tcp_server.hh"
#include "thread_pool.hh"
#include "timer_id.hh"

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const EndPoint &endpoint);

    void Start(); // calls server_.start();

private:
    void OnConnection(const TcpConnectionPtr &conn);
    void OnMessage(const TcpConnectionPtr &conn, Buffer *data);
    void OnWriteComplete(const TcpConnectionPtr &conn);
    void Run(int n, const TcpConnectionPtr &conn, const std::string &message);

    TcpServer server_;
    EventLoop *loop_;
    TimerId timerId_;
    ThreadPool threadPool_;
    int index_ = 0;
};

#endif