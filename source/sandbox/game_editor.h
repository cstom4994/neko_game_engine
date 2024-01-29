#ifndef GAME_EDITOR_H
#define GAME_EDITOR_H

#include "engine/neko_profiler.h"

#define __neko_desired_frame_rate 30.0f
#define __neko_minimum_frame_rate 20.0f
#define __neko_flash_time_in_ms 333.0
struct pan_and_zoon {
    f32 offset;
    f32 start_pan;
    f32 zoom;

    pan_and_zoon() {
        offset = 0.0f;
        start_pan = 0.0f;
        zoom = 1.0f;
    }

    inline f32 w2s(f32 wld, f32 minX, f32 maxX) { return minX + wld * (maxX - minX) * zoom - offset; }
    inline f32 w2sdelta(f32 wld, f32 minX, f32 maxX) { return wld * (maxX - minX) * zoom; }
    inline f32 s2w(f32 scr, f32 minX, f32 maxX) { return (scr + offset - minX) / ((maxX - minX) * zoom); }
};

struct frame_info {
    f32 time;
    u32 offset;
    u32 size;
};

int profiler_draw_frame(profiler_frame* _data, void* _buffer = 0, size_t _bufferSize = 0, bool _inGame = true, bool _multi = false);
void profiler_draw_stats(profiler_frame* _data, bool _multi = false);

#endif