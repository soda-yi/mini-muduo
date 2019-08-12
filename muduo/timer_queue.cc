#include "timer_queue.hh"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "event_loop.hh"

using std::cout;
using std::endl;
using std::vector;

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_{loop},
      timerfd_{::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)},
      timerfdChannel_{loop_, timerfd_.GetFd()}
{
    timerfdChannel_.SetReadCallback([this] { HandleRead(); });
    timerfdChannel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.DisableAll();
    timerfdChannel_.Remove();
}

void TimerQueue::doAddTimer(TimerPtr timer)
{
    if (timers_.empty() || timers_.top()->GetExpiration() < timer->GetExpiration()) {
        timerfd_.SetTime(timer->GetExpiration());
    }
    timers_.push(timer);
}

void TimerQueue::doCancelTimer(TimerId timerId) noexcept
{
    timerId.timer_->Cancel();
}

Timer::Id TimerQueue::AddTimer(Timer::TimePoint when,
                               Timer::Duration interval,
                               TimerCallback doWhat)
{
    TimerPtr timer = std::make_shared<Timer>(when, interval, doWhat);
    loop_->QueueInLoop([this, timer] { doAddTimer(timer); });
    return timer->GetId();
}

void TimerQueue::CancelTimer(Timer::Id timerId)
{
    loop_->QueueInLoop([this, timerId] { doCancelTimer(timerId); });
}

void TimerQueue::HandleRead()
{
    TimePoint now{std::chrono::system_clock::now()};
    uint64_t howmany;
    ssize_t n = timerfd_.Read(&howmany, sizeof(howmany));
    if (n != sizeof(howmany)) {
        cout << "Timer::readTimerfd() error " << endl;
    }

    vector<TimerPtr> expired = GetExpired(now);
    for (auto &e : expired) {
        e->Run();
    }
    ResetExpired(expired, now);
    SetNextExpired();
}

std::vector<TimerQueue::TimerPtr> TimerQueue::GetExpired(const TimePoint &now)
{
    std::vector<TimerPtr> expired;

    for (; !timers_.empty();) {
        auto &timer = timers_.top();
        auto &&expiration = timer->GetExpiration();
        if (timer->IsCancelled()) {
            timers_.pop();
        } else if (expiration < now) {
            expired.push_back(timer);
            timers_.pop();
        } else {
            break;
        }
    }

    return expired;
}

void TimerQueue::ResetExpired(const vector<TimerPtr> &expired, const TimePoint &now)
{
    for (auto &timer : expired) {
        if (timer->IsRepeat()) {
            timer->Restart(now);
            timers_.push(timer);
        }
    }
}

void TimerQueue::SetNextExpired()
{
    if (!timers_.empty()) {
        const auto &nextExpiration = timers_.top()->GetExpiration();
        timerfd_.SetTime(nextExpiration);
    }
}