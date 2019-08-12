#ifndef TIMER_HH
#define TIMER_HH

#include <atomic>
#include <chrono>

#include "callbacks.hh"

/**
 * @brief 定时器类
 * 
 * 虽然称为定时器类，但并不具备定时的能力，只有置于TimerQueue中才具备定时能力
 */
class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    /**
     * @brief 定时器的Id类
     * 
     * 标识一个定时器的标识，用于AddTimer后CancelTimer时使用
     */
    class Id
    {
        friend class Timer;
        friend class TimerQueue;

    public:
        Id() = default;
        Id(const Id &) = default;
        Id &operator=(const Id &) = default;
        Id(Id &&) = default;
        Id &operator=(Id &&) = default;

    private:
        Id(Timer *timer)
            : timer_{timer},
              sequence_{kTimerCount++}
        {
        }

        Timer *timer_;
        int64_t sequence_;

        static std::atomic<int64_t> kTimerCount;
    };

    /**
     * @brief 构造一个定时器
     * 
     * @param when 某一时间点
     * @param interval 时间间隔
     * @param doWhat 时间过期后调用的回调
     * 
     * 若时间间隔interval>0，则定时器会不断重复执行，第一次过期点在when处，之后每隔interval后过期一次\n
     * 否则，定时器只执行一次，过期点在when处
     */
    Timer(Timer::TimePoint when,
          Timer::Duration interval,
          TimerCallback doWhat)
        : id_{this},
          when_{std::move(when)},
          interval_{std::chrono::duration_cast<Duration>(std::move(interval))},
          repeat_{std::chrono::nanoseconds{0} < interval_},
          doWhat_{std::move(doWhat)}
    {
    }

    /**
     * @brief 获取过期时间
     * 
     * @return TimePoint 过期时间
     */
    TimePoint GetExpiration() const noexcept { return when_; }
    Id GetId() const noexcept { return id_; }

    /**
     * @brief 执行过期时间到后应该执行的回调函数
     */
    void Run() const
    {
        if (doWhat_) {
            doWhat_();
        }
    }
    bool IsRepeat() const noexcept { return repeat_; }
    bool IsCancelled() const noexcept { return cancelled_; }

    /**
     * @brief 重启定时器
     * 
     * @param now 当前时间
     * 
     * 根据当前时间重新计算下次过期时间点，并重启计时器。只有当定时器是repeat的才有效
     */
    void Restart(TimePoint now);
    /**
     * @brief 取消一个已经启动的定时器
     */
    void Cancel() noexcept { cancelled_ = true; }

private:
    Id id_;
    TimePoint when_;
    Duration interval_;
    bool repeat_;
    bool cancelled_;
    TimerCallback doWhat_;
};

#endif