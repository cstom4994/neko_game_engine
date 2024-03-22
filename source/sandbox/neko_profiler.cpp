
#include "neko_profiler.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "engine/util/neko_asset.h"

u64 __neko_profiler_get_clock_frequency();

neko_profiler_context_t *g_profiler_context = 0;

void neko_profiler_init() {
    // g_profiler_context = (neko_profiler_context_t *)malloc(sizeof(neko_profiler_context_t));
    g_profiler_context = new neko_profiler_context_t();

    neko_profiler_context_init(g_profiler_context);
}

void neko_profiler_shutdown() {

    neko_profiler_context_shutdown(g_profiler_context);
    // free(g_profiler_context);
    delete g_profiler_context;
    g_profiler_context = 0;
}

void neko_profiler_set_threshold(f32 _ms, s32 _level) { neko_profiler_context_set_threshold(g_profiler_context, _ms, _level); }

void neko_profiler_register_thread(const_str _name, u64 _thread_id) {
    if (_thread_id == 0) _thread_id = neko_get_thread_id();

    neko_profiler_context_register_thread(g_profiler_context, _thread_id, _name);
}

void neko_profiler_unregister_thread(u64 _thread_id) { neko_profiler_context_unregister_thread(g_profiler_context, _thread_id); }

void neko_profiler_begin_frame() { neko_profiler_context_begin_frame(g_profiler_context); }

uintptr_t neko_profiler_begin_scope(const_str _file, s32 _line, const_str _name) { return (uintptr_t)neko_profiler_context_begin_scope(g_profiler_context, _file, _line, _name); }

void neko_profiler_end_scope(uintptr_t _scopeHandle) { neko_profiler_context_end_scope(g_profiler_context, (neko_profiler_scope_t *)_scopeHandle); }

s32 neko_profiler_is_paused() { return neko_profiler_context_is_paused(g_profiler_context) ? 1 : 0; }

s32 neko_profiler_was_threshold_crossed() { return neko_profiler_context_was_threshold_crossed(g_profiler_context) ? 1 : 0; }

void neko_profiler_set_paused(s32 _paused) { return neko_profiler_context_set_paused(g_profiler_context, _paused != 0); }

void neko_profiler_get_frame(neko_profiler_frame_t *_data) {
    neko_profiler_context_get_frame_data(g_profiler_context, _data);

    // clamp scopes crossing frame boundary
    const u32 numScopes = _data->num_scopes;
    for (u32 i = 0; i < numScopes; ++i) {
        neko_profiler_scope_t &cs = _data->scopes[i];

        if (cs.start == cs.end) {
            cs.end = _data->end_time;
            if (cs.start < _data->start_time) cs.start = _data->start_time;
        }
    }
}

s32 neko_profiler_save(neko_profiler_frame_t *_data, void *_buffer, size_t _buffer_size) {
    // fill string data
    string_store str_store;
    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t &scope = _data->scopes[i];
        str_store.add_string(scope.name);
        str_store.add_string(scope.file);
    }
    for (u32 i = 0; i < _data->num_threads; ++i) {
        str_store.add_string(_data->threads[i].name);
    }

    // calc data size
    u32 total_size = _data->num_scopes * sizeof(neko_profiler_scope_t) + _data->num_threads * sizeof(neko_profiler_thread) + sizeof(neko_profiler_frame_t) + str_store.total_size;

    u8 *buffer = new u8[total_size];
    u8 *bufPtr = buffer;

    write_var(buffer, _data->start_time);
    write_var(buffer, _data->end_time);
    write_var(buffer, _data->prev_frame_time);
    // writeVar(buffer, _data->m_platformID);
    write_var(buffer, __neko_profiler_get_clock_frequency());

    // write scopes
    write_var(buffer, _data->num_scopes);
    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t *scope = &_data->scopes[i];
        write_var(buffer, scope->start);
        write_var(buffer, scope->end);
        write_var(buffer, scope->thread_id);
        write_var(buffer, str_store.get_string(scope->name));
        write_var(buffer, str_store.get_string(scope->file));
        write_var(buffer, scope->line);
        write_var(buffer, scope->level);
    }

    // write thread info
    write_var(buffer, _data->num_threads);
    for (u32 i = 0; i < _data->num_threads; ++i) {
        neko_profiler_thread *t = &_data->threads[i];
        write_var(buffer, t->thread_id);
        write_var(buffer, str_store.get_string(t->name));
    }

    // write string data
    u32 num_strings = (u32)str_store.strings.size();
    write_var(buffer, num_strings);

    for (u32 i = 0; i < str_store.strings.size(); ++i) write_str(buffer, str_store.strings[i].c_str());

    const int max_dst_size = neko_lz_bounds((s32)_buffer_size, 0);
    s32 comp_size = neko_lz_encode((const_str)bufPtr, (s32)(buffer - bufPtr), (char *)_buffer, max_dst_size, 9);

    // s32 compSize = LZ4_compress_default((const_str)bufPtr, (char *)_buffer, (s32)(buffer - bufPtr), (s32)_buffer_size);
    delete[] bufPtr;
    return comp_size;
}

