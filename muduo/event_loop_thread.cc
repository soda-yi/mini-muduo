#include "event_loop_thread.hh"

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_) {
        loop_->Quit();
    }
    if (thread_) {
        thread_->join();
    }
}

EventLoop *EventLoopThread::StartLoop()
{
    thread_ = std::make_unique<std::thread>([this] { ThreadFunc(); });
    {
        std::unique_lock lock{mutex_};
        cond_.wait(lock, [this] { return loop_ != nullptr; });
    }

    return loop_;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;

    {
        std::unique_lock lock{mutex_};
        loop_ = &loop;
        cond_.notify_all();
    }

    loop.Loop();
    loop_ = nullptr;
}