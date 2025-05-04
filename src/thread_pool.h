#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    
    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        auto boundTask = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(boundTask);
        }
        condition.notify_one();
    }

    ~ThreadPool();
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};