void neko_profiler_load(neko_profiler_frame_t *_data, void *_buffer, size_t _buffer_size) {
    size_t buffer_size = _buffer_size;
    u8 *buffer = 0;
    u8 *bufferPtr;

    s32 decomp = -1;
    do {
        free(buffer);
        buffer_size *= 2;
        buffer = (u8 *)malloc(buffer_size * sizeof(u8));
        // decomp = LZ4_decompress_safe((const_str)_buffer, (char *)buffer, (s32)_buffer_size, (s32)buffer_size);
        decomp = neko_lz_decode((const_str)_buffer, (s32)_buffer_size, (char *)buffer, (s32)buffer_size);

    } while (decomp < 0);

    bufferPtr = buffer;

    u32 strIdx;

    read_var(buffer, _data->start_time);
    read_var(buffer, _data->end_time);
    read_var(buffer, _data->prev_frame_time);
    // readVar(buffer, _data->m_platformID);
    read_var(buffer, _data->cpu_frequency);

    // read scopes
    read_var(buffer, _data->num_scopes);

    _data->scopes = new neko_profiler_scope_t[_data->num_scopes * 2];  // extra space for viewer - m_scopesStats
    _data->scopes_stats = &_data->scopes[_data->num_scopes];
    _data->scope_stats_info = new neko_profiler_scope_stats[_data->num_scopes * 2];

    for (u32 i = 0; i < _data->num_scopes * 2; ++i) _data->scopes[i].stats = &_data->scope_stats_info[i];

    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t &scope = _data->scopes[i];
        read_var(buffer, scope.start);
        read_var(buffer, scope.end);
        read_var(buffer, scope.thread_id);
        read_var(buffer, strIdx);
        scope.name = (const_str)(uintptr_t)strIdx;
        read_var(buffer, strIdx);
        scope.file = (const_str)(uintptr_t)strIdx;
        read_var(buffer, scope.line);
        read_var(buffer, scope.level);

        scope.stats->inclusive_time = scope.end - scope.start;
        scope.stats->exclusive_time = scope.stats->inclusive_time;
        scope.stats->occurences = 0;
    }

    // read thread info
    read_var(buffer, _data->num_threads);
    _data->threads = new neko_profiler_thread[_data->num_threads];
    for (u32 i = 0; i < _data->num_threads; ++i) {
        neko_profiler_thread &t = _data->threads[i];
        read_var(buffer, t.thread_id);
        read_var(buffer, strIdx);
        t.name = (const_str)(uintptr_t)strIdx;
    }

    // read string data
    u32 numStrings;
    read_var(buffer, numStrings);

    const_str strings[__neko_profiler_scopes_max];
    for (u32 i = 0; i < numStrings; ++i) strings[i] = read_string(buffer);

    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t &scope = _data->scopes[i];
        uintptr_t idx = (uintptr_t)scope.name;
        scope.name = duplicate_string(strings[(u32)idx]);

        idx = (uintptr_t)scope.file;
        scope.file = duplicate_string(strings[(u32)idx]);
    }

    for (u32 i = 0; i < _data->num_threads; ++i) {
        neko_profiler_thread &t = _data->threads[i];
        uintptr_t idx = (uintptr_t)t.name;
        t.name = duplicate_string(strings[(u32)idx]);
    }

    for (u32 i = 0; i < numStrings; ++i) delete[] strings[i];

    free(bufferPtr);

    // process frame data

    for (u32 i = 0; i < _data->num_scopes; ++i)
        for (u32 j = 0; j < _data->num_scopes; ++j) {
            neko_profiler_scope_t &scopeI = _data->scopes[i];
            neko_profiler_scope_t &scopeJ = _data->scopes[j];

            if ((scopeJ.start > scopeI.start) && (scopeJ.end < scopeI.end) && (scopeJ.level == scopeI.level + 1) && (scopeJ.thread_id == scopeI.thread_id))
                scopeI.stats->exclusive_time -= scopeJ.stats->inclusive_time;
        }

    _data->num_scopes_stats = 0;

    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t &scopeI = _data->scopes[i];

        scopeI.stats->inclusive_time_total = scopeI.stats->inclusive_time;
        scopeI.stats->exclusive_time_total = scopeI.stats->exclusive_time;

        s32 foundIndex = -1;
        for (u32 j = 0; j < _data->num_scopes_stats; ++j) {
            neko_profiler_scope_t &scopeJ = _data->scopes_stats[j];
            if (strcmp(scopeI.name, scopeJ.name) == 0) {
                foundIndex = j;
                break;
            }
        }

        if (foundIndex == -1) {
            s32 index = _data->num_scopes_stats++;
            neko_profiler_scope_t &scope = _data->scopes_stats[index];
            scope = scopeI;
            scope.stats->occurences = 1;
        } else {
            neko_profiler_scope_t &scope = _data->scopes_stats[foundIndex];
            scope.stats->inclusive_time_total += scopeI.stats->inclusive_time;
            scope.stats->exclusive_time_total += scopeI.stats->exclusive_time;
            scope.stats->occurences++;
        }
    }
}

