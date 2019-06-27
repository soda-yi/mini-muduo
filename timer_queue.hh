#ifndef TIMERQUEUE_HH
#define TIMERQUEUE_HH

#include "callback.hh"
#include "channel.hh"
#include "timestamp.hh"

#include <set>
#include <vector>

class EventLoop;
class Timer;
class TimerId;

class TimerQueue
{
public:
    TimerQueue(EventLoop *pLoop);
    ~TimerQueue();
    TimerId AddTimer(TimerCallback cb,
                     Timestamp when,
                     double interval);
    void CancelTimer(TimerId timerId);

    void HandleRead();

private:
    typedef std::pair<Timestamp, Timer *> Entry;
    typedef std::set<Entry> TimerList;

    void doAddTimer(Timer *timer);
    void doCancelTimer(TimerId timerId);
    std::vector<TimerQueue::Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry> &expired, Timestamp now);
    bool insert(Timer *pItem);

    EventLoop *loop_;
    int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
};

#endif