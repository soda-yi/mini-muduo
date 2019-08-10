#ifndef ACCEPTOR_HH
#define ACCEPTOR_HH

#include <functional>
#include <memory>

#include "channel.hh"
#include "network.hh"

#include "file_descriptor.hh"

class EventLoop;

/**
 * @brief Acceptor类，EventHandle的一种实现
 * 
 * 用于接收外部TCP连接，构造后调用Listen可进行监听\n
 * 一般每个TcpServer持有一个本类的实例，用于监听该Server所有的连接
 */
class Acceptor
{
public:
    /**
     * @brief 有新的socket连接时的回调
     * 
     * @param sockfd 新建立连接的socket
     * @param endpoint 新socket对应的endpoint
     */
    using NewConnectionCallback = std::function<void(sockets::Socket socket, const EndPoint &endpoint)>;

    /**
     * @brief 构造一个Acceptor对象
     * 
     * @param loop 所属的EventLoop
     * @param endpoint 进行监听的EndPoint
     */
    Acceptor(EventLoop *loop, const EndPoint &endpoint);
    Acceptor(const Acceptor &) = delete;
    Acceptor &operator=(const Acceptor &) = delete;
    Acceptor(Acceptor &&) = default;
    Acceptor &operator=(Acceptor &&) = default;
    ~Acceptor();

    void SetNewConnectionCallback(NewConnectionCallback cb) noexcept { newConnectionCallback_ = std::move(cb); }

    /**
     * @brief 开始监听listenfd_
     * 
     * 构造完成后调用，设置acceptChannel_读关心并监听listenfd_
     */
    void Listen();

private:
    /**
     * @brief 处理读事件
     * 
     * EventHandle模型中处理读事件的Handle，作为Channel的回调函数使用
     */
    void HandleRead() const;

    EventLoop *loop_;          /**< 所属的EventLoop，一般是TcpServer的EventLoop（主IO线程） */
    sockets::Socket listenfd_; /**< 持有的socket，负责创建和销毁 */
    Channel acceptChannel_;    /**< 持有的Handle，通常只对读关心 */
    NewConnectionCallback newConnectionCallback_ = nullptr;
};
#endif