#ifndef GAME_SIMULATION_H
#define GAME_SIMULATION_H

#include <stdio.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <utility>

#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "sandbox/game_cvar.h"
#include "sandbox/helper.h"
#include "sandbox/magic_pixel.h"
#include "sandbox/thread_pool.h"

class sandbox_buffer;
class Cell {
public:
    Cell() = default;
    bool Empty();
    void PreUpdate();
    void SetUpdateFlag();
    void Update(sandbox_buffer &buffer);
    void TransferHeat(int temperature);
    void CreateMagicPixel(material_type material);
    void ReplacMagicPixel(material_type material);
    void RemoveMagicPixel();
    material_type GetMaterial();
    void Burn(material_type material, int base_ttl);
    std::unique_ptr<MagicPixel> magic_pixel_ptr_;
    bool update_;
    int x_, y_;
};

class sandbox_buffer {
public:
    sandbox_buffer();
    neko_color_t GetCellColor(int x, int y);
    material_type GetMaterial(int x, int y);
    bool IsCellEmpty(int x, int y);
    bool IsExpired(int x, int y);
    bool Ignites(int x, int y, int temperature);
    void CreateMagicPixel(material_type material, int x, int y);
    void ReplacMagicPixel(material_type material, int x, int y);
    void SetTemperature(int x, int y, int temperature);
    void RemoveMagicPixel(int x, int y);
    void SwapCell(int x, int y, int a, int b);
    void MoveCell(int x, int y, int a, int b);
    void Reset();
    std::array<std::array<Cell, g_sim_chunk_height>, g_sim_chunk_width> buffer_;

private:
    //    static void Create(){
    //
    //    }
    //    static void Swap(MagicPixel *a, MagicPixel *b){
    //        a->UpdateIndex(b->index_);
    //        b->UpdateIndex(a->index_);
    //        std::swap(a,b);
    //    }
    //    static void Destroy(MagicPixel *a){
    //        delete a;
    //        a = nullptr;
    //    }
    //    static void Mutate(MagicPixel *a, MaterialType material){
    //        Destroy(a);
    //    }
};

class sandbox_chunk {
public:
    sandbox_chunk() = default;
    void Init(int x, int y, std::shared_ptr<std::array<sandbox_chunk, kMaxChunk> > chunks, std::shared_ptr<sandbox_buffer> buffer);
    void draw_debug(neko_immediate_draw_t *idraw, float scale);
    void GetCurrentDirtyRect(int &max_x, int &max_y, int &min_x, int &min_y);
    void Update();
    void UpdateRect(int x, int y);
    void AddCell(material_type material, int x, int y);
    void RemoveCell(int x, int y);
    void ResetRect();
    std::shared_ptr<sandbox_buffer> buffer_ptr_;
    std::shared_ptr<std::array<sandbox_chunk, kMaxChunk> > chunks_;
    std::vector<Vector2> active_;
    u32 last_frame_;
    int dirty_rect_min_x_, dirty_rect_min_y_;
    int dirty_rect_max_x_, dirty_rect_max_y_;
    int min_x_, min_y_;
    int max_x_, max_y_;
    int live_pixel_;
};

class sandbox_simulation {
public:
    sandbox_simulation();
    ~sandbox_simulation();
    void update();
    void playground();
    void reset();
    void SetCellInsideCircle(neko_vec2_t point, u16 rad, material_type material, bool physics = false);
    void SetCell(neko_vec2_t point, material_type material, bool physics = false);
    std::shared_ptr<sandbox_buffer> buffer_ptr_;
    std::shared_ptr<std::array<sandbox_chunk, kMaxChunk> > chunks_ptr_;

private:
    //    neko::thread_pool pool{12};
    chunk_thread_pool pool_;
    chunk_job *jobs_;
    void process_chunk(int *chunks, u16 num);
};

#endif