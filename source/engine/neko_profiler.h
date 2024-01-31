
#ifndef NEKO_PROFILER_H
#define NEKO_PROFILER_H

#define __neko_profiler_scopes_max (16 * 1024)
#define __neko_profiler_text_max (1024 * 1024)
#define __neko_profiler_threads_max (16)

#include "engine/neko.h"
#include "engine/neko_platform.h"

// #include "engine/common/neko_util.h"
// #include "engine/platform/neko_platform.h"

typedef struct neko_profiler_scope_stats_s {
    u64 inclusive_time;
    u64 exclusive_time;
    u64 inclusive_time_total;
    u64 exclusive_time_total;
    u32 occurences;

} neko_profiler_scope_stats;

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

typedef struct neko_profiler_thread_s {
    u64 thread_id;
    const_str name;

} neko_profiler_thread;

typedef struct neko_profiler_frame_s {
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

} neko_profiler_frame_t;

NEKO_API_DECL void neko_profiler_init();
NEKO_API_DECL void neko_profiler_shutdown();
NEKO_API_DECL void neko_profiler_set_threshold(f32 _ms, s32 _level);
NEKO_API_DECL void neko_profiler_register_thread(const_str _name, u64 _threadID);
NEKO_API_DECL void neko_profiler_unregister_thread(u64 _threadID);
NEKO_API_DECL void neko_profiler_begin_frame();
NEKO_API_DECL uintptr_t neko_profiler_begin_scope(const_str _file, s32 _line, const_str _name);
NEKO_API_DECL void neko_profiler_end_scope(uintptr_t _scopeHandle);
NEKO_API_DECL s32 neko_profiler_is_paused();
NEKO_API_DECL s32 neko_profiler_was_threshold_crossed();
NEKO_API_DECL void neko_profiler_set_paused(s32 _paused);
NEKO_API_DECL void neko_profiler_get_frame(neko_profiler_frame_t *_data);
NEKO_API_DECL s32 neko_profiler_save(neko_profiler_frame_t *_data, void *_buffer, size_t _bufferSize);
NEKO_API_DECL void neko_profiler_load(neko_profiler_frame_t *_data, void *_buffer, size_t _bufferSize);
NEKO_API_DECL void neko_profiler_load_time_only(f32 *_time, void *_buffer, size_t _bufferSize);
NEKO_API_DECL void neko_profiler_release(neko_profiler_frame_t *_data);
NEKO_API_DECL u64 neko_profiler_get_clock();
NEKO_API_DECL u64 __neko_profiler_get_clock_frequency();
NEKO_API_DECL f32 __neko_profiler_clock2ms(u64 _clock, u64 _frequency);

struct neko_profiler_scoped {
    uintptr_t scope;
    // neko_profiler_scoped(const_str _file, s32 _line, const_str _name) { scope = neko_profiler_begin_scope(_file, _line, _name); }
    //~neko_profiler_scoped() { neko_profiler_end_scope(scope); }
};

#ifndef NEKO_DISABLE_PROFILING

#define neko_profiler_init() neko_profiler_init()
// #define neko_profiler_scope_auto(x, ...) neko_profiler_scoped neko_concat(__neko_gen_profile_scope_, __LINE__)(__FILE__, __LINE__, x)
#define neko_profiler_scope_begin(n, ...) \
    uintptr_t profileid_##n;              \
    profileid_##n = neko_profiler_begin_scope(__FILE__, __LINE__, #n)
#define neko_profiler_scope_end(n) neko_profiler_end_scope(profileid_##n)
#define neko_profiler_begin() neko_profiler_begin_frame()
#define neko_profiler_thread(n) neko_profiler_register_thread(n)
#define neko_profiler_shutdown() neko_profiler_shutdown()
#else
#define neko_profiler_init() void()
#define neko_profiler_begin() void()
#define neko_profiler_thread(n) void()
#define neko_profiler_shutdown() void()
#endif

typedef struct neko_profiler_free_list_s {
    u32 max_blocks;
    u32 block_size;
    u32 blocks_free;
    u32 blocks_alllocated;
    u8 *buffer;
    u8 *next;
} neko_profiler_free_list_t;

NEKO_API_DECL void neko_profiler_free_list_create(size_t _blockSize, u32 _maxBlocks, neko_profiler_free_list_t *_freeList);
NEKO_API_DECL void neko_profiler_free_list_destroy(neko_profiler_free_list_t *_freeList);
NEKO_API_DECL void *neko_profiler_free_list_alloc(neko_profiler_free_list_t *_freeList);
NEKO_API_DECL void neko_profiler_free_list_free(neko_profiler_free_list_t *_freeList, void *_ptr);
NEKO_API_DECL s32 neko_profiler_free_list_check_ptr(neko_profiler_free_list_t *_freeList, void *_ptr);

typedef enum { Capture, Display, Open, Count } buffer_use;

#if defined(__cplusplus)

#include <map>
#include <string>

typedef struct neko_profiler_context_s {
    neko_mutex internal_mutex;

    // neko_pthread_mutex mutex;
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
    char names_data_buffers[Count][__neko_profiler_text_max];
    char *names_data[Count];
    s32 names_size[Count];
    u32 tls_level;

    // const_str thread_names[16];
    // s32 thread_num;

    std::map<u64, std::string> thread_names;
} neko_profiler_context_t;

void neko_profiler_context_init(neko_profiler_context_t *ctx);
void neko_profiler_context_shutdown(neko_profiler_context_t *ctx);

void neko_profiler_context_set_threshold(neko_profiler_context_t *ctx, f32 _ms, s32 _levelThreshold);
bool neko_profiler_context_is_paused(neko_profiler_context_t *ctx);
bool neko_profiler_context_was_threshold_crossed(neko_profiler_context_t *ctx);
void neko_profiler_context_set_paused(neko_profiler_context_t *ctx, bool _paused);
void neko_profiler_context_register_thread(neko_profiler_context_t *ctx, u64 _threadID, const_str _name);
void neko_profiler_context_unregister_thread(neko_profiler_context_t *ctx, u64 _threadID);
void neko_profiler_context_begin_frame(neko_profiler_context_t *ctx);
s32 neko_profiler_context_inc_level(neko_profiler_context_t *ctx);
void neko_profiler_context_dec_level(neko_profiler_context_t *ctx);
profiler_scope *neko_profiler_context_begin_scope(neko_profiler_context_t *ctx, const_str _file, s32 _line, const_str _name);
void neko_profiler_context_end_scope(neko_profiler_context_t *ctx, profiler_scope *_scope);
const_str neko_profiler_context_add_string(neko_profiler_context_t *ctx, const_str _name, buffer_use _buffer);
void neko_profiler_context_get_frame_data(neko_profiler_context_t *ctx, neko_profiler_frame_t *_data);

#endif

#endif