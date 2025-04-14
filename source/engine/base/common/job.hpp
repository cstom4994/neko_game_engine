#pragma once

#include "engine/base/common/base.hpp"

#include <functional>

struct JobDispatchArgs {
    u32 jobIndex;    // 作业索引
    u32 groupIndex;  // 作业组索引
};

namespace Neko {

// 固定大小的简单线程安全环形缓冲区
template <typename T, size_t capacity>
class ThreadSafeRingBuffer {
public:
    // 如果有空闲空间 将项目推送到末尾
    // 成功返回true 空间不足返回false
    inline bool push_back(const T& item) {
        bool result = false;
        lock.lock();
        size_t next = (head + 1) % capacity;
        if (next != tail) {
            data[head] = item;
            head = next;
            result = true;
        }
        lock.unlock();
        return result;
    }

    // 如果有项目 则获取一个项目
    // 成功返回true 没有项目返回false
    inline bool pop_front(T& item) {
        bool result = false;
        lock.lock();
        if (tail != head) {
            item = data[tail];
            tail = (tail + 1) % capacity;
            result = true;
        }
        lock.unlock();
        return result;
    }

private:
    T data[capacity];
    size_t head = 0;
    size_t tail = 0;
    std::mutex lock;
};

class Job {
public:
    static void init();

    // 添加一个异步执行的作业 任何空闲线程都会执行这个作业
    static void Execute(const std::function<void()>& job);

    // 将一个作业分成多个并行执行的子作业
    // jobCount: 为此任务生成的作业数量
    // groupSize: 每个线程执行的作业数量 组内的作业串行执行 对于小作业 增加此值可能更有效
    // func: 接收JobDispatchArgs作为参数的函数
    static void Dispatch(u32 jobCount, u32 groupSize, const std::function<void(JobDispatchArgs)>& job);

    static bool IsBusy();  // 检查当前是否有任何线程在工作

    static void Wait();  // 等待直到所有线程空闲
};

}  // namespace Neko