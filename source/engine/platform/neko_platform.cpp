#include "engine/platform/neko_platform.h"

#include <algorithm>           // std::max
#include <atomic>              // to use std::atomic<u64>
#include <condition_variable>  // to use std::condition_variable
#include <sstream>
#include <thread>

#include "engine/base/neko_engine.h"
#include "engine/common/neko_hash.h"
#include "engine/common/neko_mem.h"

/*============================
// Platform Window
============================*/

neko_resource_handle __neko_platform_create_window(const_str title, u32 width, u32 height) {
    struct neko_platform_i* platform = neko_engine_instance()->ctx.platform;
    void* win = platform->create_window_internal(title, width, height);
    neko_assert(win);
    neko_resource_handle handle = neko_slot_array_insert(platform->windows, win);
    neko_dyn_array_push(platform->active_window_handles, handle);
    return handle;
}

// 获取主窗口句柄 实际上返回是句柄槽的第一个窗口
neko_resource_handle __neko_platform_main_window() { return 0; }

/*============================
// Platform Input
============================*/

#define __input() (&neko_engine_instance()->ctx.platform->input)

void __neko_platform_update_input(neko_platform_input*) {
    neko_platform_input* input = __input();

    // Update all input and mouse keys from previous frame
    // Previous key presses
    neko_for_range_i(neko_keycode_count) { input->prev_key_map[i] = input->key_map[i]; }

    // Previous mouse button presses
    neko_for_range_i(neko_mouse_button_code_count) { input->mouse.prev_button_map[i] = input->mouse.button_map[i]; }

    input->mouse.prev_position = input->mouse.position;
    input->mouse.wheel = neko_vec2{0.0f, 0.0f};
    input->mouse.moved_this_frame = false;
}

bool __neko_platform_was_key_down(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    return (input->prev_key_map[code]);
}

bool __neko_platform_key_down(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    return (input->key_map[code]);
}

bool __neko_platform_key_pressed(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    if (__neko_platform_key_down(code) && !__neko_platform_was_key_down(code)) {
        return true;
    }
    return false;
}

bool __neko_platform_key_released(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    return (__neko_platform_was_key_down(code) && !__neko_platform_key_down(code));
}

bool __neko_platform_was_mouse_down(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    return (input->mouse.prev_button_map[code]);
}

void __neko_platform_press_mouse_button(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    if ((u32)code < (u32)neko_mouse_button_code_count) {
        input->mouse.button_map[code] = true;
    }
}

void __neko_platform_release_mouse_button(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    if ((u32)code < (u32)neko_mouse_button_code_count) {
        input->mouse.button_map[code] = false;
    }
}

bool __neko_platform_mouse_down(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    return (input->mouse.button_map[code]);
}

bool __neko_platform_mouse_pressed(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    if (__neko_platform_mouse_down(code) && !__neko_platform_was_mouse_down(code)) {
        return true;
    }
    return false;
}

bool __neko_platform_mouse_released(neko_platform_mouse_button_code code) {
    neko_platform_input* input = __input();
    return (__neko_platform_was_mouse_down(code) && !__neko_platform_mouse_down(code));
}

neko_vec2 __neko_platform_mouse_delta() {
    neko_platform_input* input = __input();

    if (input->mouse.prev_position.x < 0.0f || input->mouse.prev_position.y < 0.0f || input->mouse.position.x < 0.0f || input->mouse.position.y < 0.0f) {
        return neko_vec2{0.0f, 0.0f};
    }

    return neko_vec2{input->mouse.position.x - input->mouse.prev_position.x, input->mouse.position.y - input->mouse.prev_position.y};
}

neko_vec2 __neko_platform_mouse_position() {
    neko_platform_input* input = __input();

    return neko_vec2{input->mouse.position.x, input->mouse.position.y};
}

void __neko_platform_mouse_position_x_y(f32* x, f32* y) {
    neko_platform_input* input = __input();
    *x = input->mouse.position.x;
    *y = input->mouse.position.y;
}

