//
// Created by KaoruXun on 24-5-17.
//

#ifndef NEKO_ENGINE_GAME_CVAR_H
#define NEKO_ENGINE_GAME_CVAR_H

#include <random>

#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "engine/neko_math.h"

extern u32 frame_count;
extern u32 current_tick;
extern u32 last_tick;
extern neko_color_t kEmptyPixelValue;
extern neko_vec2_t cursor;
extern thread_local std::mt19937 rng;

constexpr u32 kScreenWidth = 512;
constexpr u32 kScreenHeight = 512;
constexpr u32 kScreenWidthPower2Expoent = 9;
constexpr u32 kMaxCell = kScreenWidth * kScreenHeight;
constexpr int kMaxChunk = 64;
constexpr int kChunkBatchSize = 16;
constexpr neko_vec4_t kViewportRect = {0, 0, kScreenWidth, kScreenHeight};
constexpr u16 kMinDrawRadius = 0;
constexpr u16 kMaxDrawRadius = 75;

#endif  // NEKO_ENGINE_GAME_CVAR_H
