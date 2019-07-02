#ifndef CHANNEL_HH
#define CHANNEL_HH

#include <functional>

class EventLoop;

class Channel
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int sockfd);
    ~Channel();

    void SetReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
    void SetWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void SetCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void SetRevents(int revent) { revents_ = revent; }
    void SetIndex(int index) { index_ = index; }
    int GetIndex() const { return index_; }
    int GetFd() const { return fd_; }
    int GetEvents() const { return events_; }

    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWriting();
    void DisableAll();
    /* 判断events_中是否有EPOLLOUT，即是否对写感兴趣 */
    bool IsWriting() const;
    bool IsReading() const;

    void HandleEvent();
    void Update();
    void Remove();

private:
    EventLoop *loop_;
    int fd_;
    int events_ = 0;  // 关注的事件
    int revents_ = 0; // 发生的事件
    int index_ = -1;
    EventCallback readCallback_ = nullptr;
    EventCallback writeCallback_ = nullptr;
    EventCallback closeCallback_ = nullptr;
};

#endif