void neko_profiler_load_time_only(f32 *_time, void *_buffer, size_t _buffer_size) {
    size_t buffer_size = _buffer_size;
    u8 *buffer = 0;
    u8 *bufferPtr;

    s32 decomp = -1;
    do {
        delete[] buffer;
        buffer_size *= 2;
        buffer = new u8[buffer_size];
        // decomp = LZ4_decompress_safe((const_str)_buffer, (char *)buffer, (s32)_buffer_size, (s32)buffer_size);
        decomp = neko_lz_decode((const_str)_buffer, (s32)_buffer_size, (char *)buffer, (s32)buffer_size);

    } while (decomp < 0);

    bufferPtr = buffer;

    u64 start_time;
    u64 end_time;
    u8 dummy8;
    u64 frequency;

    read_var(buffer, start_time);
    read_var(buffer, end_time);
    read_var(buffer, frequency);  // dummy
    read_var(buffer, dummy8);     // dummy
    read_var(buffer, frequency);
    *_time = __neko_profiler_clock2ms(end_time - start_time, frequency);

    delete[] buffer;
}

void neko_profiler_release(neko_profiler_frame_t *_data) {
    for (u32 i = 0; i < _data->num_scopes; ++i) {
        neko_profiler_scope_t &scope = _data->scopes[i];
        delete[] scope.name;
        delete[] scope.file;
    }

    for (u32 i = 0; i < _data->num_threads; ++i) {
        neko_profiler_thread &t = _data->threads[i];
        delete[] t.name;
    }

    delete[] _data->scopes;
    delete[] _data->threads;
    delete[] _data->scope_stats_info;
}

u64 neko_profiler_get_clock() {
#ifdef NEKO_PLATFORM_WIN
#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
    u64 q = __rdtsc();
#else
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    int64_t q = li.QuadPart;
#endif
#else
    struct timeval now;
    gettimeofday(&now, 0);
    int64_t q = now.tv_sec * 1000000 + now.tv_usec;
#endif
    return q;
}

