
#include "engine/neko_engine.h"

#if 1

void __neko_platform_file_extension(char *buffer, usize buffer_sz, const_str file_path) { neko_util_get_file_extension(buffer, buffer_sz, file_path); }

// neko_string __neko_game_get_path(const neko_string &path) {
//
//     neko_platform_ctx *ctx = __ctx();
//
//     if (!ctx->gamepath) {
//         neko_assert(ctx->gamepath, "gamepath not detected");
//         return path;
//     }
//
//     neko_string get_path(ctx->gamepath);
//
//     switch (neko::hash(path.substr(0, 4))) {
//         case neko::hash("data"):
//             get_path.append(path);
//             break;
//         default:
//             break;
//     }
//
//     return get_path;
// }

neko_string __neko_platform_abbreviate_path(const neko_string &path, s32 max_lenght) {
    if (path.length() <= max_lenght) return path;  // 如果路径长度不超过最大长度，则无需缩略，直接返回原路径

    int filename_pos = path.find_last_of('\\');
    if (filename_pos == std::string::npos) return path;

    std::string fileName = path.substr(filename_pos + 1);
    int dirPathLength = max_lenght - fileName.length() - 3;  // 3 表示省略号的长度
    if (dirPathLength <= 0) return "..." + fileName;         // 如果文件名已经超过最大长度，则只显示文件名前面的省略号和文件名
    std::string dirPath = path.substr(0, dirPathLength);

    return dirPath + "..." + fileName;
}

neko_string __neko_game_get_path(const neko_string &path);
neko_string __neko_platform_abbreviate_path(const neko_string &path, s32 max_lenght = 30);

#define neko_file_path(x) (neko_engine_subsystem(platform))->get_path(x).c_str()
#define neko_abbreviate_path(x) (neko_engine_subsystem(platform))->abbreviate_path(x, 30).c_str()

static inline uint64_t neko_get_thread_id() {
#if defined(NEKO_PLATFORM_WIN)
    return (uint64_t)GetCurrentThreadId();
#elif defined(NEKO_PLATFORM_LINUX)
    return (uint64_t)syscall(SYS_gettid);
#elif defined(NEKO_PLATFORM_APPLE)
    return (mach_port_t)::pthread_mach_thread_np(pthread_self());
#else
#error "Unsupported platform!"
#endif
}

// Thread Local Storage(TLS)
// msvc: https://learn.microsoft.com/en-us/cpp/parallel/thread-local-storage-tls

#ifdef NEKO_PLATFORM_WIN

static inline u32 neko_tls_allocate() { return (u32)TlsAlloc(); }

static inline void neko_tls_set_value(u32 _handle, void *_value) { TlsSetValue(_handle, _value); }

static inline void *neko_tls_get_value(u32 _handle) { return TlsGetValue(_handle); }

static inline void neko_tls_free(u32 _handle) { TlsFree(_handle); }

#else

static inline pthread_key_t neko_tls_allocate() {
    pthread_key_t handle;
    pthread_key_create(&handle, NULL);
    return handle;
}

static inline void neko_tls_set_value(pthread_key_t _handle, void *_value) { pthread_setspecific(_handle, _value); }

static inline void *neko_tls_get_value(pthread_key_t _handle) { return pthread_getspecific(_handle); }

static inline void neko_tls_free(pthread_key_t _handle) { pthread_key_delete(_handle); }

#endif

#if defined(NEKO_PLATFORM_WIN)
typedef CRITICAL_SECTION neko_mutex;

static inline void neko_mutex_init(neko_mutex *_mutex) { InitializeCriticalSection(_mutex); }

static inline void neko_mutex_destroy(neko_mutex *_mutex) { DeleteCriticalSection(_mutex); }

static inline void neko_mutex_lock(neko_mutex *_mutex) { EnterCriticalSection(_mutex); }

static inline int neko_mutex_trylock(neko_mutex *_mutex) { return TryEnterCriticalSection(_mutex) ? 0 : 1; }

