#ifndef CHANNEL_HH
#define CHANNEL_HH

#include <functional>

class EventLoop;

class Channel
{
public:
    using EventCallback = std::function<void(int)>;

    Channel(EventLoop *loop, int sockfd);
    ~Channel();

    void SetCallback(EventCallback callback) { callback_ = std::move(callback); }
    void SetRevents(int revent) { revents_ = revent; }
    int GetSockfd() const { return sockfd_; }
    int GetEvents() const { return events_; }
    void EnableReading();
    void HandleEvent();

private:
    void Update();
    EventLoop *loop_;
    int sockfd_;
    int events_ = 0;  // 关注的事件
    int revents_ = 0; // 发生的事件
    EventCallback callback_ = nullptr;
};

#endif