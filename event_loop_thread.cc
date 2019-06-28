#include "event_loop_thread.hh"

#include "event_loop.hh"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::ThreadFunc, this)),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->Quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::StartLoop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return loop_ != nullptr; });
    }

    return loop_;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;

    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }

    loop.Loop();
    loop_ = nullptr;
}