u64 __neko_profiler_get_clock_frequency() {
#ifdef NEKO_PLATFORM_WIN
#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
#if defined(NEKO_DEBUG)
    static LONGLONG frequency = 1;
    static bool initialized = false;
    if (!initialized) {
        LARGE_INTEGER li1, li2;
        QueryPerformanceCounter(&li1);
        LONGLONG tsc1 = __rdtsc();
        for (s32 i = 0; i < 230000000; ++i)
            ;
        LONGLONG tsc2 = __rdtsc();
        QueryPerformanceCounter(&li2);

        LARGE_INTEGER lif;
        QueryPerformanceFrequency(&lif);
        LONGLONG time = ((li2.QuadPart - li1.QuadPart) * 1000) / lif.QuadPart;
        frequency = (LONGLONG)(1000 * ((tsc2 - tsc1) / time));
        initialized = true;
    }
    return frequency;
#else
    return 2300000000;
#endif
#else
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    return li.QuadPart;
#endif
#else
    return 1000000;
#endif
}

f32 __neko_profiler_clock2ms(u64 _clock, u64 _frequency) { return (f32(_clock) / f32(_frequency)) * 1000.0f; }

void neko_profiler_free_list_create(size_t _blockSize, u32 _maxBlocks, neko_profiler_free_list_t *_freeList) {
    _freeList->max_blocks = _maxBlocks;
    _freeList->block_size = (u32)_blockSize;
    _freeList->blocks_free = _maxBlocks;
    _freeList->blocks_alllocated = 0;
    _freeList->buffer = (u8 *)neko_safe_malloc(_blockSize * _maxBlocks);
    _freeList->next = _freeList->buffer;
}

void neko_profiler_free_list_destroy(neko_profiler_free_list_t *_freeList) { neko_safe_free(_freeList->buffer); }

void *neko_profiler_free_list_alloc(neko_profiler_free_list_t *_freeList) {
    if (_freeList->blocks_alllocated < _freeList->max_blocks) {
        u32 *p = (u32 *)(_freeList->buffer + (_freeList->blocks_alllocated * _freeList->block_size));
        *p = _freeList->blocks_alllocated + 1;
        _freeList->blocks_alllocated++;
    }

    void *ret = 0;
    if (_freeList->blocks_free) {
        ret = _freeList->next;
        --_freeList->blocks_free;
        if (_freeList->blocks_free)
            _freeList->next = _freeList->buffer + (*(u32 *)_freeList->next * _freeList->block_size);
        else
            _freeList->next = 0;
    }
    return ret;
}

void neko_profiler_free_list_free(neko_profiler_free_list_t *_freeList, void *_ptr) {
    if (_freeList->next) {
        u32 index = ((u32)(_freeList->next - _freeList->buffer)) / _freeList->block_size;
        *(u32 *)_ptr = index;
        _freeList->next = (u8 *)_ptr;
    } else {
        *(u32 *)_ptr = _freeList->max_blocks;
        _freeList->next = (u8 *)_ptr;
    }
    ++_freeList->blocks_free;
}

s32 neko_profiler_free_list_check_ptr(neko_profiler_free_list_t *_freeList, void *_ptr) {
    return ((uintptr_t)_freeList->max_blocks * (uintptr_t)_freeList->block_size) > (uintptr_t)(((u8 *)_ptr) - _freeList->buffer) ? 1 : 0;
}

void neko_profiler_context_init(neko_profiler_context_t *ctx) {
    neko_mutex_init(&ctx->internal_mutex);

    ctx->tls_level = neko_tls_allocate();

    ctx->scopes_open = 0;
    ctx->display_scopes = 0;
    ctx->frame_start_time = 0;
    ctx->frame_end_time = 0;
    ctx->threshold_crossed = false;
    ctx->time_threshold = 0.0f;
    ctx->level_threshold = 0;
    ctx->pause_profiling = false;

    neko_profiler_free_list_create(sizeof(neko_profiler_scope_t), __neko_profiler_scopes_max, &ctx->scopes_allocator);

    for (s32 i = 0; i < buffer_use::count; ++i) {
        ctx->names_size[i] = 0;
        ctx->names_data[i] = ctx->names_data_buffers[i];
    }
}

