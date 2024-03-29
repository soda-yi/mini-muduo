#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "callbacks.hh"
#include "channel.hh"
#include "epoll_poller.hh"
#include "file_descriptor.hh"
#include "timer_queue.hh"

/**
 * @brief EventLoop类
 */
class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;
    EventLoop(EventLoop &&) = default;
    EventLoop &operator=(EventLoop &&) = default;
    ~EventLoop();

    /**
     * @brief EventLoop的主循环部分。调用后可以通过Quit退出
     * 
     * 同步异步执行回调定义：
     * - 同步：对Poll结果有事件发生的Channel执行HandleEvent中的执行的回调
     * - 异步：在DoPendingFunctors中执行的回调
     */
    void Loop();
    /**
     * @brief 退出Loop循环
     */
    void Quit();

    void AddChannel(const Channel &channel) const noexcept { poller_.AddChannel(channel); }
    void UpdateChannel(const Channel &channel) const noexcept { poller_.UpdateChannel(channel); }
    void RemoveChannel(const Channel &channel) const noexcept { poller_.RemoveChannel(channel); }

    /**
     * @brief 在IO线程内异步执行回调操作
     * 
     * @param functor 待执行的操作
     * 
     * @warning 确保QueueInLoop的被调用位置一定位于本Loop的IO线程，否则应该使用RunInLoop
     * 
     * 将待执行的操作插入pendingFunctors_集合中，待下次执行DoPendingFunctors时执行
     */
    void QueueInLoop(Functor functor);
    /**
     * @brief 在IO线程中执行回调操作
     * 
     * @param functor 待执行的操作
     * 
     * 在IO线程中执行回调操作，目的是可以最大程度的避免一些线程的同步问题。\n
     * 如果调用方在本Loop的IO线程，则同步执行该操作，否则异步执行（调用QueueInLoop）
     */
    void RunInLoop(Functor functor);
    /**
     * @brief 在IO线程中指定时间点同步执行回调操作
     * 
     * @param when 时间点
     * @param doWhat 待执行的操作
     * @return 定时器的id，用于取消定时器
     */
    Timer::Id RunAt(Timer::TimePoint when, TimerCallback doWhat);
    /**
     * @brief 在IO线程中当前时间点后一段时间间隔同步执行回调操作
     * 
     * @param delay 时间间隔
     * @param doWhat 待执行的操作
     * @return 定时器的id，用于取消定时器
     */
    Timer::Id RunAfter(Timer::Duration delay, TimerCallback doWhat);
    /**
     * @brief 在IO线程中当前时间点后一段时间间隔开始每隔一段时间间隔同步执行回调操作
     * 
     * @param interval 时间间隔
     * @param doWhat 待执行的操作
     * @return 定时器的id，用于取消定时器
     */
    Timer::Id RunEvery(Timer::Duration interval, TimerCallback doWhat);
    /**
     * @brief 在IO线程中当前时间点后一段时间间隔开始每隔一段时间间隔同步执行回调操作
     * 
     * @param timerId 定时器Id，添加定时器时获取
     */
    void CancelTimer(Timer::Id timerId);

private:
    /**
     * @brief 处理持有的eventfd的读事件
     * 
     * 处理读事件，将WakeUp时写入的一个uint_64的数据读出
     */
    void HandleRead() const;
    /**
     * @brief 唤醒阻塞在epoll_wait处的IO线程
     * 
     * 通过向持有的eventfd写入一个uint_64的数据，使epoll_wait（位于Loop方法中的Poll方法）返回\n
     * 返回后可以执行DoPendingFunctors
     */
    void WakeUp() const;
    /**
     * @brief 处理一些附加的IO操作
     * 
     * 处理一些附加的IO操作，例如改变epoll状态，socket状态等
     */
    void DoPendingFunctors();
    /**
     * @brief 判断当前调用线程是否使本EventLoop的IO线程
     */
    bool IsInLoopThread() const noexcept { return std::this_thread::get_id() == threadId_; }

    std::atomic<bool> quit_;
    bool callingPendingFunctors_; /**< 该变量的存取都在同一IO线程，没有必要是atomic的 */
    std::thread::id threadId_;
    EpollPoller poller_;
    TimerQueue timerQueue_;

    FileDescriptor wakeupfd_;
    Channel wakeupChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};

#endif