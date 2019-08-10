#ifndef CHANNEL_HH
#define CHANNEL_HH

#include <functional>

class EventLoop;

/**
 * @brief Channel类，Handle（资源）的实现
 * 
 * 管理一个fd，用于管理系统底层资源，但不持有它\n
 * 本类的实例一般由EventHandle持有，生存期和该EventHandle相同
 */
class Channel
{
public:
    /**
     * @brief 各种事件的回调接口
     * 
     * 事件的回调接口，包括了读、写、关闭事件
     */
    using EventCallback = std::function<void()>;

    /**
     * @brief 通过loop和fd构造Channel对象
     * 
     * @param loop 所属的EventLoop
     * @param fd 文件描述符，可以是sockfd、eventfd、timerfd等，通常由持有本对象的对象直接生成并提供
     */
    Channel(EventLoop *loop, int fd) noexcept;
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    Channel(Channel &&) = default;
    Channel &operator=(Channel &&) = default;

    void SetReadCallback(EventCallback cb) noexcept { readCallback_ = std::move(cb); }
    void SetWriteCallback(EventCallback cb) noexcept { writeCallback_ = std::move(cb); }
    void SetCloseCallback(EventCallback cb) noexcept { closeCallback_ = std::move(cb); }

    /**
     * @brief 设置Channel持有的fd发生的事件
     * 
     * @param revent fd发生的事件，由Synchronous Event Demultiplexer（EpollPoller）等待后获得
     */
    void SetRevents(int revent) noexcept { revents_ = revent; }
    int GetFd() const noexcept { return fd_; }
    /**
     * @brief 获取关心的事件
     * 
     * @return 关心的事件
     */
    int GetEvents() const noexcept { return events_; }

    /**
     * @brief 允许Channel读操作
     * 
     * @warning 调用前需确保已经调用过Add
     * 
     * 设置对读事件关心，并修改fd对应的epoll_event
     */
    void EnableReading();
    /**
     * @brief 禁止Channel读操作
     * 
     * @warning 调用前需确保已经调用过Add
     * 
     * 设置对读事件不关心，并修改fd对应的epoll_event
     */
    void DisableReading();
    /**
     * @brief 允许Channel写操作
     * 
     * @warning 调用前需确保已经调用过Add
     * 
     * 设置对写事件关心，并修改fd对应的epoll_event
     */
    void EnableWriting();
    /**
     * @brief 禁止Channel写操作
     * 
     * @warning 调用前需确保已经调用过Add
     * 
     * 设置对写事件不关心，并修改fd对应的epoll_event
     */
    void DisableWriting();
    /**
     * @brief 禁止Channel所有操作
     * 
     * @warning 调用前需确保已经调用过Add
     * 
     * 设置对读写事件不关心，但不将对应的fd从epoll移除，想恢复不用重新Add
     */
    void DisableAll();

    /**
     * @brief 判断Channel是否对读事件关心
     * 
     * @return true 关心
     * @return false 不关心
     */
    bool IsWriting() const noexcept;
    /**
     * @brief 判断Channel是否对写事件关心
     * 
     * @return true 关心
     * @return false 不关心
     */
    bool IsReading() const noexcept;

    /**
     * @brief 处理发生的事件
     * 
     * 一次性处理完该Channel发生的事件，三种事件分别调用三种回调函数\n
     * 通常在Poller执行Poll得到发生事件的Channel后调用
     */
    void HandleEvent() const;
    /**
     * @brief 添加Channel到EventLoop对应的Poller中
     */
    void Add();
    /**
     * @brief 将Channel从其EventLoop对应的Poller中移除
     * 
     * 将对应的fd从epoll移除，想要恢复需要重新Add
     */
    void Remove();

private:
    /**
     * @brief Channel状态的枚举类
     * 
     * 三个Channel的状态，只在Channel执行Update、Remove操作时改变 @see ::Update::Remove
     */
    enum class State : char {
        kNew,   /**< Channel是新建的或是已经被删除（EPOLL_CTL_DEL），fd还未加入epoll中 */
        kAdded, /**< Channel已经加入epoll中（EPOLL_CTL_ADD） */
    };

    /**
     * @brief 在Poller中更新关心的事件
     * 
     * 更新关心的事件，所有设置读写状态的方法都要调用它
     */
    inline void Update();

    EventLoop *loop_;
    int fd_;
    int events_ = 0;  /**< 关注的事件 */
    int revents_ = 0; /**< 发生的事件 */
    State state_ = State::kNew;
    EventCallback readCallback_ = nullptr;
    EventCallback writeCallback_ = nullptr;
    EventCallback closeCallback_ = nullptr;
};

#endif