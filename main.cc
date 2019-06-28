#include "echo_server.hh"
#include "event_loop.hh"

int main()
{
    EventLoop loop;
    EndPoint endpoint{"0.0.0.0", 27272};
    EchoServer echoserver(&loop, endpoint);
    echoserver.Start();
    loop.Loop();
    return 0;
}