neko_vec2 __neko_platform_mouse_wheel() {
    neko_platform_input* input = __input();

    return neko_vec2{input->mouse.wheel.x, input->mouse.wheel.y};
}

void __neko_platform_mouse_wheel_x_y(f32* x, f32* y) {
    neko_platform_input* input = __input();
    *x = input->mouse.wheel.x;
    *y = input->mouse.wheel.y;
}

void __neko_platform_press_key(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    if (code < neko_keycode_count) {
        input->key_map[code] = true;
    }
}

void __neko_platform_release_key(neko_platform_keycode code) {
    neko_platform_input* input = __input();
    if (code < neko_keycode_count) {
        input->key_map[code] = false;
    }
}

/*============================
// Platform File I/O
============================*/

#define __ctx() (&neko_engine_instance()->ctx.platform->ctx)

bool __neko_platform_file_exists(const_str file_path) {
    FILE* fp = fopen(file_path, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

u32 __neko_safe_truncate_u64(u64 value) {
    neko_assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}

#ifdef NEKO_PLATFORM_WIN
#else
#include <sys/stat.h>
#endif

s32 __neko_platform_file_size_in_bytes(const_str file_path) {
#ifdef NEKO_PLATFORM_WIN

    HANDLE hFile = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return -1;  // error condition, could call GetLastError to find out more

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        CloseHandle(hFile);
        return -1;  // error condition, could call GetLastError to find out more
    }

    CloseHandle(hFile);
    return __neko_safe_truncate_u64(size.QuadPart);

#else

    struct stat st;
    stat(file_path, &st);
    return (s32)st.st_size;

#endif
}

neko_result __neko_write_file_contents(const_str file_path, const_str mode, void* data, usize data_type_size, usize data_size) {
    FILE* fp = fopen(file_path, "wb");
    if (fp) {
        s32 ret = fwrite(data, data_type_size, data_size, fp);
        fclose(fp);
        if (ret == data_size) {
            return neko_result_success;
        }
    }

    return neko_result_failure;
}

char* __neko_platform_read_file_contents_into_string_null_term(const_str file_path, const_str mode, s32* sz) {
    char* buffer = 0;
    FILE* fp = fopen(file_path, mode);
    usize _sz = 0;
    if (fp) {
        _sz = __neko_platform_file_size_in_bytes(file_path);
        // fseek(fp, 0, SEEK_END);
        // _sz = ftell(fp);
        // fseek(fp, 0, SEEK_SET);
        buffer = (char*)neko_safe_malloc(_sz);
        if (buffer) {
            fread(buffer, 1, _sz, fp);
        }
        fclose(fp);
        // buffer[_sz] = '\0';
    }
    if (sz) *sz = _sz;
    return buffer;
}

neko_result __neko_platform_write_str_to_file(const_str contents, const_str mode, usize sz, const_str output_path) {
    FILE* fp = fopen(output_path, mode);
    if (fp) {
        s32 ret = fwrite(contents, sizeof(u8), sz, fp);
        if (ret == sz) {
            return neko_result_success;
        }
    }
    return neko_result_failure;
}

void __neko_platform_file_extension(char* buffer, usize buffer_sz, const_str file_path) { neko_util_get_file_extension(buffer, buffer_sz, file_path); }

neko_string __neko_platform_get_path(const neko_string& path) {

    neko_platform_ctx* ctx = __ctx();

    if (!ctx->gamepath) {
        neko_assert(ctx->gamepath, "gamepath not detected");
        return path;
    }

    neko_string get_path(ctx->gamepath);

    switch (neko::hash(path.substr(0, 4))) {
        case neko::hash("data"):
            get_path.append(path);
            break;
        default:
            break;
    }

    return get_path;
}

neko_string __neko_platform_abbreviate_path(const neko_string& path, s32 max_lenght) {
    if (path.length() <= max_lenght) return path;  // 如果路径长度不超过最大长度，则无需缩略，直接返回原路径

    int filename_pos = path.find_last_of('\\');
    if (filename_pos == std::string::npos) return path;

    std::string fileName = path.substr(filename_pos + 1);
    int dirPathLength = max_lenght - fileName.length() - 3;  // 3 表示省略号的长度
    if (dirPathLength <= 0) return "..." + fileName;         // 如果文件名已经超过最大长度，则只显示文件名前面的省略号和文件名
    std::string dirPath = path.substr(0, dirPathLength);

    return dirPath + "..." + fileName;
}

/*============================
// Platform UUID
============================*/

struct neko_uuid __neko_platform_generate_uuid() {
    struct neko_uuid uuid;

    char guid[40];
    s32 t = 0;
    char* sz_temp = "xxxxxxxxxxxx4xxxyxxxxxxxxxxxxxxx";
    char* sz_hex = "0123456789abcdef-";
    s32 n_len = strlen(sz_temp);

    for (t = 0; t < n_len + 1; t++) {
        s32 r = neko_rand_xorshf32() % 16;
        char c = ' ';

        switch (sz_temp[t]) {
            case 'x': {
                c = sz_hex[r];
            } break;
            case 'y': {
                c = sz_hex[(r & 0x03) | 0x08];
            } break;
            case '-': {
                c = '-';
            } break;
            case '4': {
                c = '4';
            } break;
        }

        guid[t] = (t < n_len) ? c : 0x00;
    }

    // Convert to uuid bytes from string
    const char *hex_string = sz_temp, *pos = hex_string;

    /* WARNING: no sanitization or error-checking whatsoever */
    for (usize count = 0; count < 16; count++) {
        sscanf(pos, "%2hhx", &uuid.bytes[count]);
        pos += 2;
    }

    return uuid;
}

// Mutable temp buffer 'tmp_buffer'
void __neko_platform_uuid_to_string(char* tmp_buffer, const struct neko_uuid* uuid) {
    neko_snprintf(tmp_buffer, 32, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", uuid->bytes[0], uuid->bytes[1], uuid->bytes[2], uuid->bytes[3], uuid->bytes[4], uuid->bytes[5],
                  uuid->bytes[6], uuid->bytes[7], uuid->bytes[8], uuid->bytes[9], uuid->bytes[10], uuid->bytes[11], uuid->bytes[12], uuid->bytes[13], uuid->bytes[14], uuid->bytes[15]);
}

u32 __neko_platform_hash_uuid(const struct neko_uuid* uuid) {
    char temp_buffer[] = neko_uuid_temp_str_buffer();
    __neko_platform_uuid_to_string(temp_buffer, uuid);
    return (neko_hash_str(temp_buffer));
}

void __neko_default_init_platform(struct neko_platform_i* platform) {
    neko_assert(platform);

    // Just assert these for now
    __neko_verify_platform_correctness(platform);

    /*============================
    // Platform Window
    ============================*/
    platform->windows = neko_slot_array_new(neko_platform_window_ptr);
    platform->active_window_handles = neko_dyn_array_new(neko_resource_handle);
    platform->create_window = &__neko_platform_create_window;
    platform->main_window = &__neko_platform_main_window;

    /*============================
    // Platform Input
    ============================*/
    platform->update_input = &__neko_platform_update_input;
    platform->press_key = &__neko_platform_press_key;
    platform->release_key = &__neko_platform_release_key;
    platform->was_key_down = &__neko_platform_was_key_down;
    platform->key_pressed = &__neko_platform_key_pressed;
    platform->key_down = &__neko_platform_key_down;
    platform->key_released = &__neko_platform_key_released;

    platform->press_mouse_button = &__neko_platform_press_mouse_button;
    platform->release_mouse_button = &__neko_platform_release_mouse_button;
    platform->was_mouse_down = &__neko_platform_was_mouse_down;
    platform->mouse_pressed = &__neko_platform_mouse_pressed;
    platform->mouse_down = &__neko_platform_mouse_down;
    platform->mouse_released = &__neko_platform_mouse_released;

    platform->mouse_delta = &__neko_platform_mouse_delta;
    platform->mouse_position = &__neko_platform_mouse_position;
    platform->mouse_position_x_y = &__neko_platform_mouse_position_x_y;
    platform->mouse_wheel = &__neko_platform_mouse_wheel;
    platform->mouse_wheel_x_y = &__neko_platform_mouse_wheel_x_y;

    /*============================
    // Platform UUID
    ============================*/
    platform->generate_uuid = &__neko_platform_generate_uuid;
    platform->uuid_to_string = &__neko_platform_uuid_to_string;
    platform->hash_uuid = &__neko_platform_hash_uuid;

    /*============================
    // Platform File IO
    ============================*/

    platform->file_exists = &__neko_platform_file_exists;
    platform->read_file_contents = &__neko_platform_read_file_contents_into_string_null_term;
    platform->write_file_contents = &__neko_write_file_contents;
    platform->write_str_to_file = &__neko_platform_write_str_to_file;
    platform->file_size_in_bytes = &__neko_platform_file_size_in_bytes;
    platform->file_extension = &__neko_platform_file_extension;
    platform->get_path = &__neko_platform_get_path;
    platform->abbreviate_path = &__neko_platform_abbreviate_path;

    // Default world time initialization
    platform->time.max_fps = 60.0;
    platform->time.current = 0.0;
    platform->time.delta = 0.0;
    platform->time.update = 0.0;
    platform->time.render = 0.0;
    platform->time.previous = 0.0;
    platform->time.frame = 0.0;

    // Custom initialize plaform layer
    platform->init(platform);
}

void __neko_verify_platform_correctness(struct neko_platform_i* platform) {
    neko_assert(platform);
    neko_assert(platform->init);
    neko_assert(platform->shutdown);
    neko_assert(platform->sleep);
    neko_assert(platform->elapsed_time);
    neko_assert(platform->process_input);
    neko_assert(platform->create_window_internal);
    neko_assert(platform->window_swap_buffer);
    neko_assert(platform->set_window_size);
    neko_assert(platform->window_size);
    neko_assert(platform->window_size_w_h);
    neko_assert(platform->set_cursor);
    neko_assert(platform->enable_vsync);
}

namespace neko {

// 固定大小的简单的线程安全环形缓冲区
template <typename T, size_t capacity>
class __neko_job_thread_safe_ring_buffer {
public:
    // 如果有可用空间则将项目推到末尾 若空间不足返回 false
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

    // 如果有则获取一个项目 若没有项目则返回 false
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
        wss << "ME_JobSystem_" << threadID;
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

void neko_job_execute(const std::function<void()>& job) {
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

void neko_job_dispatch(u32 jobCount, u32 groupSize, const std::function<void(neko_job_dispatch_args)>& job) {
    if (jobCount == 0 || groupSize == 0) {
        return;
    }

    // 计算要分派的作业组数量
    const u32 group_count = (jobCount + groupSize - 1) / groupSize;

    // 主线程标签状态更新
    current_label += group_count;

    for (u32 groupIndex = 0; groupIndex < group_count; ++groupIndex) {
        // 对于每个组 生成一个 real job
        const auto& jobGroup = [jobCount, groupSize, job, groupIndex]() {
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

#if defined(_MSC_VER)
typedef struct {
    char* out_ptr;
    const_str end_ptr;
} callstack_string_buffer_t;

static const_str alloc_string(callstack_string_buffer_t* buf, const_str str, size_t str_len) {
    char* res;

    if ((size_t)(buf->end_ptr - buf->out_ptr) < str_len + 1) return "<callstack buffer out of space!>";

    res = buf->out_ptr;
    buf->out_ptr += str_len + 1;
    memcpy(res, str, str_len);
    res[str_len] = '\0';
    return res;
}
#endif

#if defined(_MSC_VER)
#if defined(__clang__)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wignored-pragma-intrinsic"
#endif
#include <Windows.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#include <Dbghelp.h>

int neko_platform_callstack(int skip_frames, void** addresses, int num_addresses) { return RtlCaptureStackBackTrace(skip_frames + 1, num_addresses, addresses, 0); }

typedef BOOL(__stdcall* SymInitialize_f)(_In_ HANDLE hProcess, _In_opt_ PCSTR UserSearchPath, _In_ BOOL fInvadeProcess);
typedef BOOL(__stdcall* SymFromAddr_f)(_In_ HANDLE hProcess, _In_ DWORD64 Address, _Out_opt_ PDWORD64 Displacement, _Inout_ PSYMBOL_INFO Symbol);
typedef BOOL(__stdcall* SymGetLineFromAddr64_f)(_In_ HANDLE hProcess, _In_ DWORD64 qwAddr, _Out_ PDWORD pdwDisplacement, _Out_ PIMAGEHLP_LINE64 Line64);
typedef DWORD(__stdcall* SymSetOptions_f)(_In_ DWORD SymOptions);

static struct {
    int initialized;
    int init_ok;
    SymInitialize_f SymInitialize;
    SymFromAddr_f SymFromAddr;
    SymGetLineFromAddr64_f SymGetLineFromAddr64;
    SymSetOptions_f SymSetOptions;
} dbghelp = {0, 0, 0, 0, 0, 0};

static HMODULE callstack_symbols_find_dbghelp() {

    HMODULE mod;
    DWORD module_path_len;
    char module_path[4096];

    module_path_len = GetModuleFileName((HMODULE)0, (LPWSTR)module_path, sizeof(module_path));
    if (module_path_len > 0) {
        char* slash = strrchr(module_path, '\\');
        if (slash) *(slash + 1) = '\0';
        strncat(module_path, "dbghelp.dll", sizeof(module_path) - strlen(module_path) - 1);

        mod = LoadLibrary((LPWSTR)module_path);
        if (mod) return mod;
    }

    module_path_len = GetCurrentDirectory(sizeof(module_path), (LPWSTR)module_path);
    if (module_path_len > 0) {
        strncat(module_path, "\\dbghelp.dll", sizeof(module_path) - strlen(module_path) - 1);

        mod = LoadLibrary((LPWSTR)module_path);
        if (mod) return mod;
    }

    return LoadLibrary(L"dbghelp.dll");
}

static void callstack_symbols_build_search_path(char* search_path, int length) {

    DWORD mod;
    char env_var[4096];

    search_path[0] = '\0';

    mod = GetModuleFileName((HMODULE)0, (LPWSTR)search_path, length);
    if (mod > 0) {
        char* slash = strrchr(search_path, '\\');
        if (slash) *slash = '\0';
    } else {
        search_path[0] = '\0';
    }

    if (length > 2) {
        if (search_path[0] != '\0') strncat(search_path, ";", length);
        strncat(search_path, ".", length);
    }

    mod = GetEnvironmentVariable(L"_NT_SYMBOL_PATH", (LPWSTR)env_var, sizeof(env_var));
    if (mod > 0 && mod < sizeof(env_var)) {
        if (search_path[0] != '\0') strncat(search_path, ";", length);
        strncat(search_path, env_var, length);
    }

    mod = GetEnvironmentVariable(L"_NT_ALTERNATE_SYMBOL_PATH", (LPWSTR)env_var, sizeof(env_var));
    if (mod > 0 && mod < sizeof(env_var)) {
        if (search_path[0] != '\0') strncat(search_path, ";", length);
        strncat(search_path, env_var, length);
    }
}

static int callstack_symbols_initialize() {
    HANDLE process;
    HMODULE mod;
    BOOL res;
    DWORD err;

    if (!dbghelp.initialized) {
        mod = callstack_symbols_find_dbghelp();
        if (mod) {
            dbghelp.SymInitialize = (SymInitialize_f)GetProcAddress(mod, "SymInitialize");
            dbghelp.SymFromAddr = (SymFromAddr_f)GetProcAddress(mod, "SymFromAddr");
            dbghelp.SymGetLineFromAddr64 = (SymGetLineFromAddr64_f)GetProcAddress(mod, "SymGetLineFromAddr64");
            dbghelp.SymSetOptions = (SymSetOptions_f)GetProcAddress(mod, "SymSetOptions");
        }

        if (dbghelp.SymInitialize && dbghelp.SymFromAddr && dbghelp.SymGetLineFromAddr64 && dbghelp.SymSetOptions) {

            if (GetEnvironmentVariable(L"DBGTOOLS_SYMBOL_DEBUG_OUTPUT", 0, 0) > 0) dbghelp.SymSetOptions(SYMOPT_DEBUG);

            char search_path[4096];
            callstack_symbols_build_search_path(search_path, sizeof(search_path) - 1);

            process = GetCurrentProcess();
            res = dbghelp.SymInitialize(process, search_path, TRUE);

            dbghelp.init_ok = 1;
            if (res == 0) {
                err = GetLastError();

                if (err != ERROR_INVALID_PARAMETER) dbghelp.init_ok = 0;
            }
        }
    }

    dbghelp.initialized = 1;
    return dbghelp.init_ok;
}

int neko_platform_callstack_symbols(void** addresses, neko_platform_callstack_symbol_t* out_syms, int num_addresses, char* memory, int mem_size) {
    HANDLE process;
    DWORD64 offset;
    DWORD line_dis;
    BOOL res;
    IMAGEHLP_LINE64 line;
    PSYMBOL_INFO sym_info;
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    int num_translated = 0;
    callstack_string_buffer_t outbuf = {memory, memory + mem_size};

    memset(out_syms, 0x0, (size_t)num_addresses * sizeof(neko_platform_callstack_symbol_t));

    if (!callstack_symbols_initialize()) {
        out_syms[0].function = "failed to initialize dbghelp.dll";
        out_syms[0].offset = 0;
        out_syms[0].file = "";
        out_syms[0].line = 0;
        return 1;
    }

    process = GetCurrentProcess();
    sym_info = (PSYMBOL_INFO)buffer;
    sym_info->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym_info->MaxNameLen = MAX_SYM_NAME;

    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    for (int i = 0; i < num_addresses; ++i) {
        res = dbghelp.SymFromAddr(process, (DWORD64)addresses[i], &offset, sym_info);
        if (res == 0)
            out_syms[i].function = "failed to lookup symbol";
        else
            out_syms[i].function = alloc_string(&outbuf, sym_info->Name, sym_info->NameLen);

        res = dbghelp.SymGetLineFromAddr64(process, (DWORD64)addresses[i], &line_dis, &line);
        if (res == 0) {
            out_syms[i].offset = 0;
            out_syms[i].file = "failed to lookup file";
            out_syms[i].line = 0;
        } else {
            out_syms[i].offset = (unsigned int)line_dis;
            out_syms[i].file = alloc_string(&outbuf, line.FileName, strlen(line.FileName));
            out_syms[i].line = (unsigned int)line.LineNumber;
        }

        ++num_translated;
    }
    return num_translated;
}

#else

int callstack(int skip_frames, void** addresses, int num_addresses) {
    (void)skip_frames;
    (void)addresses;
    (void)num_addresses;
    return 0;
}

int callstack_symbols(void** addresses, callstack_symbol_t* out_syms, int num_addresses, char* memory, int mem_size) {
    (void)addresses;
    (void)out_syms;
    (void)num_addresses;
    (void)memory;
    (void)mem_size;
    return 0;
}

#endif

void neko_platform_print_callstack() {
    void* addresses[256];
    int num_addresses = neko_platform_callstack(0, addresses, 256);

    neko_platform_callstack_symbol_t symbols[256];
    char symbols_buffer[2048];
    num_addresses = neko_platform_callstack_symbols(addresses, symbols, num_addresses, symbols_buffer, 2048);

    int i;
    for (i = 0; i < num_addresses; ++i) printf("%3d) %-50s %s(%u)\n", i, symbols[i].function, symbols[i].file, symbols[i].line);
}
}  // namespace neko