static inline void neko_mutex_unlock(neko_mutex *_mutex) { LeaveCriticalSection(_mutex); }

#elif defined(NEKO_PLATFORM_POSIX)
typedef pthread_mutex_t neko_mutex;

static inline void neko_mutex_init(neko_mutex *_mutex) { pthread_mutex_init(_mutex, NULL); }

static inline void neko_mutex_destroy(neko_mutex *_mutex) { pthread_mutex_destroy(_mutex); }

static inline void neko_mutex_lock(neko_mutex *_mutex) { pthread_mutex_lock(_mutex); }

static inline int neko_mutex_trylock(neko_mutex *_mutex) { return pthread_mutex_trylock(_mutex); }

static inline void neko_mutex_unlock(neko_mutex *_mutex) { pthread_mutex_unlock(_mutex); }

#else
#error "Unsupported platform!"
#endif

class neko_pthread_mutex {
    neko_mutex m_mutex;

    neko_pthread_mutex(const neko_pthread_mutex &_rhs);
    neko_pthread_mutex &operator=(const neko_pthread_mutex &_rhs);

public:
    inline neko_pthread_mutex() { neko_mutex_init(&m_mutex); }
    inline ~neko_pthread_mutex() { neko_mutex_destroy(&m_mutex); }
    inline void lock() { neko_mutex_lock(&m_mutex); }
    inline void unlock() { neko_mutex_unlock(&m_mutex); }
    inline bool tryLock() { return (neko_mutex_trylock(&m_mutex) == 0); }
};

class neko_scoped_mutex_locker {
    neko_pthread_mutex &m_mutex;

    neko_scoped_mutex_locker();
    neko_scoped_mutex_locker(const neko_scoped_mutex_locker &);
    neko_scoped_mutex_locker &operator=(const neko_scoped_mutex_locker &);

public:
    inline neko_scoped_mutex_locker(neko_pthread_mutex &_mutex) : m_mutex(_mutex) { m_mutex.lock(); }
    inline ~neko_scoped_mutex_locker() { m_mutex.unlock(); }
};

#include <functional>

namespace neko {

// Job系统详细设计可以见
// https://turanszkij.wordpress.com/2018/11/24/simple-job-system-using-standard-c/

struct neko_job_dispatch_args {
    u32 job_index;
    u32 group_index;
};

void neko_job_init();
void neko_job_execute(const std::function<void()> &job);
void neko_job_dispatch(u32 jobCount, u32 groupSize, const std::function<void(neko_job_dispatch_args)> &job);
bool neko_job_is_busy();
void neko_job_wait();

typedef struct {
    const char *function;  // name of function containing address of function.
    const char *file;      // file where symbol is defined, might not work on all platforms.
    unsigned int line;     // line in file where symbol is defined, might not work on all platforms.
    unsigned int offset;   // offset from start of function where call was made.
} neko_platform_callstack_symbol_t;

int neko_platform_callstack(int skip_frames, void **addresses, int num_addresses);
int neko_platform_callstack_symbols(void **addresses, neko_platform_callstack_symbol_t *out_syms, int num_addresses, char *memory, int mem_size);

void neko_platform_print_callstack();

}  // namespace neko

#endif

