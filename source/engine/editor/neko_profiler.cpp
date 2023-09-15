
#include "neko_profiler.hpp"

// #include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "engine/utility/logger.hpp"
#include "libs/lz4/lz4.h"

namespace neko {

u64 __neko_profiler_get_clock_frequency();

neko::neko_profiler_context *g_profiler_context = 0;

void neko_profiler_init() { g_profiler_context = new neko::neko_profiler_context(); }

void neko_profiler_shutdown() {
    delete g_profiler_context;
    g_profiler_context = 0;
}

void neko_profiler_set_threshold(f32 _ms, s32 _level) { g_profiler_context->set_threshold(_ms, _level); }

void neko_profiler_register_thread(const_str _name, u64 _threadID) {
    if (_threadID == 0) _threadID = neko_get_thread_id();

    g_profiler_context->register_thread(_threadID, _name);
}

void neko_profiler_unregister_thread(u64 _threadID) { g_profiler_context->unregister_thread(_threadID); }

void neko_profiler_begin_frame() { g_profiler_context->begin_frame(); }

uintptr_t neko_profiler_begin_scope(const_str _file, s32 _line, const_str _name) { return (uintptr_t)g_profiler_context->begin_ccope(_file, _line, _name); }

void neko_profiler_end_scope(uintptr_t _scopeHandle) { g_profiler_context->end_scope((profiler_scope *)_scopeHandle); }

s32 neko_profiler_is_paused() { return g_profiler_context->is_paused() ? 1 : 0; }

s32 neko_profiler_was_threshold_crossed() { return g_profiler_context->was_threshold_crossed() ? 1 : 0; }

void neko_profiler_set_paused(s32 _paused) { return g_profiler_context->set_paused(_paused != 0); }

void neko_profiler_get_frame(profiler_frame *_data) {
    g_profiler_context->get_frame_data(_data);

    // clamp scopes crossing frame boundary
    const u32 numScopes = _data->num_scopes;
    for (u32 i = 0; i < numScopes; ++i) {
        profiler_scope &cs = _data->scopes[i];

        if (cs.start == cs.end) {
            cs.end = _data->end_time;
            if (cs.start < _data->start_time) cs.start = _data->start_time;
        }
    }
}

s32 neko_profiler_save(profiler_frame *_data, void *_buffer, size_t _bufferSize) {
    // fill string data
    string_store str_store;
    for (u32 i = 0; i < _data->num_scopes; ++i) {
        profiler_scope &scope = _data->scopes[i];
        str_store.add_string(scope.name);
        str_store.add_string(scope.file);
    }
    for (u32 i = 0; i < _data->num_threads; ++i) {
        str_store.add_string(_data->threads[i].name);
    }

    // calc data size
    u32 totalSize = _data->num_scopes * sizeof(profiler_scope) + _data->num_threads * sizeof(neko_profiler_thread) + sizeof(profiler_frame) + str_store.total_size;

    u8 *buffer = new u8[totalSize];
    u8 *bufPtr = buffer;

    write_var(buffer, _data->start_time);
    write_var(buffer, _data->end_time);
    write_var(buffer, _data->prev_frame_time);
    // writeVar(buffer, _data->m_platformID);
    write_var(buffer, __neko_profiler_get_clock_frequency());

    // write scopes
    write_var(buffer, _data->num_scopes);
    for (u32 i = 0; i < _data->num_scopes; ++i) {
        profiler_scope &scope = _data->scopes[i];
        write_var(buffer, scope.start);
        write_var(buffer, scope.end);
        write_var(buffer, scope.thread_id);
        write_var(buffer, str_store.get_string(scope.name));
        write_var(buffer, str_store.get_string(scope.file));
        write_var(buffer, scope.line);
        write_var(buffer, scope.level);
    }

    // write thread info
    write_var(buffer, _data->num_threads);
    for (u32 i = 0; i < _data->num_threads; ++i) {
        neko_profiler_thread &t = _data->threads[i];
        write_var(buffer, t.thread_id);
        write_var(buffer, str_store.get_string(t.name));
    }

    // write string data
    u32 numStrings = (u32)str_store.strings.size();
    write_var(buffer, numStrings);

    for (u32 i = 0; i < str_store.strings.size(); ++i) write_str(buffer, str_store.strings[i].c_str());

    s32 compSize = LZ4_compress_default((const_str)bufPtr, (char *)_buffer, (s32)(buffer - bufPtr), (s32)_bufferSize);
    delete[] bufPtr;
    return compSize;
}

void neko_profiler_load(profiler_frame *_data, void *_buffer, size_t _bufferSize) {
    size_t bufferSize = _bufferSize;
    u8 *buffer = 0;
    u8 *bufferPtr;

    s32 decomp = -1;
    do {
        delete[] buffer;
        bufferSize *= 2;
        buffer = new u8[bufferSize];
        decomp = LZ4_decompress_safe((const_str)_buffer, (char *)buffer, (s32)_bufferSize, (s32)bufferSize);

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

    _data->scopes = new profiler_scope[_data->num_scopes * 2];  // extra space for viewer - m_scopesStats
    _data->scopes_stats = &_data->scopes[_data->num_scopes];
    _data->scope_stats_info = new neko_profiler_scope_stats[_data->num_scopes * 2];

    for (u32 i = 0; i < _data->num_scopes * 2; ++i) _data->scopes[i].stats = &_data->scope_stats_info[i];

    for (u32 i = 0; i < _data->num_scopes; ++i) {
        profiler_scope &scope = _data->scopes[i];
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
        profiler_scope &scope = _data->scopes[i];
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

    delete[] bufferPtr;

    // process frame data

    for (u32 i = 0; i < _data->num_scopes; ++i)
        for (u32 j = 0; j < _data->num_scopes; ++j) {
            profiler_scope &scopeI = _data->scopes[i];
            profiler_scope &scopeJ = _data->scopes[j];

            if ((scopeJ.start > scopeI.start) && (scopeJ.end < scopeI.end) && (scopeJ.level == scopeI.level + 1) && (scopeJ.thread_id == scopeI.thread_id))
                scopeI.stats->exclusive_time -= scopeJ.stats->inclusive_time;
        }

    _data->num_scopes_stats = 0;

    for (u32 i = 0; i < _data->num_scopes; ++i) {
        profiler_scope &scopeI = _data->scopes[i];

        scopeI.stats->inclusive_time_total = scopeI.stats->inclusive_time;
        scopeI.stats->exclusive_time_total = scopeI.stats->exclusive_time;

        s32 foundIndex = -1;
        for (u32 j = 0; j < _data->num_scopes_stats; ++j) {
            profiler_scope &scopeJ = _data->scopes_stats[j];
            if (strcmp(scopeI.name, scopeJ.name) == 0) {
                foundIndex = j;
                break;
            }
        }

        if (foundIndex == -1) {
            s32 index = _data->num_scopes_stats++;
            profiler_scope &scope = _data->scopes_stats[index];
            scope = scopeI;
            scope.stats->occurences = 1;
        } else {
            profiler_scope &scope = _data->scopes_stats[foundIndex];
            scope.stats->inclusive_time_total += scopeI.stats->inclusive_time;
            scope.stats->exclusive_time_total += scopeI.stats->exclusive_time;
            scope.stats->occurences++;
        }
    }
}

void neko_profiler_load_time_only(f32 *_time, void *_buffer, size_t _bufferSize) {
    size_t bufferSize = _bufferSize;
    u8 *buffer = 0;
    u8 *bufferPtr;

    s32 decomp = -1;
    do {
        delete[] buffer;
        bufferSize *= 2;
        buffer = new u8[bufferSize];
        decomp = LZ4_decompress_safe((const_str)_buffer, (char *)buffer, (s32)_bufferSize, (s32)bufferSize);

    } while (decomp < 0);

    bufferPtr = buffer;

    u64 startTime;
    u64 endtime;
    u8 dummy8;
    u64 frequency;

    read_var(buffer, startTime);
    read_var(buffer, endtime);
    read_var(buffer, frequency);  // dummy
    read_var(buffer, dummy8);     // dummy
    read_var(buffer, frequency);
    *_time = __neko_profiler_clock2ms(endtime - startTime, frequency);

    delete[] buffer;
}

void neko_profiler_release(profiler_frame *_data) {
    for (u32 i = 0; i < _data->num_scopes; ++i) {
        profiler_scope &scope = _data->scopes[i];
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
#if neko_is_debug()
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

neko_profiler_context::neko_profiler_context()
    : scopes_open(0), display_scopes(0), frame_start_time(0), frame_end_time(0), threshold_crossed(false), time_threshold(0.0f), level_threshold(0), pause_profiling(false) {

    tls_level = neko_tls_allocate();

    neko_profiler_free_list_create(sizeof(profiler_scope), __neko_profiler_scopes_max, &scopes_allocator);

    for (s32 i = 0; i < buffer_use::Count; ++i) {
        names_size[i] = 0;
        names_data[i] = names_data_buffers[i];
    }
}

neko_profiler_context::~neko_profiler_context() {
    neko_profiler_free_list_destroy(&scopes_allocator);
    neko_tls_free(tls_level);
}

void neko_profiler_context::set_threshold(f32 _ms, s32 _levelThreshold) {
    time_threshold = _ms;
    level_threshold = _levelThreshold;
}

bool neko_profiler_context::is_paused() { return pause_profiling; }

bool neko_profiler_context::was_threshold_crossed() { return !pause_profiling && threshold_crossed; }

void neko_profiler_context::set_paused(bool _paused) { pause_profiling = _paused; }

void neko_profiler_context::register_thread(u64 _threadID, const_str _name) { thread_names[_threadID] = _name; }

void neko_profiler_context::unregister_thread(u64 _threadID) { thread_names.erase(_threadID); }

void neko_profiler_context::begin_frame() {
    neko_scoped_mutex_locker lock(mutex);

    u64 frameBeginTime, frameEndTime;
    static u64 beginPrevFrameTime = neko_profiler_get_clock();
    frameBeginTime = beginPrevFrameTime;
    frameEndTime = neko_profiler_get_clock();
    beginPrevFrameTime = frameEndTime;

    static u64 frameTime = frameEndTime - frameBeginTime;

    threshold_crossed = false;

    s32 level = (s32)level_threshold - 1;

    u32 scopesToRestart = 0;

    names_size[buffer_use::Open] = 0;

    static profiler_scope scopesDisplay[__neko_profiler_scopes_max];
    for (u32 i = 0; i < scopes_open; ++i) {
        profiler_scope *scope = scopes_capture[i];

        if (scope->start == scope->end) scope->name = add_string(scope->name, buffer_use::Open);

        scopesDisplay[i] = *scope;

        // scope that was not closed, spans frame boundary
        // keep it for next frame
        if (scope->start == scope->end)
            scopes_capture[scopesToRestart++] = scope;
        else {
            neko_profiler_free_list_free(&scopes_allocator, scope);
            scope = &scopesDisplay[i];
        }

        // did scope cross threshold?
        if (level == (s32)scope->level) {
            u64 scopeEnd = scope->end;
            if (scope->start == scope->end) scopeEnd = frameEndTime;

            if (time_threshold <= __neko_profiler_clock2ms(scopeEnd - scope->start, __neko_profiler_get_clock_frequency())) threshold_crossed = true;
        }
    }

    // did frame cross threshold ?
    f32 prevFrameTime = __neko_profiler_clock2ms(frameEndTime - frameBeginTime, __neko_profiler_get_clock_frequency());
    if ((level == -1) && (time_threshold <= prevFrameTime)) threshold_crossed = true;

    if (threshold_crossed && !pause_profiling) {
        std::swap(names_data[buffer_use::Capture], names_data[buffer_use::Display]);

        memcpy(scopes_display, scopesDisplay, sizeof(profiler_scope) * scopes_open);

        display_scopes = scopes_open;
        frame_start_time = frameBeginTime;
        frame_end_time = frameEndTime;
    }

    names_size[buffer_use::Capture] = 0;
    for (u32 i = 0; i < scopesToRestart; ++i) scopes_capture[i]->name = add_string(scopes_capture[i]->name, buffer_use::Capture);

    scopes_open = scopesToRestart;
    frameTime = frameEndTime - frameBeginTime;
}

s32 neko_profiler_context::inc_level() {
    // may be a first call on this thread
    void *tl = neko_tls_get_value(tls_level);
    if (!tl) {
        // we'd like to start with -1 but then the ++ operator below
        // would result in NULL value for tls so we offset by 2
        tl = (void *)1;
        neko_tls_set_value(tls_level, tl);
    }
    intptr_t threadLevel = (intptr_t)tl - 1;
    neko_tls_set_value(tls_level, (void *)(threadLevel + 2));
    return (s32)threadLevel;
}

void neko_profiler_context::dec_level() {
    intptr_t threadLevel = (intptr_t)neko_tls_get_value(tls_level);
    --threadLevel;
    neko_tls_set_value(tls_level, (void *)threadLevel);
}

profiler_scope *neko_profiler_context::begin_ccope(const_str _file, s32 _line, const_str _name) {
    profiler_scope *scope = 0;
    {
        neko_scoped_mutex_locker lock(mutex);
        if (scopes_open == __neko_profiler_scopes_max) return 0;

        scope = (profiler_scope *)neko_profiler_free_list_alloc(&scopes_allocator);
        scopes_capture[scopes_open++] = scope;

        scope->name = add_string(_name, buffer_use::Capture);
        scope->start = neko_profiler_get_clock();
        scope->end = scope->start;
    }

    scope->thread_id = neko_get_thread_id();
    scope->file = _file;
    scope->line = _line;
    scope->level = inc_level();

    return scope;
}

void neko_profiler_context::end_scope(profiler_scope *_scope) {
    if (!_scope) return;

    _scope->end = neko_profiler_get_clock();
    dec_level();
}

const_str neko_profiler_context::add_string(const_str _name, buffer_use _buffer) {
    char *nameData = names_data[_buffer];
    s32 &nameSize = names_size[_buffer];

    char c, *ret = &nameData[nameSize];
    while ((c = *_name++) && (nameSize < __neko_profiler_text_max)) nameData[nameSize++] = c;

    if (nameSize < __neko_profiler_text_max)
        nameData[nameSize++] = 0;
    else
        nameData[__neko_profiler_text_max - 1] = 0;

    return ret;
}

void neko_profiler_context::get_frame_data(profiler_frame *_data) {
    neko_scoped_mutex_locker lock(mutex);

    static neko_profiler_thread threadData[__neko_profiler_threads_max];

    u32 numThreads = (u32)thread_names.size();
    if (numThreads > __neko_profiler_threads_max) numThreads = __neko_profiler_threads_max;

    _data->num_scopes = display_scopes;
    _data->scopes = scopes_display;
    _data->num_threads = numThreads;
    _data->threads = threadData;
    _data->start_time = frame_start_time;
    _data->end_time = frame_end_time;
    _data->prev_frame_time = frame_end_time - frame_start_time;
    _data->cpu_frequency = __neko_profiler_get_clock_frequency();
    _data->time_threshold = time_threshold;
    _data->level_threshold = level_threshold;

    std::map<u64, std::string>::iterator it = thread_names.begin();
    for (u32 i = 0; i < numThreads; ++i) {
        threadData[i].thread_id = it->first;
        threadData[i].name = it->second.c_str();
        ++it;
    }
}

}  // namespace neko