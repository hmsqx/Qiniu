#include "thread_pool.h"
#include <iostream>

// 全局线程池实例
std::unique_ptr<ThreadPool> g_threadPool = nullptr;

void initThreadPool(size_t threadCount) {
    if (g_threadPool == nullptr) {
        g_threadPool = std::make_unique<ThreadPool>(threadCount);
        std::cout << "线程池初始化完成，线程数: " << threadCount << std::endl;
    }
}

ThreadPool& getThreadPool() {
    if (g_threadPool == nullptr) {
        initThreadPool();
    }
    return *g_threadPool;
}