void neko_profiler_context_shutdown(neko_profiler_context_t *ctx) {
    neko_profiler_free_list_destroy(&ctx->scopes_allocator);
    neko_tls_free(ctx->tls_level);

    neko_mutex_destroy(&ctx->internal_mutex);
}

void neko_profiler_context_set_threshold(neko_profiler_context_t *ctx, f32 _ms, s32 _levelThreshold) {
    ctx->time_threshold = _ms;
    ctx->level_threshold = _levelThreshold;
}

bool neko_profiler_context_is_paused(neko_profiler_context_t *ctx) { return ctx->pause_profiling; }

bool neko_profiler_context_was_threshold_crossed(neko_profiler_context_t *ctx) { return !ctx->pause_profiling && ctx->threshold_crossed; }

void neko_profiler_context_set_paused(neko_profiler_context_t *ctx, bool _paused) { ctx->pause_profiling = _paused; }

void neko_profiler_context_register_thread(neko_profiler_context_t *ctx, u64 _thread_id, const_str _name) { ctx->thread_names[_thread_id] = _name; }

void neko_profiler_context_unregister_thread(neko_profiler_context_t *ctx, u64 _thread_id) { ctx->thread_names.erase(_thread_id); }

void neko_profiler_context_begin_frame(neko_profiler_context_t *ctx) {
    neko_mutex_lock(&ctx->internal_mutex);

    u64 frameBeginTime, frameEndTime;
    static u64 beginPrevFrameTime = neko_profiler_get_clock();
    frameBeginTime = beginPrevFrameTime;
    frameEndTime = neko_profiler_get_clock();
    beginPrevFrameTime = frameEndTime;

    static u64 frameTime = frameEndTime - frameBeginTime;

    ctx->threshold_crossed = false;

    s32 level = (s32)ctx->level_threshold - 1;

    u32 scopes_to_restart = 0;

    ctx->names_size[buffer_use::open] = 0;

    static neko_profiler_scope_t scopesDisplay[__neko_profiler_scopes_max];
    for (u32 i = 0; i < ctx->scopes_open; ++i) {
        neko_profiler_scope_t *scope = ctx->scopes_capture[i];

        if (scope->start == scope->end) scope->name = neko_profiler_context_add_string(ctx, scope->name, buffer_use::open);

        scopesDisplay[i] = *scope;

        // scope that was not closed, spans frame boundary
        // keep it for next frame
        if (scope->start == scope->end)
            ctx->scopes_capture[scopes_to_restart++] = scope;
        else {
            neko_profiler_free_list_free(&ctx->scopes_allocator, scope);
            scope = &scopesDisplay[i];
        }

        // did scope cross threshold?
        if (level == (s32)scope->level) {
            u64 scopeEnd = scope->end;
            if (scope->start == scope->end) scopeEnd = frameEndTime;

            if (ctx->time_threshold <= __neko_profiler_clock2ms(scopeEnd - scope->start, __neko_profiler_get_clock_frequency())) ctx->threshold_crossed = true;
        }
    }

    // did frame cross threshold ?
    f32 prevFrameTime = __neko_profiler_clock2ms(frameEndTime - frameBeginTime, __neko_profiler_get_clock_frequency());
    if ((level == -1) && (ctx->time_threshold <= prevFrameTime)) ctx->threshold_crossed = true;

    if (ctx->threshold_crossed && !ctx->pause_profiling) {
        std::swap(ctx->names_data[buffer_use::capture], ctx->names_data[buffer_use::display]);

        memcpy(ctx->scopes_display, scopesDisplay, sizeof(neko_profiler_scope_t) * ctx->scopes_open);

        ctx->display_scopes = ctx->scopes_open;
        ctx->frame_start_time = frameBeginTime;
        ctx->frame_end_time = frameEndTime;
    }

    ctx->names_size[buffer_use::capture] = 0;
    for (u32 i = 0; i < scopes_to_restart; ++i) ctx->scopes_capture[i]->name = neko_profiler_context_add_string(ctx, ctx->scopes_capture[i]->name, buffer_use::capture);

    ctx->scopes_open = scopes_to_restart;
    frameTime = frameEndTime - frameBeginTime;

    neko_mutex_unlock(&ctx->internal_mutex);
}

