#include "echo_server.hh"

#include <iostream>

#include "muduo/event_loop.hh"
#include "muduo/tcp_connection.hh"

using std::cout;
using std::endl;
using std::placeholders::_1;
using std::placeholders::_2;

namespace
{
int Fib(int n)
{
    return (n == 1 || n == 2) ? 1 : (Fib(n - 1) + Fib(n - 2));
}
} // namespace

EchoServer::EchoServer(EventLoop *loop, const EndPoint &endpoint)
    : server_(loop, endpoint), loop_(loop), threadPool_(3)
{
    server_.SetConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&EchoServer::OnMessage, this, _1, _2));
}

void EchoServer::Start()
{
    server_.Start();
}

void EchoServer::OnConnection(const TcpConnectionPtr &conn)
{
    using namespace std::chrono_literals;
    cout << "EchoServer - " << conn->GetPeerEndPoint().ip_addr << conn->GetPeerEndPoint().port
         << " is "
         << (conn->IsConnected() ? "UP" : "DOWN") << endl;
    threadPool_.Commit([this, conn] {
        //auto id1 = loop_->RunAfter(10s, [conn] { conn->Send("After 10s\n"); });
        auto id2 = loop_->RunAfter(5s, [conn] { conn->Send("After 5s\n"); });
        //auto id3 = loop_->RunAfter(4s, [conn] { conn->Send("After 4s\n"); });
        auto id4 = loop_->RunAfter(2s, [conn] { conn->Send("After 2s\n"); });
        //auto id5 = loop_->RunEvery(1s, [conn] { conn->Send("Every 1s\n"); });
        //std::this_thread::sleep_for(10s);
        //loop_->CancelTimer(id2);
    });
}

void EchoServer::OnMessage(const TcpConnectionPtr &conn,
                           Buffer *data)
{
    constexpr size_t kMessageLength = 8;

    while (data->ReadableBytes() != 0) {
        std::string message = data->RetrieveAsString(kMessageLength);
        threadPool_.Commit([this, conn, message] { Run(30, conn, message); });
    }
}

void EchoServer::OnWriteComplete(const TcpConnectionPtr &conn)
{
    cout << "OnWriteComplete" << endl;
}

void EchoServer::Run(int n, const TcpConnectionPtr &conn, const std::string &message)
{
    //cout << "Fib(" << n << ") = " << Fib(30) << endl;
    //cout << "Work thread: " << std::this_thread::get_id() << endl;
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    conn->Send(message + "\n", std::bind(&EchoServer::OnWriteComplete, this, _1));
}