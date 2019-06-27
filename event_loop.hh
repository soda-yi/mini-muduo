#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include <memory>

class Channel;
class EpollPoller;

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();
    void Loop();
    void UpdateChannel(Channel *channel);

private:
    bool quit_;
    std::unique_ptr<EpollPoller> poller_;
};

#endif