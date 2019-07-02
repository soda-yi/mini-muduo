#include "http_server.hh"

#include "http_context.hh"
#include "http_request.hh"
#include "http_response.hh"
#include "muduo/tcp_connection.hh"

void DefaultHttpCallback(const HttpRequest &, HttpResponse *resp)
{
    resp->SetStatusCode(HttpResponse::k404NotFound);
    resp->SetStatusMessage("Not Found");
    resp->SetCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop, const EndPoint &listenAddr)
    : server_(loop, listenAddr),
      httpCallback_(DefaultHttpCallback)
{
    server_.SetConnectionCallback(
        std::bind(&HttpServer::OnConnection, this, std::placeholders::_1));
    server_.SetMessageCallback(
        std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
}

HttpServer::~HttpServer()
{
}

void HttpServer::Start()
{
    server_.Start();
}

void HttpServer::OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->IsConnected()) {
        conn->SetContext(std::make_shared<HttpContext>());
    }
}

void HttpServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buf)
{
    auto context = conn->GetContext<HttpContext>();

    // if (!detail::parseRequest(buf, context, receiveTime))
    if (!context->ParseRequest(buf)) {
        conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->Shutdown();
    }

    if (context->GotAll()) {
        OnRequest(conn, context->GetRequest());
        context->Reset();
    }
}

void HttpServer::OnRequest(const TcpConnectionPtr &conn, const HttpRequest &req)
{
    const std::string &connection = req.GetHeader("Connection");
    bool close = connection == "close" ||
                 (req.GetVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.AppendToBuffer(&buf);
    conn->Send(buf.RetrieveAllAsString());
    if (response.GetCloseConnection()) {
        conn->Shutdown();
    }
}
