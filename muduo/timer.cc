#include "timer.hh"

std::atomic<int64_t> Timer::kTimerCount(0);

void Timer::Restart(Timestamp now)
{
    if (repeat_) {
        expiration_ = AddTime(now, interval_);
    } else {
        expiration_ = Timestamp::Invalid();
    }
}