s32 neko_profiler_context_inc_level(neko_profiler_context_t *ctx) {
    // may be a first call on this thread
    void *tl = neko_tls_get_value(ctx->tls_level);
    if (!tl) {
        // we'd like to start with -1 but then the ++ operator below
        // would result in NULL value for tls so we offset by 2
        tl = (void *)1;
        neko_tls_set_value(ctx->tls_level, tl);
    }
    intptr_t threadLevel = (intptr_t)tl - 1;
    neko_tls_set_value(ctx->tls_level, (void *)(threadLevel + 2));
    return (s32)threadLevel;
}

void neko_profiler_context_dec_level(neko_profiler_context_t *ctx) {
    intptr_t threadLevel = (intptr_t)neko_tls_get_value(ctx->tls_level);
    --threadLevel;
    neko_tls_set_value(ctx->tls_level, (void *)threadLevel);
}

neko_profiler_scope_t *neko_profiler_context_begin_scope(neko_profiler_context_t *ctx, const_str _file, s32 _line, const_str _name) {
    neko_profiler_scope_t *scope = 0;
    {
        neko_mutex_lock(&ctx->internal_mutex);

        if (ctx->scopes_open == __neko_profiler_scopes_max) return 0;

        scope = (neko_profiler_scope_t *)neko_profiler_free_list_alloc(&ctx->scopes_allocator);
        ctx->scopes_capture[ctx->scopes_open++] = scope;

        scope->name = neko_profiler_context_add_string(ctx, _name, buffer_use::capture);
        scope->start = neko_profiler_get_clock();
        scope->end = scope->start;

        neko_mutex_unlock(&ctx->internal_mutex);
    }

    scope->thread_id = neko_get_thread_id();
    scope->file = _file;
    scope->line = _line;
    scope->level = neko_profiler_context_inc_level(ctx);

    return scope;
}

void neko_profiler_context_end_scope(neko_profiler_context_t *ctx, neko_profiler_scope_t *_scope) {
    if (!_scope) return;

    _scope->end = neko_profiler_get_clock();
    neko_profiler_context_dec_level(ctx);
}

const_str neko_profiler_context_add_string(neko_profiler_context_t *ctx, const_str _name, buffer_use _buffer) {
    char *nameData = ctx->names_data[_buffer];
    s32 &nameSize = ctx->names_size[_buffer];

    char c, *ret = &nameData[nameSize];
    while ((c = *_name++) && (nameSize < __neko_profiler_text_max)) nameData[nameSize++] = c;

    if (nameSize < __neko_profiler_text_max)
        nameData[nameSize++] = 0;
    else
        nameData[__neko_profiler_text_max - 1] = 0;

    return ret;
}

void neko_profiler_context_get_frame_data(neko_profiler_context_t *ctx, neko_profiler_frame_t *_data) {
    neko_mutex_lock(&ctx->internal_mutex);

    static neko_profiler_thread threadData[__neko_profiler_threads_max];

    u32 numThreads = (u32)ctx->thread_names.size();
    if (numThreads > __neko_profiler_threads_max) numThreads = __neko_profiler_threads_max;

    _data->num_scopes = ctx->display_scopes;
    _data->scopes = ctx->scopes_display;
    _data->num_threads = numThreads;
    _data->threads = threadData;
    _data->start_time = ctx->frame_start_time;
    _data->end_time = ctx->frame_end_time;
    _data->prev_frame_time = ctx->frame_end_time - ctx->frame_start_time;
    _data->cpu_frequency = __neko_profiler_get_clock_frequency();
    _data->time_threshold = ctx->time_threshold;
    _data->level_threshold = ctx->level_threshold;

    std::map<u64, std::string>::iterator it = ctx->thread_names.begin();
    for (u32 i = 0; i < numThreads; ++i) {
        threadData[i].thread_id = it->first;
        threadData[i].name = it->second.c_str();
        ++it;
    }

    neko_mutex_unlock(&ctx->internal_mutex);
}