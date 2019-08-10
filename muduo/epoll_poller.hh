#ifndef EPOLL_POLLER_H
#define EPOLL_POLLER_H

#include <sys/epoll.h>

#include <vector>

#include "channel.hh"
#include "file_descriptor.hh"

/**
 * @brief Epoll操作的封装类，Initiation Dispatcher的实现
 * 
 * 主要操作对象是Channel
 */
class EpollPoller
{
public:
    using ChannelList = std::vector<Channel *>;

    EpollPoller();
    EpollPoller(const EpollPoller &) = delete;
    EpollPoller &operator=(const EpollPoller &) = delete;
    EpollPoller(EpollPoller &&) = default;
    EpollPoller &operator=(EpollPoller &&) = default;

    /**
     * @brief 等待所有发生事件的Channel，将它们的指针添加到activeChannels中
     * 
     * @param[out] activeChannels 方法执行结束后，将有事件发生的Channel添加到此参数中，\n
     * 若为nullptr则行为未定义
     * 
     * Initiation Dispatcher中handle_events的实现，\n
     * 同时也是对epoll_wait操作（Synchronous Event Demultiplexer）的封装
     */
    void Poll(ChannelList *activeChannels);
    /**
     * @brief 添加Channel至epoll中，register_handle的实现
     * 
     * @param[in] channel 需要添加的channel
     */
    void AddChannel(const Channel &channel) const;
    /**
     * @brief 更新Channel关心的事件，将变更反映至epoll中
     * 
     * @param[in] channel 需要更改的channel
     */
    void UpdateChannel(const Channel &channel) const;
    /**
     * @brief 将Channel从epoll中移除
     * 
     * @param[in] channel 需要移除的channel，remove_handle的实现
     */
    void RemoveChannel(const Channel &channel) const;

private:
    using EventList = std::vector<struct epoll_event>;
    static constexpr int kMaxEvents = 500;

    FileDescriptor epollfd_;
    EventList events_;
};

#endif