
#ifndef NEKO_PROFILER_HPP
#define NEKO_PROFILER_HPP

#define __neko_profiler_scopes_max (16 * 1024)
#define __neko_profiler_text_max (1024 * 1024)
#define __neko_profiler_threads_max (16)

#include <map>
#include <string>

#include "engine/common/neko_util.h"
#include "engine/platform/neko_platform.h"

namespace neko {

typedef struct neko_profiler_scope_stats {
    u64 inclusive_time;
    u64 exclusive_time;
    u64 inclusive_time_total;
    u64 exclusive_time_total;
    u32 occurences;

} profiler_scope_stats;

typedef struct neko_profiler_scope_t {
    u64 start;
    u64 end;
    u64 thread_id;
    const_str name;
    const_str file;
    u32 line;
    u32 level;
    neko_profiler_scope_stats *stats;

} profiler_scope;

typedef struct neko_profiler_thread {
    u64 thread_id;
    const_str name;

} profiler_thread;

typedef struct neko_profiler_frame_t {
    u32 num_scopes;
    profiler_scope *scopes;
    u32 num_threads;
    neko_profiler_thread *threads;
    u64 start_time;
    u64 end_time;
    u64 prev_frame_time;
    u64 cpu_frequency;
    f32 time_threshold;
    u32 level_threshold;
    u32 num_scopes_stats;
    profiler_scope *scopes_stats;
    neko_profiler_scope_stats *scope_stats_info;

} profiler_frame;

void neko_profiler_init();
void neko_profiler_shutdown();
void neko_profiler_set_threshold(f32 _ms, s32 _level = 0);
void neko_profiler_register_thread(const_str _name, u64 _threadID = 0);
void neko_profiler_unregister_thread(u64 _threadID);
void neko_profiler_begin_frame();
uintptr_t neko_profiler_begin_scope(const_str _file, s32 _line, const_str _name);
void neko_profiler_end_scope(uintptr_t _scopeHandle);
s32 neko_profiler_is_paused();
s32 neko_profiler_was_threshold_crossed();
void neko_profiler_set_paused(s32 _paused);
void neko_profiler_get_frame(profiler_frame *_data);
s32 neko_profiler_save(profiler_frame *_data, void *_buffer, size_t _bufferSize);
void neko_profiler_load(profiler_frame *_data, void *_buffer, size_t _bufferSize);
void neko_profiler_load_time_only(f32 *_time, void *_buffer, size_t _bufferSize);
void neko_profiler_release(profiler_frame *_data);
u64 neko_profiler_get_clock();
u64 __neko_profiler_get_clock_frequency();
f32 __neko_profiler_clock2ms(u64 _clock, u64 _frequency);

struct neko_profiler_scoped {
    uintptr_t scope;
    neko_profiler_scoped(const_str _file, s32 _line, const_str _name) { scope = neko_profiler_begin_scope(_file, _line, _name); }
    ~neko_profiler_scoped() { neko_profiler_end_scope(scope); }
};

#ifndef NEKO_DISABLE_PROFILING

#define neko_profiler_init() neko_profiler_init()
#define neko_profiler_scope_auto(x, ...) neko_profiler_scoped neko_concat(__neko_gen_profile_scope, __LINE__)(__FILE__, __LINE__, x)
#define neko_profiler_scope_begin(n, ...) \
    uintptr_t profileid_##n;              \
    profileid_##n = neko_profiler_begin_scope(__FILE__, __LINE__, #n);
#define neko_profiler_scope_end(n) neko_profiler_end_scope(profileid_##n);
#define neko_profiler_begin() neko_profiler_begin_frame()
#define neko_profiler_thread(n) neko_profiler_register_thread(n)
#define neko_profiler_shutdown() neko_profiler_shutdown()
#else
#define neko_profiler_init() void()
#define neko_profiler_begin() void()
#define neko_profiler_thread(n) void()
#define neko_profiler_shutdown() void()
#endif

struct neko_profiler_free_list_t {
    u32 max_blocks;
    u32 block_size;
    u32 blocks_free;
    u32 blocks_alllocated;
    u8 *buffer;
    u8 *next;
};

void neko_profiler_free_list_create(size_t _blockSize, u32 _maxBlocks, neko_profiler_free_list_t *_freeList);
void neko_profiler_free_list_destroy(neko_profiler_free_list_t *_freeList);
void *neko_profiler_free_list_alloc(neko_profiler_free_list_t *_freeList);
void neko_profiler_free_list_free(neko_profiler_free_list_t *_freeList, void *_ptr);
s32 neko_profiler_free_list_check_ptr(neko_profiler_free_list_t *_freeList, void *_ptr);

class neko_profiler_context {
    enum buffer_use {
        Capture,
        Display,
        Open,

        Count
    };

    neko_pthread_mutex mutex;
    neko_profiler_free_list_t scopes_allocator;
    u32 scopes_open;
    profiler_scope *scopes_capture[__neko_profiler_scopes_max];
    profiler_scope scopes_display[__neko_profiler_scopes_max];
    u32 display_scopes;
    u64 frame_start_time;
    u64 frame_end_time;
    bool threshold_crossed;
    f32 time_threshold;
    u32 level_threshold;
    bool pause_profiling;
    char names_data_buffers[buffer_use::Count][__neko_profiler_text_max];
    char *names_data[buffer_use::Count];
    s32 names_size[buffer_use::Count];
    u32 tls_level;

    std::map<u64, std::string> thread_names;

public:
    neko_profiler_context();
    ~neko_profiler_context();

    void set_threshold(f32 _ms, s32 _levelThreshold);
    bool is_paused();
    bool was_threshold_crossed();
    void set_paused(bool _paused);
    void register_thread(u64 _threadID, const_str _name);
    void unregister_thread(u64 _threadID);
    void begin_frame();
    s32 inc_level();
    void dec_level();
    profiler_scope *begin_ccope(const_str _file, s32 _line, const_str _name);
    void end_scope(profiler_scope *_scope);
    const_str add_string(const_str _name, buffer_use _buffer);
    void get_frame_data(profiler_frame *_data);
};

}  // namespace neko

#endif