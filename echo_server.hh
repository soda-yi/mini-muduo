#ifndef ECHO_SERVER_HH
#define ECHO_SERVER_HH

#include "tcp_server.hh"

class EchoServer
{
public:
    EchoServer(EventLoop *loop);

    void Start(); // calls server_.start();

private:
    void OnConnection(const TcpConnectionPtr &conn);
    void OnMessage(const TcpConnectionPtr &conn, Buffer *data);
    void OnWriteComplete(const TcpConnectionPtr &conn);

    TcpServer server_;
};

#endif