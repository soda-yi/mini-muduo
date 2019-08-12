#ifndef TIMERQUEUE_HH
#define TIMERQUEUE_HH

#include "callbacks.hh"
#include "channel.hh"
#include "file_descriptor.hh"
#include "timer.hh"

#include <memory>
#include <queue>
#include <vector>

class EventLoop;

/**
 * @brief 定时器队列
 * 
 * 真正意义上的定时器，想要启动、取消计时器要调用类内方法来操作
 */
class TimerQueue
{
public:
    using Clock = Timer::Clock;
    using TimePoint = Timer::TimePoint;
    using Duration = Timer::Duration;
    using TimerId = Timer::Id;

    TimerQueue(EventLoop *pLoop);
    TimerQueue(const TimerQueue &) = delete;
    TimerQueue &operator=(const TimerQueue &) = delete;
    TimerQueue(TimerQueue &&) = default;
    TimerQueue &operator=(TimerQueue &&) = default;
    ~TimerQueue();

    /**
     * @brief 添加一个定时器
     * 
     * @param when 某一时间点
     * @param interval 时间间隔
     * @param doWhat 时间过期后调用的回调
     * @return TimerId 添加的定时器的id
     * 
     * 若时间间隔interval>0，则定时器会不断重复执行，第一次过期点在when处，之后每隔interval后过期一次\n
     * 否则，定时器只执行一次，过期点在when处
     */
    TimerId AddTimer(TimePoint when,
                     Duration interval,
                     TimerCallback doWhat);
    /**
     * @brief 取消一个定时器
     * 
     * @param timerId 添加定时器时获取的id
     * 
     * @warning 若定时器已经过期，此时Cancel是未定义的（可能会指涉空悬指针）
     */
    void CancelTimer(TimerId timerId);

private:
    /**
     * @brief 用于比较两个定时器哪个更早
     * 
     * @tparam TimerPtr 定时器的指针
     */
    template <typename TimerPtr>
    struct TimerPtrLess {
        bool operator()(TimerPtr pt1, TimerPtr pt2) const
        {
            return pt2->GetExpiration() < pt1->GetExpiration();
        }
    };

    using TimerPtr = std::shared_ptr<Timer>;
    using TimerList = std::priority_queue<TimerPtr, std::vector<TimerPtr>, TimerPtrLess<TimerPtr>>;

    /**
     * @brief 处理持有的timerfd的读事件
     * 
     * 有读事件到来意味着定时器队列中最早的定时器过期，此时应该做的事：
     * - 找到所有过期的定时器，执行他们的回调
     * - 对于过期的且是repeat的定时器，重新计算后再次加入队列
     * - 从队列中找到最早的计时器，为其在内核中注册定时器
     */
    void HandleRead();

    void doAddTimer(TimerPtr timer);
    void doCancelTimer(TimerId timerId) noexcept;

    /**
     * @brief 获取所有过期的（在当前时间点之前）定时器
     * 
     * @param now 当前时间点
     * @return std::vector<TimerPtr> 所有过期的定时器
     */
    std::vector<TimerPtr> GetExpired(const TimePoint &now);
    /**
     * @brief 以当前时间点基准，重新计算过期且repeat的定时器并入列
     * 
     * @param expired 过期的定时器
     * @param now 当前时间点
     */
    void ResetExpired(const std::vector<TimerPtr> &expired, const TimePoint &now);
    /**
     * @brief 从队列中找到最早的计时器，为其在内核中注册定时器
     */
    void SetNextExpired();

    EventLoop *loop_;
    timerfds::TimerFd timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
};

#endif