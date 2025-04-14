#include "job.hpp"

#include <algorithm>
#include <sstream>
#include <thread>

#include "engine/base/common/os.hpp"
#include "engine/base/common/mutex.hpp"

namespace Neko {

static u32 numThreads = 0;  // 工作线程数量 将在Initialize()中初始化

// 线程安全的作业池 容量256 用于存放待处理作业
static ThreadSafeRingBuffer<std::function<void()>, 256> job_pool;

static Cond wake_cond;    // 与wakeMutex配合使用的工作线程唤醒条件变量
static Mutex wake_mutex;  // 与wakeCondition配合使用的互斥锁

static u64 currentLabel = 0;            // 跟踪主线程执行状态
static std::atomic<u64> finishedLabel;  // 跟踪后台工作线程执行状态

void Job::init() {
    finishedLabel.store(0);

    numThreads = std::max(1u, std::thread::hardware_concurrency());

    // 创建并立即启动所有工作线程
    for (u32 threadID = 0; threadID < numThreads; ++threadID) {
        std::thread worker([] {
            std::function<void()> job;  // 当前线程要执行的作业

            // 工作线程的无限循环
            while (true) {
                if (job_pool.pop_front(job)) {   // 尝试从作业池获取作业
                    job();                       // 执行作业
                    finishedLabel.fetch_add(1);  // 更新工作线程状态
                } else {
                    // 没有作业 线程进入休眠
                    std::unique_lock<std::mutex> lock(wake_mutex);
                    wake_cond.wait(lock);
                }
            }
        });

#if defined(NEKO_IS_WIN32)
        // Windows特定的线程设置
        HANDLE handle = (HANDLE)worker.native_handle();

        // 将每个线程分配到专用核心
        DWORD_PTR affinityMask = 1ull << threadID;
        DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
        assert(affinity_result > 0);

        // 设置线程名称
        std::wstringstream wss;
        wss << "NekoEngineJob_" << threadID;
        HRESULT hr = SetThreadDescription(handle, wss.str().c_str());
        assert(SUCCEEDED(hr));
#endif

        worker.detach();  // 分离线程
    }
}

// 辅助函数 防止主线程等待时系统死锁
inline void poll() {
    wake_cond.notify_one();     // 唤醒一个工作线程
    std::this_thread::yield();  // 允许当前线程重新调度
}

void Job::Execute(const std::function<void()>& job) {
    // 更新主线程状态标签
    currentLabel += 1;

    // 尝试推送新作业直到成功
    while (!job_pool.push_back(job)) {
        poll();
    }

    wake_cond.notify_one();  // 唤醒一个线程
}

bool Job::IsBusy() { return finishedLabel.load() < currentLabel; }

void Job::Wait() {
    while (IsBusy()) {
        poll();
    }
}

void Job::Dispatch(u32 job_count, u32 group_size, const std::function<void(JobDispatchArgs)>& job) {
    if (job_count == 0 || group_size == 0) return;

    const u32 groupCount = (job_count + group_size - 1) / group_size;

    currentLabel += groupCount;  // 更新主线程状态标签

    for (u32 groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
        // 为每个组生成一个实际作业
        const auto& jobGroup = [job_count, group_size, job, groupIndex]() {
            const u32 group_job_offset = groupIndex * group_size;  // 计算当前组的作业偏移量
            const u32 group_job_end = std::min(group_job_offset + group_size, job_count);
            JobDispatchArgs args;
            args.groupIndex = groupIndex;
            for (u32 i = group_job_offset; i < group_job_end; ++i) {  // 在组内循环执行所有作业
                args.jobIndex = i;
                job(args);
            }
        };

        // 尝试推送新作业直到成功
        while (!job_pool.push_back(jobGroup)) {
            poll();
        }

        wake_cond.notify_one();  // 唤醒一个线程
    }
}

}  // namespace Neko
