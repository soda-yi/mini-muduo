#include "event_loop_thread_pool.hh"

#include <cstdio>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThread)
    : baseLoop_{baseLoop},
      started_{false},
      next_{0}
{
    started_ = true;

    for (int i = 0; i < numThread; ++i) {
        auto t = std::make_unique<EventLoopThread>();
        loops_.push_back(t->StartLoop());
        threads_.push_back(std::move(t));
    }
}

EventLoop *EventLoopThreadPool::GetNextLoop()
{
    EventLoop *loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}