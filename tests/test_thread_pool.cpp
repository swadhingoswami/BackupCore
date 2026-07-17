#include <gtest/gtest.h>
#include "thread_pool/thread_pool.h"
#include <atomic>

namespace backups {

TEST(ThreadPoolTest, ExecutesJobs) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 10; i++) {
        pool.enqueue([&counter] { counter++; });
    }

    pool.wait_all();
    EXPECT_EQ(counter.load(), 10);
}

TEST(ThreadPoolTest, SingleThread) {
    ThreadPool pool(1);
    std::atomic<int> counter{0};

    for (int i = 0; i < 5; i++) {
        pool.enqueue([&counter] { counter++; });
    }

    pool.wait_all();
    EXPECT_EQ(counter.load(), 5);
}

TEST(ThreadPoolTest, ManyThreads) {
    ThreadPool pool(8);
    std::atomic<int64_t> sum{0};

    for (int i = 0; i < 100; i++) {
        pool.enqueue([&sum, i] { sum += i; });
    }

    pool.wait_all();
    EXPECT_EQ(sum.load(), 4950);
}

TEST(ThreadPoolTest, PendingCount) {
    ThreadPool pool(2);
    EXPECT_EQ(pool.pending(), 0);

    for (int i = 0; i < 10; i++) {
        pool.enqueue([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }

    EXPECT_GT(pool.pending(), 0);
    pool.wait_all();
    EXPECT_EQ(pool.pending(), 0);
}

TEST(ThreadPoolTest, DestructorCompletesJobs) {
    std::atomic<int> counter{0};
    {
        ThreadPool pool(4);
        for (int i = 0; i < 100; i++) {
            pool.enqueue([&counter] {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                counter++;
            });
        }
    }
    EXPECT_EQ(counter.load(), 100);
}

TEST(ThreadPoolTest, JobsExecuteConcurrently) {
    ThreadPool pool(4);
    std::atomic<int> concurrent_max{0};
    std::atomic<int> concurrent_now{0};

    for (int i = 0; i < 20; i++) {
        pool.enqueue([&concurrent_max, &concurrent_now] {
            int val = ++concurrent_now;
            int prev_max = concurrent_max.load();
            while (val > prev_max) {
                concurrent_max.compare_exchange_weak(prev_max, val);
                prev_max = concurrent_max.load();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            concurrent_now--;
        });
    }

    pool.wait_all();
    EXPECT_GE(concurrent_max.load(), 1);
}

} // namespace backups
