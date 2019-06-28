#include "echo_server.hh"
#include "tcp_connection.hh"

#include <iostream>

#include "event_loop.hh"

using std::cout;
using std::endl;
using std::placeholders::_1;
using std::placeholders::_2;

EchoServer::EchoServer(EventLoop *loop)
    : server_(loop), loop_(loop)
{
    server_.SetConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, _1));
    server_.SetMessageCallback(
        std::bind(&EchoServer::OnMessage, this, _1, _2));
    server_.SetWriteCompleteCallback(
        std::bind(&EchoServer::OnWriteComplete, this, _1));
}

void EchoServer::Start()
{
    server_.Start();
}

void EchoServer::OnConnection(const TcpConnectionPtr &conn)
{
    cout << "onConnection" << endl;
}

void EchoServer::OnMessage(const TcpConnectionPtr &conn,
                           Buffer *data)
{
    constexpr size_t kMessageLength = 8;
    while (data->ReadableBytes() > kMessageLength) {
        std::string message = data->RetrieveAsString(kMessageLength);
        conn->Send(message + "\n");
    }
    conn->Send(data->RetrieveAllAsString() + "\n");
    timerId_ = loop_->RunEvery(0.5, std::bind(&EchoServer::Run, this));
}

void EchoServer::OnWriteComplete(const TcpConnectionPtr &conn)
{
    cout << "onWriteComplete" << endl;
}

void EchoServer::Run()
{
    cout << index_ << endl;
    if (index_++ == 3) {
        loop_->CancelTimer(timerId_);
        index_ = 0;
    }
}