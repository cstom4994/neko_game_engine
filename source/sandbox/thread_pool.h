
#ifndef GAME_THREAD_POOL_H
#define GAME_THREAD_POOL_H

#include <cstdio>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "sandbox/magic_pixel.h"

class sandbox_chunk;

class chunk_job {
public:
    sandbox_chunk *chunk_;
    chunk_job() {}
    chunk_job(sandbox_chunk *chunk);
};

class chunk_thread_pool {
public:
    void Start();
    void QueueJob(chunk_job &job);
    void Stop();
    bool busy();

private:
    void thread_loop();

    bool should_terminate = false;            // 线程停止
    std::mutex queue_mutex;                   // 防止数据争用到 Job 队列
    std::condition_variable mutex_condition;  // 允许线程等待新 Job 或终止
    std::vector<std::thread> threads;
    std::queue<chunk_job *> jobs;
};

#endif