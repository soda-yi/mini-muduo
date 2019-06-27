#include "event_loop.hh"
#include "tcp_server.hh"

int main()
{
    EventLoop loop;
    TcpServer tcpserver(&loop);
    tcpserver.Start();
    loop.Loop();
    return 0;
}