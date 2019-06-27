#ifndef ECHO_SERVER_HH
#define ECHO_SERVER_HH

#include "tcp_server.hh"
#include "timer_id.hh"

class EchoServer
{
public:
    EchoServer(EventLoop *loop);

    void Start(); // calls server_.start();

private:
    void OnConnection(const TcpConnectionPtr &conn);
    void OnMessage(const TcpConnectionPtr &conn, Buffer *data);
    void OnWriteComplete(const TcpConnectionPtr &conn);
    void Run();

    TcpServer server_;
    EventLoop *loop_;
    TimerId timerId_;
    int index_ = 0;
};

#endif