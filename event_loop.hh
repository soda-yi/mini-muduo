#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include <functional>
#include <memory>
#include <vector>

class Channel;
class EpollPoller;

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    void Loop();
    void UpdateChannel(Channel *channel);
    void QueueLoop(Functor functor);
    void HandleRead();

private:
    void WakeUp();
    void DoPendingFunctors();

    bool quit_;
    std::unique_ptr<EpollPoller> poller_;
    int wakeupfd_;
    std::unique_ptr<Channel> wakeupChannel_;
    std::vector<Functor> pendingFunctors_;
};

#endif