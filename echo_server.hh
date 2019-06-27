#ifndef MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H
#define MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H

#include "tcp_server.hh"

class EchoServer
{
public:
    EchoServer(EventLoop *loop);

    void Start(); // calls server_.start();

private:
    void OnConnection(const TcpConnectionPtr &conn);

    void OnMessage(const TcpConnectionPtr &conn,
                   const std::string &buf);

    TcpServer server_;
};

#endif