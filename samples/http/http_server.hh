#ifndef SAMPLE_HTTP_HTTPSERVER_HH
#define SAMPLE_HTTP_HTTPSERVER_HH

#include "muduo/tcp_server.hh"
#include "muduo/timestamp.hh"

class HttpRequest;
class HttpResponse;

class HttpServer
{
public:
    typedef std::function<void(const HttpRequest &, HttpResponse *)> HttpCallback;

    HttpServer(EventLoop *loop,
               const EndPoint &listenAddr);

    ~HttpServer(); // force out-line dtor, for scoped_ptr members.

    /// Not thread safe, callback be registered before calling start().
    void SetHttpCallback(const HttpCallback &cb)
    {
        httpCallback_ = cb;
    }

    void Start();

private:
    void OnConnection(const TcpConnectionPtr &conn);
    void OnMessage(const TcpConnectionPtr &conn, Buffer *buf);
    void OnRequest(const TcpConnectionPtr &, const HttpRequest &);

    TcpServer server_;
    HttpCallback httpCallback_;
};

#endif