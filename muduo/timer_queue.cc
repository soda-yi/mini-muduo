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
    if (timers_.empty() || timer->GetExpiration() < timers_.top()->GetExpiration()) {
        timerfd_.SetTime(timer->GetExpiration());
    }
    timers_.push(timer);
}

void TimerQueue::doCancelTimer(TimerId timerId) noexcept
{
    auto timer = timerId.timer_.lock();
    if (timer) {
        timer->Cancel();
        if (timerId == timers_.top()->GetId()) {
            timerfd_.CancelTime();
        }
    }
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
    TimePoint now{Clock::now()};
    uint64_t howmany;
    ssize_t n = timerfd_.Read(&howmany, sizeof(howmany));
    //cout << "timerfd " << timerfd_.GetFd() << ": some timer expired， expired time: " << howmany << endl;
    if (n != sizeof(howmany)) {
        cout << "Timer::readTimerfd() error " << endl;
    }

    PerTickBookkeeping(now);
}

void TimerQueue::PerTickBookkeeping(const TimePoint &now)
{
    for (; !timers_.empty();) {
        auto timer = timers_.top();
        auto &&expiration = timer->GetExpiration();
        if (timer->IsCancelled()) {
            timers_.pop();
        } else if (expiration < now) {
            timers_.pop();
            timer->Run();
            if (timer->IsRepeat()) {
                timer->Restart(now);
                timers_.push(timer);
            }
        } else {
            const auto &nextExpiration = timers_.top()->GetExpiration();
            timerfd_.SetTime(nextExpiration);
            break;
        }
    }
}