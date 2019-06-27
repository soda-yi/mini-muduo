#ifndef EPOLL_POLLER_H
#define EPOLL_POLLER_H

#include <sys/epoll.h>

#include <vector>

class Channel;

class EpollPoller
{
public:
    using ChannelList = std::vector<Channel *>;

    EpollPoller();
    ~EpollPoller();
    void Poll(ChannelList *activeChannels);
    void UpdateChannel(Channel *channel);

private:
    using EventList = std::vector<struct epoll_event>;
    static constexpr int kMaxEvents = 500;

    int epollfd_ = -1;
    EventList events_;
};

#endif