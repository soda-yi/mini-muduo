#include "thread_pool.hh"

ThreadPool::ThreadPool(int size)
{
    for (int i = 0; i != size && i != kMaxThreadNum; ++i) {
        pool_.emplace_back([this] {
            while (run_) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cond_.wait(lock, [this] { return !run_ || !tasks_.empty(); });
                    if (!run_ && tasks_.empty()) {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                --idleThrNum_;
                task();
                ++idleThrNum_;
            }
        });
        ++idleThrNum_;
    }
}

ThreadPool::~ThreadPool()
{
    run_ = false;
    cond_.notify_all();
    for (auto &thread : pool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}