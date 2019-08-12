#include "timer.hh"

std::atomic<int64_t> Timer::Id::kTimerCount{0};

void Timer::Restart(TimePoint now)
{
    if (repeat_) {
        when_ = now + interval_;
    } else {
        when_ = TimePoint{};
    }
}