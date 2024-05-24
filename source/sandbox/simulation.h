#ifndef GAME_SIMULATION_H
#define GAME_SIMULATION_H

#include <stdio.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "engine/neko.h"
#include "engine/neko.hpp"
#include "engine/neko_engine.h"
#include "game_cvar.h"
#include "helper.h"

#define window_width 1280
#define window_height 740

enum class material_type {
    EMPTY,
    ROCK,
    SAND,
    WATER,
    STEAM,
    WOOD,
    FIRE,
    GAS,
    LIQUID,
    SOLID,
    STATIC,
};

class sandbox_buffer;
class MagicPixel {
public:
    MagicPixel();
    virtual ~MagicPixel() {}
    virtual void Update(sandbox_buffer &buffer, int x, int y) {}
    Vector2 velocity_;
    neko_color_t color_;
    material_type material_;
    u32 surface_area_ = 15;
    u32 temperature_ = 0;
    u32 ignite_temperature_ = 0;
    u32 ttl_ = 0;
    int desinty_;
};

class MagicPixelFactory {
public:
    typedef std::unique_ptr<MagicPixel> (*CreateMagicPixelCallback)();
    bool RegisterMagicPixel(material_type material, CreateMagicPixelCallback fn);
    std::unique_ptr<MagicPixel> CreateMagicPixel(material_type material);
    static MagicPixelFactory *Instance() {
        if (!ptr_instance_) ptr_instance_ = new MagicPixelFactory;
        return ptr_instance_;
    }

    neko_inline const std::map<material_type, CreateMagicPixelCallback> &get_registy() const { return callback_map; }

private:
    std::map<material_type, CreateMagicPixelCallback> callback_map;
    MagicPixelFactory() = default;
    MagicPixelFactory(const MagicPixelFactory &);
    MagicPixelFactory &operator=(const MagicPixelFactory &);
    static MagicPixelFactory *ptr_instance_;
};

class Movable : virtual public MagicPixel {
public:
    enum Orientation {
        DOWN,
        UP,
        RIGHT,
        LEFT,
        DOWN_RIGHT,
        DOWN_LEFT,
        UP_RIGHT,
        UP_LEFT,
    };
    virtual int CanMove(sandbox_buffer &buffer, int x, int y) { return false; }
    bool MoveStep(sandbox_buffer &buffer, int step, int direction, int x, int y);
    bool LineCover(sandbox_buffer &buffer, Vector2 target, int x, int y, bool super = false);
    int dx_[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    int dy_[8] = {1, -1, 0, 0, 1, 1, -1, -1};

private:
};

class Fire : public MagicPixel {
private:
    static neko_color_t colors[3];
    static u32 min_temperature;
    static u32 max_temperature;
    static u32 default_ttl;

public:
    Fire();
    void Update(sandbox_buffer &buffer, int x, int y);
};

class Gas : public Movable {
protected:
    int dispersion_rate_;
    int oscillation_rate_;
    bool up_;

public:
    Gas();
    int CanMove(sandbox_buffer &buffer, int x, int y);
    void CelularAutomata(sandbox_buffer &buffer, int x, int y);
    void Update(sandbox_buffer &buffer, int x, int y);
};

class Liquid : public Movable {
protected:
    int dispersion_rate_;

public:
    Liquid();
    int CanMove(sandbox_buffer &buffer, int x, int y);
    void CelularAutomata(sandbox_buffer &buffer, int x, int y);
    void Update(sandbox_buffer &buffer, int x, int y);
};

class Solid : public Movable {
public:
    Solid();
    int CanMove(sandbox_buffer &buffer, int x, int y);
    void Update(sandbox_buffer &buffer, int x, int y);

private:
    void CelularAutomata(sandbox_buffer &buffer, int x, int y);
    void PhysicSimulation();
};

class Static : public MagicPixel {
public:
    Static();
    void Update();
};

class Rock : public Static {
public:
    Rock();
};

class Sand : public Solid {
public:
    Sand();
};

class Steam : public Gas {
public:
    Steam();
};

class Water : public Liquid {
public:
    Water();
};

class Wood : public Static {
public:
    Wood();
};

namespace {
std::unique_ptr<MagicPixel> CreateWood() { return std::make_unique<Wood>(); }
const bool registered_wood = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::WOOD, CreateWood);
std::unique_ptr<MagicPixel> CreateWater() { return std::make_unique<Water>(); }
const bool registered_water = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::WATER, CreateWater);
std::unique_ptr<MagicPixel> CreateSteam() { return std::make_unique<Steam>(); }
const bool registered_steam = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::STEAM, CreateSteam);
std::unique_ptr<MagicPixel> CreateSand() { return std::make_unique<Sand>(); }
const bool registered_sand = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::SAND, CreateSand);
std::unique_ptr<MagicPixel> CreateRock() { return std::make_unique<Rock>(); }
const bool registered_rock = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::ROCK, CreateRock);
std::unique_ptr<MagicPixel> CreateStatic() { return std::make_unique<Static>(); }
const bool registered_static = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::STATIC, CreateStatic);
std::unique_ptr<MagicPixel> CreateSolid() { return std::make_unique<Solid>(); }
const bool registered_solid = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::SOLID, CreateSolid);
std::unique_ptr<MagicPixel> CreateLiquid() { return std::make_unique<Liquid>(); }
const bool registered_liquid = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::LIQUID, CreateLiquid);
std::unique_ptr<MagicPixel> CreateGas() { return std::make_unique<Gas>(); }
const bool registered_gas = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::GAS, CreateGas);
std::unique_ptr<MagicPixel> CreateFire() { return std::make_unique<Fire>(); }
const bool registered_fire = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::FIRE, CreateFire);
}  // namespace

