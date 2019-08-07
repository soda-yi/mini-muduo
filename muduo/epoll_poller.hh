#ifndef EPOLL_POLLER_H
#define EPOLL_POLLER_H

#include <sys/epoll.h>

#include <vector>

#include "channel.hh"

class EpollPoller
{
public:
    using ChannelList = std::vector<Channel *>;

    EpollPoller() noexcept;
    EpollPoller(const EpollPoller &) = delete;
    EpollPoller &operator=(const EpollPoller &) = delete;
    // FIXME 移动构造不能是默认的
    EpollPoller(EpollPoller &&) = default;
    EpollPoller &operator=(EpollPoller &&) = default;
    ~EpollPoller();

    void Poll(ChannelList *activeChannels);
    void UpdateChannel(Channel *channel) const noexcept;
    void RemoveChannel(Channel *channel) const noexcept;

private:
    using EventList = std::vector<struct epoll_event>;
    static constexpr int kMaxEvents = 500;

    int epollfd_ = -1;
    EventList events_;
};

#endif