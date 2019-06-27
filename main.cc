#include "echo_server.hh"
#include "event_loop.hh"

int main()
{
    EventLoop loop;
    EchoServer echoserver(&loop);
    echoserver.Start();
    loop.Loop();
    return 0;
}