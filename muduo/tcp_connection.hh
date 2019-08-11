#ifndef TCPCONNECTION_HH
#define TCPCONNECTION_HH

#include <any>
#include <memory>
#include <string>

#include "buffer.hh"
#include "callbacks.hh"
#include "channel.hh"
#include "file_descriptor.hh"

class EventLoop;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

/**
 * @brief TcpConnection类，EventHandle的一种实现，表示一条Tcp连接
 * 
 * 当Acceptor接收一条新的连接时，生成一个新的本类实例。\n
 * 该类是暴露给用户的接口之一。由于其对socket资源管理的独有性，导致其不能被copy。\n
 * 也正是由于此，暴露给用户使用的是它的指针。涉及资源管理，使用std::shared_ptr管理该指针，\n
 * 所以该类继承std::enable_shared_from_this
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
private:
    enum class StateE : char { kDisconnected, /**< 已断开连接 */
                               kConnecting,
                               kConnected,
                               kDisconnecting };

public:
    /**
     * @brief CloseCallback回调，连接关闭
     * 
     * 提供给TcpServer使用的接口，主要完成连接关闭过程中TcpServer中资源的清理以及对ConnectionDestroyed的封装
     */
    using CloseCallback = std::function<void(const TcpConnectionPtr &conn)>;
    /**
     * @brief 构造一个TcpConnection对象
     * 
     * @param loop 
     * @param socket 对象的socket信息
     * 
     * 由于每一个TcpConnect的生成都是从Acceptor::Accept后生成的\n
     * 然而在Acceptor::Accept后已经生成一个Socket对象，所以用此对象构造TcpConnection
     */
    TcpConnection(EventLoop *loop, sockets::Socket socket);
    TcpConnection(const TcpConnection &) = delete;
    TcpConnection &operator=(const TcpConnection &) = delete;
    TcpConnection(TcpConnection &&) = default;
    TcpConnection &operator=(TcpConnection &&) = default;
    ~TcpConnection();

    /**
     * @brief 判断是否处于连接状态
     */
    bool IsConnected() const noexcept { return state_ == StateE::kConnected; }
    /**
     * @brief 判断连接是否断开完成
     */
    bool IsDisConnected() const noexcept { return state_ == StateE::kDisconnected; }

    /**
     * @brief 向Peer发送一条消息
     * 
     * @param message 消息内容
     * 
     * @bug 当有多个线程同时调用多条消息发送时，会出现多条消息一次合并后发送，\n
     * 这时会出现处于后面的调用发送0字节导致多调用依次WriteCompleteCallback的情况
     * 
     * 对外提供的发送接口，用户调用后会自动发送直至所有内容发送完毕\n
     * 发送完毕后，调用用户设置过的WriteCompleteCallback
     */
    void Send(const std::string &message);
    /**
     * @brief 关闭连接
     * 
     * 对外提供的关闭连接接口，通过shutdown己方wr关闭一条连接。\n
     * 关闭后，Peer会探测到连接关闭，如果Peer也关闭连接，则会调用HandleClose处理
     */
    void Shutdown();
    /**
     * @brief 强制关闭连接
     * 
     * 对外提供的强制关闭连接接口，调用后不再等待Peer关闭连接而是自己主动关闭
     */
    void ForceClose();

    /**
     * @brief 连接建立后的收尾工作
     * 
     * 提供给TcpServer使用的接口，用于设置state及调用ConnectionCallback等类内操作
     */
    void ConnectEstablished();
    /**
     * @brief 关闭连接后的收尾工作
     * 
     * 提供给TcpServer使用的接口，用于调用ConnectionCallback、关闭Socket以及移除Channel等类内操作\n
     * 通常被TcpServer包装后以CloseCallback的形式回传，在HandleClose中被调用
     */
    void ConnectDestroyed();

    void SetConnectionCallback(ConnectionCallback cb) noexcept { connectionCallback_ = std::move(cb); }
    void SetMessageCallback(MessageCallback cb) noexcept { messageCallback_ = std::move(cb); }
    void SetWriteCompleteCallback(WriteCompleteCallback cb) noexcept { writeCompleteCallback_ = std::move(cb); }
    void SetCloseCallback(CloseCallback cb) noexcept { closeCallback_ = std::move(cb); }

    void SetContext(std::any context) noexcept { context_ = std::move(context); }
    std::any &GetContext() noexcept { return context_; }
    void SetPeerEndPoint(EndPoint endPoint) { endPoint_ = std::move(endPoint); }
    EndPoint GetPeerEndPoint() const { return endPoint_; }

    EventLoop *GetLoop() noexcept { return loop_; }
    const EventLoop *GetLoop() const noexcept { return loop_; }
    int GetFd() const noexcept { return socket_.GetFd(); }

private:
    /**
     * @brief 处理读事件
     * 
     * 通过回调的方式注册到Channel，当Channel有读事件时调用\n
     * 每次Read完，调用MessageCallback回调
     */
    void HandleRead();
    /**
     * @brief 处理写事件
     * 
     * 通过回调的方式注册到Channel，当Channel有写事件（系统缓冲区可写）时调用\n
     * 发送写缓冲区中的数据，当写缓冲区的数据全部发送完毕后，调用WriteCompleteCallback回调
     */
    void HandleWrite();
    /**
     * @brief 处理对端关闭事件
     * 
     * 通过回调的方式注册到Channel，当对端关闭连接时（EPOLLRDHUP事件）调用\n
     * 主要处理对端关闭后己方的后续清理工作，包括从epoll的remove，关闭socket等
     */
    void HandleClose();

    EventLoop *loop_;
    StateE state_;
    sockets::Socket socket_;
    Channel channel_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    Buffer inBuf_;
    Buffer outBuf_;
    std::any context_;
    EndPoint endPoint_;
};

#endif