class sandbox_chunk;

class chunk_job {
public:
    sandbox_chunk *chunk_;
    chunk_job() {}
    chunk_job(sandbox_chunk *chunk);
};

class chunk_thread_pool {
public:
    void Start();
    void QueueJob(chunk_job &job);
    void Stop();
    bool busy();

private:
    void thread_loop();

    bool should_terminate = false;            // 线程停止
    std::mutex queue_mutex;                   // 防止数据争用到 Job 队列
    std::condition_variable mutex_condition;  // 允许线程等待新 Job 或终止
    std::vector<std::thread> threads;
    std::queue<chunk_job *> jobs;
};

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

class sandbox_object {
public:
    sandbox_object(neko_vec4_t rect);
    //    ~GameObject();
    void clean();
    void update();
    void render(f32 scale);
    void chunk_mask_update_mesh();
    neko_texture_t object_texture;
    neko_color_t *draw_buffer;
    neko_vec4_t rect;
    unsigned char *data;
    bool *edgeSeen;

private:
};

struct pixelui_t {

    // UI texture buffer
    neko_color_t *ui_buffer = {0};

    s32 brush_size;

    neko_global const s32 g_pixelui_scale = 4;
    neko_global const s32 g_pixelui_texture_width = window_width / g_pixelui_scale;
    neko_global const s32 g_pixelui_texture_height = window_height / g_pixelui_scale;

    // Frame counter
    u32 frame_counter = 0;

    u64 update_time = 0;

    bool show_material_selection_panel = true;
    bool show_frame_count = true;

    material_type material_selection = material_type::ROCK;

    neko_texture_t tex_ui = {0};
};

class sandbox_simulation;
class sandbox_game {
public:
    sandbox_game(neko_immediate_draw_t *idraw);
    ~sandbox_game();
    void pre_update();
    void update();
    void late_update();
    void render();
    void clean();
    void set_material(int material);
    void pause(int x);
    void reset_simulation(int x);

    sandbox_simulation *get_sim() const { return simulation; }

public:
    static neko_immediate_draw_t *idraw;

    f32 fbs_scale;
    f32 draw_scale;

    u32 tick_count;
    u16 draw_radius;
    material_type material;
    int count;
    bool paused = false;

private:
    void init(neko_immediate_draw_t *idraw);
    void create_simulation();
    void create_viewport();
    void create_ui();
    void reset_variables();
    sandbox_simulation *simulation;
    sandbox_object *viewport;
    pixelui_t pixelui;
};

bool update_ui(pixelui_t *pui);
void pixelui_init(pixelui_t *fallsand);
void pixelui_destroy(pixelui_t *fallsand);
void pixelui_draw(pixelui_t *pui);

#endif