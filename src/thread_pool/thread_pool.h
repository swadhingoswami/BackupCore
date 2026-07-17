#pragma once

#include <functional>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>

namespace backups {

class ThreadPool {
public:
    using Job = std::function<void()>;

    explicit ThreadPool(size_t thread_count);
    ~ThreadPool();

    void enqueue(Job job);

    void wait_all();
    size_t pending() const;

private:
    std::vector<std::thread> workers_;
    std::queue<Job> jobs_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<size_t> active_jobs_{0};
    bool stop_ = false;

    void worker_loop();
};

} // namespace backups
