#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t numThreads) : stop(false) 
{
    for (size_t i = 0; i < numThreads; ++i) 
    {
        workers.emplace_back([this]() 
        {
            while (true) 
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex);
                    this->condition.wait(lock, [this]() {
                        return this->stop || not this->tasks.empty();
                    });

                    if (this->stop && this->tasks.empty())
                        return;

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() 
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) 
    {
        worker.join();
    }
}