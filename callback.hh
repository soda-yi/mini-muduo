#ifndef CALLBACKS_HH
#define CALLBACKS_HH

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &, Buffer *)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;

#endif