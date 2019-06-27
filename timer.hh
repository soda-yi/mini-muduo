#ifndef TIMER_HH
#define TIMER_HH

#include <atomic>

#include "callback.hh"
#include "timestamp.hh"

class Timer
{
public:
    Timer(TimerCallback cb, Timestamp expiration, double interval)
        : callback_(cb),
          expiration_(expiration),
          interval_(interval),
          repeat_(interval > 0.0),
          sequence_(kTimerCount++)
    {
    }
    Timestamp GetExpiration() const { return expiration_; }
    int64_t GetSequence() const { return sequence_; }
    void Run() const { callback_(); }
    bool IsRepeat() const { return repeat_; }

    void Restart(Timestamp now);

private:
    TimerCallback callback_;
    Timestamp expiration_;
    double interval_; //seconds
    bool repeat_;
    int64_t sequence_;

    static std::atomic<int64_t> kTimerCount;
};

#endif