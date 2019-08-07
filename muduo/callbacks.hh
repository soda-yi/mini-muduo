#ifndef CALLBACKS_HH
#define CALLBACKS_HH

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *)>;

#endif