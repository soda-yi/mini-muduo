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
    auto Commit(F &&f, Args &&... args) -> std::future<decltype(f(args...))>
    {
        if (!run_) {
            throw std::runtime_error("commit on ThreadPool is stopped.");
        }

        using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)); // 把函数入口及参数,打包(绑定)
        std::future<RetType> future = task->get_future();
        {                                             // 添加任务到队列
            std::lock_guard<std::mutex> lock{mutex_}; //对当前块的语句加锁  lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候 unlock()
            tasks_.emplace([task]() { (*task)(); });  // push(Task{...}) 放到队列后面
        }
        cond_.notify_one(); // 唤醒一个线程执行

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