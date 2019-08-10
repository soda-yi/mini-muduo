#ifndef THREAD_POOL_HH
#define THREAD_POOL_HH

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
private:
    using Task = std::function<void()>;
    static constexpr int kMaxThreadNum = 16;

public:
    ThreadPool(int size);
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ~ThreadPool();

    template <class F, class... Args>
    auto Commit(F &&f, Args &&... args)
    {
        if (!run_) {
            throw std::runtime_error("commit on ThreadPool is stopped.");
        }

        using RetType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> lock{mutex_};
            tasks_.emplace([task]() { (*task)(); });
        }
        cond_.notify_one();

        return future;
    }
    //空闲线程数量
    int IdlCount() { return idleThrNum_; }
    //线程数量
    int ThrCount() { return pool_.size(); }

private:
    std::queue<Task> tasks_;
    std::vector<std::thread> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> run_{true};
    std::atomic<int> idleThrNum_{0};
};

#endif