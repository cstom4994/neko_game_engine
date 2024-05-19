

#include "thread_pool.h"

#include "sandbox/simulation.h"

chunk_job::chunk_job(sandbox_chunk* chunk) { chunk_ = chunk; }

void chunk_thread_pool::Start() {
    const uint32_t num_threads = std::thread::hardware_concurrency();  // Max # of threads the system supports
    threads.resize(num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        threads.at(i) = std::thread(&chunk_thread_pool::thread_loop, this);
    }
}

void chunk_thread_pool::thread_loop() {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    while (true) {
        chunk_job* job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] { return !jobs.empty() || should_terminate; });
            if (should_terminate) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job->chunk_->Update();
    }
}

void chunk_thread_pool::QueueJob(chunk_job& job) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(&job);
    }
    mutex_condition.notify_one();
}

bool chunk_thread_pool::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = jobs.empty();
    }
    return poolbusy;
}

void chunk_thread_pool::Stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}
