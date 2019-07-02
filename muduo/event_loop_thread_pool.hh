#ifndef EVENT_LOOP_THREAD_POOL_HH
#define EVENT_LOOP_THREAD_POOL_HH

#include <functional>
#include <memory>
#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    EventLoopThreadPool(EventLoop *baseLoop);
    ~EventLoopThreadPool();
    void SetThreadNum(int numThreads) { numThreads_ = numThreads; }
    void Start(const ThreadInitCallback &cb = ThreadInitCallback());

    // valid after calling start()
    /// round-robin
    EventLoop *GetNextLoop();

private:
    EventLoop *baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};

#endif