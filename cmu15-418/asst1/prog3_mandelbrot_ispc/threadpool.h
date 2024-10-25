#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

// 任务类型，使用 std::function 封装
using Task = std::function<void()>;

// 线程池类声明
class ThreadPool {
public:
    ThreadPool(size_t numThreads);  // 构造函数
    ~ThreadPool();  // 析构函数

    void enqueue(Task task);  // 添加任务到队列

private:
    std::vector<std::thread> workers;  // 工作线程
    std::queue<Task> tasks;  // 任务队列

    std::mutex queueMutex;  // 队列互斥锁
    std::condition_variable condition;  // 条件变量
    std::atomic<bool> stop;  // 控制线程停止

    void worker();  // 每个线程的工作函数
};

#endif  // THREAD_POOL_H
