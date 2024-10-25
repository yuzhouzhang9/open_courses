#include "threadpool.h"  // 只包含头文件，不要重复定义类

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { this->worker(); });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();  // 唤醒所有线程

    for (std::thread& worker : workers) {
        worker.join();  // 等待所有线程结束
    }
}

void ThreadPool::enqueue(Task task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));  // 将任务添加到队列
    }
    condition.notify_one();  // 唤醒一个线程
}

void ThreadPool::worker() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) return;

            task = std::move(tasks.front());
            tasks.pop();
        }
        task();  // 执行任务
    }
}
