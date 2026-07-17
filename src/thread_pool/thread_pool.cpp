#include "thread_pool.h"

namespace backups {

ThreadPool::ThreadPool(size_t thread_count) {
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; i++) {
        workers_.emplace_back(&ThreadPool::worker_loop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(Job job) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        jobs_.push(std::move(job));
    }
    cv_.notify_one();
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] {
        return jobs_.empty() && active_jobs_ == 0;
    });
}

size_t ThreadPool::pending() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return jobs_.size();
}

void ThreadPool::worker_loop() {
    while (true) {
        std::optional<Job> job;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {
                return stop_ || !jobs_.empty();
            });

            if (stop_ && jobs_.empty()) {
                return;
            }

            job = std::move(jobs_.front());
            jobs_.pop();
            active_jobs_++;
        }

        if (job) {
            (*job)();
            active_jobs_--;
            cv_.notify_all();
        }
    }
}

} // namespace backups