namespace neko {

// 固定大小的简单的线程安全环形缓冲区
template <typename T, size_t capacity>
class __neko_job_thread_safe_ring_buffer {
public:
    // 如果有可用空间则将项目推到末尾 若空间不足返回 false
    inline bool push_back(const T &item) {
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

    // 如果有则获取一个项目 若没有项目则返回 false
    inline bool pop_front(T &item) {
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
    std::mutex lock;  // 这比这里的自旋锁效果更好
};

neko_global u32 num_threads = 0;                                                      // 工作线程数 将在neko_job_init函数中初始化
neko_global __neko_job_thread_safe_ring_buffer<std::function<void()>, 256> job_pool;  // 一个线程安全队列 用于将挂起的作业放在末尾 工作线程可以从一开始就抢到一份工作
neko_global std::condition_variable wake_condition;                                   // 与下面的wakeMutex结合使用 工作线程在没有作业时只是休眠 主线程可以唤醒它们
neko_global std::mutex wake_mutex;                                                    // 与上面的wakeCondition结合使用
neko_global u64 current_label = 0;                                                    // 跟踪主线程的执行状态
neko_global std::atomic<u64> finished_label;                                          // 跟踪后台工作线程的执行状态

void neko_job_init() {
    // 将worker执行状态初始化为0
    finished_label.store(0);

    // 检索该系统中的硬件线程数
    auto numCores = std::thread::hardware_concurrency();

    // 计算我们想要的实际工作线程数
    num_threads = std::max(1u, numCores);

    // 创建所有工作线程 同时立即启动它们
    for (u32 threadID = 0; threadID < num_threads; ++threadID) {
        std::thread worker([] {
            std::function<void()> job;  // 线程的当前作业 在启动时它是空的

            // 这是工作线程将执行的无限循环
            while (true) {
                if (job_pool.pop_front(job))  // 尝试从 job_pool 队列中获取作业
                {
                    job();                        // 如果有job久执行它
                    finished_label.fetch_add(1);  // 更新job标签状态
                } else {
                    std::unique_lock<std::mutex> lock(wake_mutex);  // 没有job让线程休眠
                    wake_condition.wait(lock);
                }
            }
        });

#ifdef NEKO_PLATFORM_WIN
        // 进行 Windows 特定的线程设置
        HANDLE handle = (HANDLE)worker.native_handle();

        // 将每个线程放到专用核心上
        DWORD_PTR affinityMask = 1ull << threadID;
        DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
        neko_assert(affinity_result > 0);

        // 提升线程优先级
        BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
        neko_assert(priority_result != 0);

        // 命名线程
        std::wstringstream wss;
        wss << "neko_engine_job_" << threadID;
        HRESULT hr = SetThreadDescription(handle, wss.str().c_str());
        neko_assert(SUCCEEDED(hr));
#endif

        worker.detach();  // 忘记这个线程 让它在我们上面创建的无限循环中完成job
    }
}

// 当主线程等待某些东西时 这个辅助函数不会让系统陷入死锁
inline void poll() {
    wake_condition.notify_one();  // 唤醒一个工作线程
    std::this_thread::yield();    // 允许重新安排该线程
}

void neko_job_execute(const std::function<void()> &job) {
    // 主线程标签状态更新
    current_label += 1;

    // 尝试推送新job 直到推送成功
    while (!job_pool.push_back(job)) {
        poll();
    }

    wake_condition.notify_one();  // 唤醒一个线程
}

bool neko_job_is_busy() {
    // 每当worker没有到达主线程标签时 就表明有一些worker还活着
    return finished_label.load() < current_label;
}

void neko_job_wait() {
    while (neko_job_is_busy()) {
        poll();
    }
}

void neko_job_dispatch(u32 jobCount, u32 groupSize, const std::function<void(neko_job_dispatch_args)> &job) {
    if (jobCount == 0 || groupSize == 0) {
        return;
    }

    // 计算要分派的作业组数量
    const u32 group_count = (jobCount + groupSize - 1) / groupSize;

    // 主线程标签状态更新
    current_label += group_count;

    for (u32 groupIndex = 0; groupIndex < group_count; ++groupIndex) {
        // 对于每个组 生成一个 real job
        const auto &jobGroup = [jobCount, groupSize, job, groupIndex]() {
            // 计算当前组在job中的偏移量
            const u32 groupJobOffset = groupIndex * groupSize;
            const u32 groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

            neko_job_dispatch_args args;
            args.group_index = groupIndex;

            // 在组内循环遍历所有job索引并为每个索引执行job
            for (u32 i = groupJobOffset; i < groupJobEnd; ++i) {
                args.job_index = i;
                job(args);
            }
        };

        // 尝试推送新job直到推送成功
        while (!job_pool.push_back(jobGroup)) {
            poll();
        }

        wake_condition.notify_one();  // 唤醒一个线程
    }
}
}  // namespace neko