#include "timer_queue.hh"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "event_loop.hh"
#include "timer.hh"
#include "timer_id.hh"

using std::cout;
using std::endl;
using std::vector;

namespace
{

void ReadTimerfd(int timerfd)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    if (n != sizeof(howmany)) {
        cout << "Timer::readTimerfd() error " << endl;
    }
}

struct timespec HowMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.GetMicroSecondsSinceEpoch() - Timestamp::Now().GetMicroSecondsSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void ResetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof(newValue));
    bzero(&oldValue, sizeof(oldValue));
    newValue.it_value = HowMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        cout << "timerfd_settime error" << endl;
    }
}

} // namespace

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      timerfdChannel_(loop_, timerfd_)
{
    if (timerfd_ < 0) {
        cout << "failed in timerfd_create" << endl;
    }
    timerfdChannel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
    timerfdChannel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
}

void TimerQueue::doAddTimer(Timer *timer)
{
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        ResetTimerfd(timerfd_, timer->GetExpiration());
    }
}

void TimerQueue::doCancelTimer(TimerId timerId)
{
    TimerList::iterator it;
    for (it = timers_.begin(); it != timers_.end(); ++it) {
        if (it->second == timerId.timer_) {
            timers_.erase(it);
            break;
        }
    }
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer *timer = new Timer(cb, when, interval); //Memory Leak !!!
    loop_->QueueInLoop(std::bind(&TimerQueue::doAddTimer, this, timer));
    return TimerId(timer, timer->GetSequence());
}

void TimerQueue::CancelTimer(TimerId timerId)
{
    loop_->QueueInLoop(std::bind(&TimerQueue::doCancelTimer, this, timerId));
}

void TimerQueue::HandleRead()
{
    Timestamp now(Timestamp::Now());
    ReadTimerfd(timerfd_);

    vector<Entry> expired = getExpired(now);
    for (const Entry &it : expired) {
        it.second->Run();
    }
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);
    copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    return expired;
}

void TimerQueue::reset(const vector<Entry> &expired, Timestamp now)
{
    vector<Entry>::const_iterator it;
    for (it = expired.begin(); it != expired.end(); ++it) {
        if (it->second->IsRepeat()) {
            it->second->Restart(now);
            insert(it->second);
        }
    }

    Timestamp nextExpire;
    if (!timers_.empty()) {
        nextExpire = timers_.begin()->second->GetExpiration();
    }
    if (nextExpire.IsValid()) {
        ResetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer *pTimer)
{
    bool earliestChanged = false;
    Timestamp when = pTimer->GetExpiration();
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }
    std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, pTimer));
    if (!(result.second)) {
        cout << "timers_.insert() error " << endl;
    }

    return earliestChanged;
}