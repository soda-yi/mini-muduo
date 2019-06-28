#ifndef EVENT_LOOP_THREAD_HH
#define EVENT_LOOP_THREAD_HH

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class EventLoop;

class EventLoopThread
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());
    ~EventLoopThread();
    EventLoop *StartLoop();

private:
    void ThreadFunc();

    EventLoop *loop_;
    bool exiting_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};

#endif