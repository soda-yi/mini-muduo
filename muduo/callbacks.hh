#ifndef CALLBACKS_HH
#define CALLBACKS_HH

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;
/**
 * @brief ConnectionCallback回调，连接状态改变
 * 
 * 当conn的状态改变时调用的回调函数，状态改变指的是：\n
 * - Connecting->Connected时，表示连接建立\n
 * - Connected->Disconnected时，表示连接关闭\n
 * 可以通过conn的特定方法来判断其处于哪种状态
 */
using ConnectionCallback = std::function<void(const TcpConnectionPtr &conn)>;
/**
 * @brief WriteCompleteCallback回调，本次发送完成
 * 
 * 每次调用Send后，当数据发送完成后都会进行调用（而不是等待缓冲区数据全部发送完再调用）
 */
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &conn)>;
/**
 * @brief MessageCallback回调，有消息到达
 * 
 * 每次调用Socket::Read后，若有收到数据则调用
 */
using MessageCallback = std::function<void(const TcpConnectionPtr &conn, Buffer *buf)>;

#endif