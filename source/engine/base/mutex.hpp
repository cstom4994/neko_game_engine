#pragma once

#include "engine/base/base.hpp"

using Mutex = std::mutex;
using SharedMutex = std::shared_mutex;
using Cond = std::condition_variable;

template <typename T>
using LockGuard = std::unique_lock<T>;

class RWLock {
public:
    inline void make() {}
    inline void trash() {}
    inline void shared_lock() { mtx.lock_shared(); }
    inline void shared_unlock() { mtx.unlock_shared(); }
    inline void unique_lock() { mtx.lock(); }
    inline void unique_unlock() { mtx.unlock(); }

private:
    SharedMutex mtx;
};

class Thread {
public:
    using ThreadProc = void (*)(void *);

    inline void make(ThreadProc fn, void *udata) { thread = std::thread(fn, udata); }

    inline void join() {
        if (thread.joinable()) thread.join();
    }

private:
    std::thread thread;
};

uint64_t this_thread_id();