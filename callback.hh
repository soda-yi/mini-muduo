#ifndef CALLBACKS_HH
#define CALLBACKS_HH

#include <functional>
#include <memory>
#include <string>

class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &, std::string *)> MessageCallback;

#endif