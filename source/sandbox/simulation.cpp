#include "simulation.h"

#include <list>

#include "game_main.h"
#include "game_physics_math.hpp"
#include "helper.h"
#include "neko_api.h"

#define NEKO_BUILTIN_IMPL
#include "../old/neko_builtin_font.h"

u32 frame_count = 0;
u32 current_tick = 0;
u32 last_tick = 0;
neko_color_t kEmptyPixelValue = neko_color(0, 0, 0, 0);
neko_vec2_t cursor = {-1, -1};

thread_local std::mt19937 rng;

MagicPixel::MagicPixel() {}

MagicPixelFactory *MagicPixelFactory::ptr_instance_ = nullptr;

bool MagicPixelFactory::RegisterMagicPixel(material_type material, CreateMagicPixelCallback fn) {
    callback_map[material] = fn;
    return true;
}
std::unique_ptr<MagicPixel> MagicPixelFactory::CreateMagicPixel(material_type material) { return callback_map[material](); }

bool Movable::MoveStep(sandbox_buffer &buffer, int step, int direction, int x, int y) {
    int a = x, b = y;
    int next_x = x, next_y = y;
    for (int i = 0; i < step; i++) {
        a += dx_[direction];
        b += dy_[direction];
        if (a < 0 || b < 0 || a >= g_sim_chunk_width || b >= g_sim_chunk_height) break;
        int can_move = CanMove(buffer, a, b);
        if (can_move == 1) {
            next_x = a;
            next_y = b;
            continue;
        } else if (!can_move) {
            break;
        }
    }
    if (next_x != x || next_y != y) {
        buffer.MoveCell(x, y, next_x, next_y);
        return true;
    }
    return false;
}

bool Movable::LineCover(sandbox_buffer &buffer, Vector2 target, int x, int y, bool super) {
    //    bool swap = false;
    //    int x = position_.x_, y = position_.y_;
    //    int dx = target.x_-position_.x_, dy = target.y_-position_.y_;
    //    auto cmp = [](int a,int b) { return (a < b)?-1:1; };
    //    if(cmp(abs(dx),abs(dy)) < 0){
    //        swap = true;
    //        std::swap(x,y);
    //        std::swap(dx,dy);
    //    }
    //    int x_step = cmp(0,dx), y_step = cmp(0,dy);
    //    dx = abs(dx);
    //    dy = abs(dy);
    //    int ddx = 2*dx, ddy = 2*dy;
    //    int errorprev = dy, error = dy;
    //    Vector2 ans = position_;
    //    for(int i = 0;i<dy;i++){
    //        y += y_step;
    //        error += ddx;
    //        if(error > ddy){
    //            x += x_step;
    //            error -= ddy;
    //            if(super){
    //                if(error + errorprev <= ddy){
    //                    Vector2(x-x_step,y);
    //                }
    //                if(error + errorprev >= ddy){
    //                    Vector2(x,y-y_step);
    //                }
    //            }
    //        }
    //        int bar = Helper::GetIndex(x, y);
    //        if(CanMove(bar)){
    //            ans = Vector2(x,y);
    //            last_frame_ = frame_count;
    //        }
    //    }
    //    if(swap) std::swap(ans.x_,ans.y_);
    //    return ans;
    return true;
}

neko_color_t Fire::colors[3] = {neko_color(215, 53, 2, 255), neko_color(252, 100, 0, 255), neko_color(250, 192, 0, 255)};
u32 Fire::min_temperature = 250;
u32 Fire::max_temperature = 500;
u32 Fire::default_ttl = 100;

Fire::Fire() {
    ttl_ = current_tick + default_ttl + Random::IntOnInterval(0, 20);
    material_ = material_type::FIRE;
    temperature_ = min_temperature;
    color_ = neko_color_interpolate(colors[0], colors[2], Random::DoubleOnInterval(0.0, 1));
}

void Fire::Update(sandbox_buffer &buffer, int x, int y) {
    temperature_ = std::min(temperature_ + 2, max_temperature);
    for (int i = 0; i < 8; i++) {
        int a = x + Navigation::dx[i], b = y + Navigation::dy[i];
        if (a < 0 || b < 0 || a >= g_sim_chunk_width || b >= g_sim_chunk_height) continue;
        if (buffer.buffer_[a][b].Empty()) {
            int should_emit = Random::IntOnInterval(0, 10);
            if (should_emit <= 1) {
                buffer.buffer_[a][b].CreateMagicPixel(material_type::GAS);
                buffer.buffer_[a][b].magic_pixel_ptr_->color_ = color_;
                buffer.buffer_[a][b].magic_pixel_ptr_->ttl_ = current_tick + 20;
            } else if (should_emit == 3) {
                buffer.buffer_[a][b].CreateMagicPixel(material_type::GAS);
                buffer.buffer_[a][b].magic_pixel_ptr_->ttl_ = current_tick + 100;
            }
        } else if (buffer.buffer_[a][b].GetMaterial() != material_type::FIRE) {
            buffer.buffer_[a][b].TransferHeat(temperature_);
            buffer.buffer_[a][b].Burn(material_type::FIRE, default_ttl);
        }
    }
    if (!Random::IntOnInterval(0, 3)) {
        color_ = neko_color_interpolate(colors[0], colors[2], Random::DoubleOnInterval(0.0, 1));
    }
    buffer.buffer_[x][y].SetUpdateFlag();
}

Gas::Gas() {
    color_ = neko_color(128, 128, 128, 128);
    material_ = material_type::GAS;
    up_ = true;
    dispersion_rate_ = 2;
    oscillation_rate_ = 1;
}

int Gas::CanMove(sandbox_buffer &buffer, int x, int y) {
    if (buffer.IsCellEmpty(x, y)) {
        return 1;
    } else if (buffer.buffer_[x][y].magic_pixel_ptr_->material_ == material_) {
        return -1;
    }
    return 0;
}

void Gas::CelularAutomata(sandbox_buffer &buffer, int x, int y) {
    if (up_ && MoveStep(buffer, dispersion_rate_, Orientation::UP, x, y)) return;
    if (oscillation_rate_ && MoveStep(buffer, oscillation_rate_, Orientation::DOWN, x, y)) return;
    if (Random::CoinToss()) {
        if (MoveStep(buffer, 1, Orientation::UP_RIGHT, x, y)) return;
        if (MoveStep(buffer, 1, Orientation::UP_LEFT, x, y)) return;
    } else {
        if (MoveStep(buffer, 1, Orientation::UP_LEFT, x, y)) return;
        if (MoveStep(buffer, 1, Orientation::UP_RIGHT, x, y)) return;
    }
}

void Gas::Update(sandbox_buffer &buffer, int x, int y) {
    CelularAutomata(buffer, x, y);
    up_ = !up_;
}

Liquid::Liquid() {
    color_ = neko_color(128, 128, 128, 180);
    material_ = material_type::LIQUID;
    dispersion_rate_ = 1;
}

int Liquid::CanMove(sandbox_buffer &buffer, int x, int y) { return buffer.IsCellEmpty(x, y); }

void Liquid::CelularAutomata(sandbox_buffer &buffer, int x, int y) {
    if (MoveStep(buffer, Random::IntOnInterval(1, 3), Orientation::DOWN, x, y)) return;
    if (Random::CoinToss()) {
        if (MoveStep(buffer, 1, Orientation::DOWN_RIGHT, x, y)) return;
        if (MoveStep(buffer, 1, Orientation::DOWN_LEFT, x, y)) return;
    } else {
        if (MoveStep(buffer, 1, Orientation::DOWN_LEFT, x, y)) return;
        if (MoveStep(buffer, 1, Orientation::DOWN_RIGHT, x, y)) return;
    }
    if (Random::CoinToss()) {
        if (MoveStep(buffer, dispersion_rate_, Orientation::RIGHT, x, y)) return;
        if (MoveStep(buffer, dispersion_rate_, Orientation::LEFT, x, y)) return;
    } else {
        if (MoveStep(buffer, dispersion_rate_, Orientation::LEFT, x, y)) return;
        if (MoveStep(buffer, dispersion_rate_, Orientation::RIGHT, x, y)) return;
    }
}

void Liquid::Update(sandbox_buffer &buffer, int x, int y) { CelularAutomata(buffer, x, y); }

Solid::Solid() {
    color_ = neko_color_interpolate(neko_color(128, 128, 128, 255), NEKO_COLOR_BLACK, Random::DoubleOnInterval(0.0, 0.15));
    velocity_ = Vector2(0, 0);
    material_ = material_type::SOLID;
}

int Solid::CanMove(sandbox_buffer &buffer, int x, int y) { return buffer.IsCellEmpty(x, y); }

void Solid::CelularAutomata(sandbox_buffer &buffer, int x, int y) {
    if (MoveStep(buffer, Random::IntOnInterval(1, 3), Orientation::DOWN, x, y)) return;
    if (Random::CoinToss()) {
        if (MoveStep(buffer, 1, Orientation::DOWN_RIGHT, x, y)) return;
        if (MoveStep(buffer, 1, Orientation::DOWN_LEFT, x, y)) return;
    } else {
        if (MoveStep(buffer, 1, Orientation::DOWN_LEFT, x, y)) return;
        if (MoveStep(buffer, 1, Orientation::DOWN_RIGHT, x, y)) return;
    }
}

void Solid::PhysicSimulation() {}

void Solid::Update(sandbox_buffer &buffer, int x, int y) {
    if (velocity_.x_ == 0 && velocity_.y_ == 0) {
        CelularAutomata(buffer, x, y);
    } else {
        PhysicSimulation();
    }
}

Static::Static() {
    color_ = neko_color(128, 128, 128, 255);
    if (Random::CoinToss()) color_ = neko_color_interpolate(color_, NEKO_COLOR_WHITE, Random::DoubleOnInterval(0, 0.15));
}

void Static::Update() {}

Rock::Rock() {
    color_ = neko_color(58, 50, 50, 255);
    if (Random::CoinToss()) color_ = neko_color_interpolate(color_, NEKO_COLOR_WHITE, Random::DoubleOnInterval(0.0, 0.15));
    material_ = material_type::ROCK;
}

Sand::Sand() {
    color_ = neko_color_interpolate(neko_color(255, 207, 92, 255), NEKO_COLOR_BLACK, Random::DoubleOnInterval(0.0, 0.15));
    material_ = material_type::SAND;
}

Steam::Steam() {
    color_ = neko_color(201, 208, 255, 128);
    ttl_ = current_tick + Random::IntOnInterval(2000, 50000);
    material_ = material_type::STEAM;
    up_ = true;
}

Water::Water() {
    color_ = neko_color(131, 215, 238, 180);
    material_ = material_type::WATER;
    dispersion_rate_ = 5;
}

Wood::Wood() {
    color_ = neko_color(106, 75, 63, 255);
    ignite_temperature_ = 250;
    surface_area_ = 80;
    material_ = material_type::WOOD;
    if (Random::CoinToss()) color_ = neko_color_interpolate(color_, NEKO_COLOR_BLACK, Random::DoubleOnInterval(0.0, 0.4));
}

chunk_job::chunk_job(sandbox_chunk *chunk) { chunk_ = chunk; }

void chunk_thread_pool::Start() {
    const uint32_t num_threads = std::thread::hardware_concurrency();  // Max # of threads the system supports
    threads.resize(num_threads);
    for (uint32_t i = 0; i < num_threads; i++) {
        threads.at(i) = std::thread(&chunk_thread_pool::thread_loop, this);
    }
}

void chunk_thread_pool::thread_loop() {
    rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    while (true) {
        chunk_job *job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] { return !jobs.empty() || should_terminate; });
            if (should_terminate) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job->chunk_->Update();
    }
}

void chunk_thread_pool::QueueJob(chunk_job &job) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(&job);
    }
    mutex_condition.notify_one();
}

bool chunk_thread_pool::busy() {
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = jobs.empty();
    }
    return poolbusy;
}

void chunk_thread_pool::Stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread &active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}

sandbox_buffer::sandbox_buffer() {
    for (int i = 0; i < buffer_.size(); i++) {
        for (int j = 0; j < buffer_[i].size(); j++) {
            buffer_[i][j].x_ = i;
            buffer_[i][j].y_ = j;
            buffer_[i][j].update_ = false;
        }
    }
}

neko_color_t sandbox_buffer::GetCellColor(int x, int y) { return IsCellEmpty(x, y) ? kEmptyPixelValue : buffer_[x][y].magic_pixel_ptr_->color_; }

material_type sandbox_buffer::GetMaterial(int x, int y) {
    if (buffer_[x][y].magic_pixel_ptr_ == nullptr) return material_type::EMPTY;
    return buffer_[x][y].magic_pixel_ptr_->material_;
}

bool sandbox_buffer::IsCellEmpty(int x, int y) { return buffer_[x][y].magic_pixel_ptr_ == nullptr; }

bool sandbox_buffer::IsExpired(int x, int y) { return (buffer_[x][y].magic_pixel_ptr_ != nullptr) && (buffer_[x][y].magic_pixel_ptr_->ttl_) && (buffer_[x][y].magic_pixel_ptr_->ttl_ <= current_tick); }

bool sandbox_buffer::Ignites(int x, int y, int temperature) {
    return !IsCellEmpty(x, y) && (buffer_[x][y].magic_pixel_ptr_->ignite_temperature_) && (buffer_[x][y].magic_pixel_ptr_->ignite_temperature_ <= temperature);
}

void sandbox_buffer::CreateMagicPixel(material_type material, int x, int y) {
    buffer_[x][y].magic_pixel_ptr_ = MagicPixelFactory::Instance()->CreateMagicPixel(material);
    buffer_[x][y].update_ = true;
}

void sandbox_buffer::ReplacMagicPixel(material_type material, int x, int y) {
    RemoveMagicPixel(x, y);
    CreateMagicPixel(material, x, y);
}

void sandbox_buffer::RemoveMagicPixel(int x, int y) {
    buffer_[x][y].magic_pixel_ptr_.reset();
    buffer_[x][y].update_ = true;
}

void sandbox_buffer::SwapCell(int x, int y, int a, int b) {}

void sandbox_buffer::MoveCell(int x, int y, int a, int b) {
    if (!IsCellEmpty(a, b)) {
        SwapCell(x, y, a, b);
    } else {
        buffer_[a][b].magic_pixel_ptr_ = std::move(buffer_[x][y].magic_pixel_ptr_);
        buffer_[x][y].update_ = true;
        buffer_[a][b].update_ = true;
    }
}

void sandbox_buffer::Reset() {
    for (int i = 0; i < buffer_.size(); i++) {
        for (int j = 0; j < buffer_[i].size(); j++) {
            buffer_[i][j].magic_pixel_ptr_.reset();
            buffer_[i][j].update_ = false;
        }
    }
}

bool Cell::Empty() { return magic_pixel_ptr_ == nullptr; }

void Cell::PreUpdate() {
    if (Empty()) return;
    if (magic_pixel_ptr_->ttl_ && magic_pixel_ptr_->ttl_ <= current_tick) {
    }
}

void Cell::Update(sandbox_buffer &buffer) {
    update_ = false;
    if (magic_pixel_ptr_ == nullptr) return;
    magic_pixel_ptr_->Update(buffer, x_, y_);
}

void Cell::SetUpdateFlag() { update_ = true; }

void Cell::TransferHeat(int temperature) { magic_pixel_ptr_->temperature_ += (int)(temperature * Random::DoubleOnInterval(0.01, 0.02)); }

void Cell::CreateMagicPixel(material_type material) {
    magic_pixel_ptr_ = MagicPixelFactory::Instance()->CreateMagicPixel(material);
    update_ = true;
}

void Cell::ReplacMagicPixel(material_type material) {
    RemoveMagicPixel();
    CreateMagicPixel(material);
}

void Cell::RemoveMagicPixel() {
    magic_pixel_ptr_.reset();
    update_ = true;
}

material_type Cell::GetMaterial() {
    if (magic_pixel_ptr_ == nullptr) return material_type::EMPTY;
    return magic_pixel_ptr_->material_;
}

void Cell::Burn(material_type material, int base_ttl) {
    if (magic_pixel_ptr_->ignite_temperature_ && magic_pixel_ptr_->ignite_temperature_ <= magic_pixel_ptr_->temperature_) {
        u32 surface_area = magic_pixel_ptr_->surface_area_;
        ReplacMagicPixel(material);
        magic_pixel_ptr_->ttl_ = current_tick + base_ttl * surface_area + Random::IntOnInterval(0, 1000);
    }
}

void sandbox_chunk::Init(int x, int y, std::shared_ptr<std::array<sandbox_chunk, kMaxChunk>> chunks, std::shared_ptr<sandbox_buffer> buffer) {
    buffer_ptr_ = buffer;
    chunks_ = chunks;
    min_x_ = x;
    min_y_ = y;
    max_x_ = min_x_ + kMaxChunk - 1;
    max_y_ = min_y_ + kMaxChunk - 1;
    last_frame_ = 0;
    live_pixel_ = 0;
    ResetRect();
}

void sandbox_chunk::UpdateRect(int x, int y) {
    dirty_rect_min_x_ = std::min(x, dirty_rect_min_x_);
    dirty_rect_min_y_ = std::min(y, dirty_rect_min_y_);
    dirty_rect_max_x_ = std::max(x, dirty_rect_max_x_);
    dirty_rect_max_y_ = std::max(y, dirty_rect_max_y_);
}

void sandbox_chunk::GetCurrentDirtyRect(int &max_x, int &max_y, int &min_x, int &min_y) {
    min_x = std::clamp(dirty_rect_min_x_ - 1, 0, (s32)g_sim_chunk_width - 1);
    min_y = std::clamp(dirty_rect_min_y_ - 1, 0, (s32)g_sim_chunk_height - 1);
    max_x = std::clamp(dirty_rect_max_x_ + 1, 0, (s32)g_sim_chunk_width - 1);
    max_y = std::clamp(dirty_rect_max_y_ + 1, 0, (s32)g_sim_chunk_height - 1);
}

void sandbox_chunk::AddCell(material_type material, int x, int y) {
    if (!buffer_ptr_->IsCellEmpty(x, y)) return;
    buffer_ptr_->CreateMagicPixel(material, x, y);
}

void sandbox_chunk::RemoveCell(int x, int y) { buffer_ptr_->RemoveMagicPixel(x, y); }

void sandbox_chunk::ResetRect() {
    dirty_rect_min_x_ = max_x_;
    dirty_rect_min_y_ = max_y_;
    dirty_rect_max_x_ = min_x_;
    dirty_rect_max_y_ = min_y_;
}

void sandbox_chunk::Update() {
    ResetRect();
    for (int i = min_x_; i <= max_x_; i++) {
        for (int j = min_y_; j <= max_y_; j++) {
            if (buffer_ptr_->IsExpired(i, j)) buffer_ptr_->RemoveMagicPixel(i, j);
            if (buffer_ptr_->buffer_[i][j].update_) UpdateRect(i, j);
        }
    }
    int min_x, min_y, max_x, max_y;
    GetCurrentDirtyRect(max_x, max_y, min_x, min_y);
    active_.clear();
    for (int i = min_x; i <= max_x; i++) {
        for (int j = min_y; j <= max_y; j++) {
            active_.push_back(Vector2(i, j));
        }
    }
    std::shuffle(active_.begin(), active_.end(), rng);
    for (int i = 0; i < active_.size(); i++) {
        buffer_ptr_->buffer_[active_[i].x_][active_[i].y_].Update(*buffer_ptr_);
    }
    last_frame_ = frame_count;
}

void sandbox_chunk::draw_debug(neko_immediate_draw_t *idraw, float scale) {
    neko_vec4_t rectToDraw = {scale * min_x_, scale * min_y_, scale * max_x_, scale * max_y_};

    neko_idraw_rectv(idraw, neko_v2(rectToDraw.x, rectToDraw.y), neko_v2(rectToDraw.w, rectToDraw.z), NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);

    int min_x, min_y, max_x, max_y;
    GetCurrentDirtyRect(max_x, max_y, min_x, min_y);
    if (min_x <= max_x && min_y <= max_y) {
        rectToDraw = {scale * min_x, scale * min_y, scale * (max_x) + 1, scale * (max_y) + 1};
        neko_idraw_rectv(idraw, neko_v2(rectToDraw.x, rectToDraw.y), neko_v2(rectToDraw.z, rectToDraw.w), NEKO_COLOR_RED, NEKO_GRAPHICS_PRIMITIVE_LINES);
    }
}

#if 0


sandbox_simulation::sandbox_simulation() {
    buffer_ptr_ = std::make_shared<sandbox_buffer>();
    chunks_ptr_ = std::make_shared<std::array<sandbox_chunk, kMaxChunk>>();
    jobs_ = new chunk_job[kMaxChunk];
    int x = 0, y = 0;
    for (int i = 0; i < chunks_ptr_->size(); i++) {
        (*chunks_ptr_)[i].Init(x, y, chunks_ptr_, buffer_ptr_);
        x += kMaxChunk;
        if (x >= g_sim_chunk_width) {
            x = 0;
            y += kMaxChunk;
        }
        jobs_[i] = chunk_job(&(*chunks_ptr_)[i]);
    }
    pool_.Start();
    //    pool.init();
}

void sandbox_simulation::reset() { buffer_ptr_->Reset(); }

sandbox_simulation::~sandbox_simulation() {
    pool_.Stop();
    //    pool.shutdown();
}

void sandbox_simulation::process_chunk(int *chunks, u16 num) {
    int done = 0;

    auto update_thread = [](sandbox_chunk &ch) {
        ch.Update();
        return true;
    };

    std::vector<std::future<bool>> future_queue;
    for (int i = 0; i < num; i++) {
        pool_.QueueJob(jobs_[chunks[i]]);
        //        future_queue.push_back(this->pool.submit(update_thread, (*chunks_ptr_)[chunks[i]]));
    }
    //    for (auto& future : future_queue) {
    //        future.get();
    //    }
    while (done != num) {
        done = 0;
        for (int i = 0; i < num; i++) done += (*chunks_ptr_)[chunks[i]].last_frame_ == frame_count;
    }
}

void sandbox_simulation::update() {

    int first_batch_[kChunkBatchSize] = {0, 4, 10, 14, 16, 20, 26, 30, 32, 36, 42, 46, 48, 52, 58, 62};
    int second_batch_[kChunkBatchSize] = {1, 5, 11, 15, 17, 21, 27, 31, 33, 37, 43, 47, 49, 53, 59, 63};
    int third_batch_[kChunkBatchSize] = {2, 6, 8, 12, 18, 22, 24, 28, 34, 38, 40, 44, 50, 54, 56, 60};
    int fourth_batch_[kChunkBatchSize] = {3, 7, 9, 13, 19, 23, 25, 29, 35, 39, 41, 45, 51, 55, 57, 61};

    process_chunk(first_batch_, kChunkBatchSize);
    process_chunk(second_batch_, kChunkBatchSize);
    process_chunk(third_batch_, kChunkBatchSize);
    process_chunk(fourth_batch_, kChunkBatchSize);

    //    int batch[kMaxChunk];
    //    int s = 0;
    //    for (int k = 0; k < 4; k++) {
    //        for (int i = k; i < kMaxChunk; i = i + 4) batch[s++] = i;
    //    }
    //    neko_assert(s == kMaxChunk);
    //    ProcessChunk(batch, kMaxChunk);
}

void sandbox_simulation::SetCell(neko_vec2_t point, material_type material, bool physics) {
    u32 index = point.x + point.y * g_sim_chunk_width;
    u32 chunk_idx = Helper::GetChunk(index);
    if (material == material_type::EMPTY) {
        (*chunks_ptr_)[chunk_idx].RemoveCell(point.x, point.y);
    } else if (material != material_type::EMPTY) {
        (*chunks_ptr_)[chunk_idx].AddCell(material, point.x, point.y);
        if (physics) {
            std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        }
    }
}

void sandbox_simulation::SetCellInsideCircle(neko_vec2_t point, u16 rad, material_type material, bool physics) {
    int sqr = rad * rad;
    for (int i = -rad; i <= rad; i++) {
        for (int j = -rad; j <= rad; j++) {
            if (i * i + j * j <= sqr) {
                s32 x = point.x + i;
                s32 y = point.y + j;
                if (y >= 0 && y < g_sim_chunk_height && x >= 0 && x < g_sim_chunk_width) {
                    SetCell({(f32)x, (f32)y}, material, physics);
                }
            }
        }
    }
}



sandbox_object::sandbox_object(neko_vec4_t rect) {

    this->rect = rect;

    this->draw_buffer = new neko_color_t[g_sim_chunk_width * g_sim_chunk_height];

    this->data = (unsigned char *)neko_safe_malloc(sizeof(unsigned char) * g_sim_chunk_width * g_sim_chunk_height);
    this->edgeSeen = (bool *)neko_safe_malloc(sizeof(bool) * g_sim_chunk_width * g_sim_chunk_height);

    neko_graphics_texture_desc_t t_desc = neko_default_val();

    t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = this->rect.w - this->rect.x;
    t_desc.height = this->rect.z - this->rect.y;

    t_desc.data[0] = this->draw_buffer;

    this->object_texture = neko_graphics_texture_create(&t_desc);
}

void sandbox_object::update() {}

void sandbox_object::render(f32 scale) {

    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = this->rect.w - this->rect.x;
    t_desc.height = this->rect.z - this->rect.y;

    t_desc.data[0] = this->draw_buffer;

    neko_graphics_texture_update(this->object_texture, &t_desc);

    if (object_texture.id != 0) {

        //        neko_idraw_rect_textured_ext(sandbox_game::idraw, this->rect.x, this->rect.y, this->rect.x + this->rect.w * this->scale,
        //                                     this->rect.y + this->rect.z * this->scale, 0, 1, 1, 0, this->object_texture.id, NEKO_COLOR_WHITE);
        //
        //        neko_idraw_rectv(sandbox_game::idraw, neko_v2(this->rect.x,this->rect.y), neko_v2(this->rect.x + this->rect.w * this->scale, this->rect.y + this->rect.z * this->scale),
        //        NEKO_COLOR_WHITE,
        //                         NEKO_GRAPHICS_PRIMITIVE_LINES);

        neko_idraw_rect_textured_ext(sandbox_game::idraw, 0, 0, 512 * scale, 512 * scale, 0, 1, 1, 0, this->object_texture.id, NEKO_COLOR_WHITE);
    }

    if (CL_GAME_USERDATA()->game->debug_mode >= 1) this->chunk_mask_update_mesh();
}
// GameObject::~GameObject() {}

void sandbox_object::chunk_mask_update_mesh() {

    neko_immediate_draw_t *idraw = &CL_GAME_USERDATA()->idraw;

    // std::lock_guard<std::mutex> locker(g_mutex_updatechunkmesh);

    /*
    if (chunk->rb != nullptr) b2world->DestroyBody(chunk->rb->body);
    */

    // 区块坐标转化为世界坐标
    // int chTx = chunk->x * ch->chunk_w + loadZone.x;
    // int chTy = chunk->y * ch->chunk_h + loadZone.y;

    // if (chTx < 0 || chTy < 0 || chTx + ch->chunk_w >= width || chTy + kScreenHeight >= height) {
    //     return;
    // }

    auto sim = CL_GAME_USERDATA()->game->get_sim();

    // 优化掉无固态碰撞的区块
    bool foundAnything = false;
    for (int x = 0; x < g_sim_chunk_width; x++) {
        for (int y = 0; y < g_sim_chunk_height; y++) {

            auto type = sim->buffer_ptr_->GetMaterial(x, y);

            if (type == material_type::ROCK) {
                foundAnything = true;
                goto found;
            }

            // bool f = tiles[(x + meshZone.x) + (y + meshZone.y) * width].mat->physicsType == PhysicsType::SOLID;
            // foundAnything = foundAnything || f;
        }
    }

found:

    if (!foundAnything) {
        return;
    }

    // unsigned char data[ch->chunk_w * kScreenHeight] = neko_default_val();
    // bool edgeSeen[ch->chunk_w * kScreenHeight] = neko_default_val();

    for (int y = 0; y < g_sim_chunk_height; y++) {
        for (int x = 0; x < g_sim_chunk_width; x++) {
            // data[x + y * kScreenWidth] = real_tiles[(x + chTx) + (y + chTy) * width].mat->physicsType == PhysicsType::SOLID;
            //            u8 mat_id = get_particle_at(ch, x, y).mat_id;
            auto type = sim->buffer_ptr_->GetMaterial(x, y);
            this->data[x + y * g_sim_chunk_width] = (type == material_type::ROCK);
            this->edgeSeen[x + y * g_sim_chunk_width] = false;
        }
    }

    // TPPLPoly和MarchingSquares::Result都是非聚合类 需要构造函数 使用STL来存储
    std::vector<std::vector<neko_vec2>> world_meshes = {};
    std::list<neko::TPPLPoly> shapes;
    std::list<neko::marching_squares::ms_result> results;

    int inn = 0;
    int lookIndex = 0;

    // TODO: 23/10/19 优化一下啦
    u32 test_count = 0;

    while (true) {
        // inn++;
        int lookX = lookIndex % g_sim_chunk_width;
        int lookY = lookIndex / g_sim_chunk_width;
        /*if (inn == 1) {
            lookX = kScreenWidth / 2;
            lookY = kScreenHeight / 2;
        }*/

        int edgeX = -1;
        int edgeY = -1;
        int size = g_sim_chunk_width * g_sim_chunk_height;

        for (int i = lookIndex; i < size; i++) {
            if (this->data[i] != 0) {

                int numBorders = 0;
                // if (i % kScreenWidth - 1 >= 0) numBorders += data[(i % kScreenWidth - 1) + i / kScreenWidth * kScreenWidth];
                // if (i / kScreenWidth - 1 >= 0) numBorders += data[(i % kScreenWidth)+(i / kScreenWidth - 1) * kScreenWidth];
                if (i % g_sim_chunk_width + 1 < g_sim_chunk_width) numBorders += this->data[(i % g_sim_chunk_width + 1) + i / g_sim_chunk_width * g_sim_chunk_width];
                if (i / g_sim_chunk_width + 1 < g_sim_chunk_height) numBorders += this->data[(i % g_sim_chunk_width) + (i / g_sim_chunk_width + 1) * g_sim_chunk_width];
                if (i / g_sim_chunk_width + 1 < g_sim_chunk_height && i % g_sim_chunk_width + 1 < g_sim_chunk_width)
                    numBorders += this->data[(i % g_sim_chunk_width + 1) + (i / g_sim_chunk_width + 1) * g_sim_chunk_width];

                // int val = value(i % ch->render_w, i / ch->render_w, ch->render_w, height, data);
                if (numBorders != 3) {
                    edgeX = i % g_sim_chunk_width;
                    edgeY = i / g_sim_chunk_width;
                    break;
                }
            }
        }

        if (edgeX == -1) {
            break;
        }

        // marching_squares::ms_direction edge = marching_squares::find_edge(ch->render_w, ch->render_h, data, lookX, lookY);

        lookX = edgeX;
        lookY = edgeY;

        lookIndex = lookX + lookY * g_sim_chunk_width + 1;

        if (this->edgeSeen[lookX + lookY * g_sim_chunk_width]) {
            inn--;
            continue;
        }

        int val = neko::marching_squares::ms_value(lookX, lookY, g_sim_chunk_width, g_sim_chunk_height, this->data);

        if (val == 0 || val == 15) {
            inn--;
            continue;
        }

        neko::marching_squares::ms_result r = neko::marching_squares::find_perimeter(lookX, lookY, g_sim_chunk_width, g_sim_chunk_height, this->data);

        results.push_back(r);

        std::vector<neko_vec2> worldMesh;

        f32 lastX = (f32)r.initial_x;
        f32 lastY = (f32)r.initial_y;

        for (int i = 0; i < r.directions.size(); i++) {
            // if(r.directions[i].x != 0) r.directions[i].x = r.directions[i].x / abs(r.directions[i].x);
            // if(r.directions[i].y != 0) r.directions[i].y = r.directions[i].y / abs(r.directions[i].y);

            for (int ix = 0; ix < std::max(abs(r.directions[i].x), 1); ix++) {
                for (int iy = 0; iy < std::max(abs(r.directions[i].y), 1); iy++) {
                    int ilx = (int)(lastX + ix * (r.directions[i].x < 0 ? -1 : 1));
                    int ily = (int)(lastY - iy * (r.directions[i].y < 0 ? -1 : 1));

                    if (ilx < 0) ilx = 0;
                    if (ilx >= g_sim_chunk_width) ilx = g_sim_chunk_width - 1;

                    if (ily < 0) ily = 0;
                    if (ily >= g_sim_chunk_height) ily = g_sim_chunk_height - 1;

                    int ind = ilx + ily * g_sim_chunk_width;
                    if (ind >= size) {
                        continue;
                    }
                    this->edgeSeen[ind] = true;

                    test_count++;
                }
            }

            lastX += (f32)r.directions[i].x;
            lastY -= (f32)r.directions[i].y;
            worldMesh.emplace_back(neko_v2(lastX, lastY));
        }

        // 优化
        f32 simplify_tolerance = 1.f;
        if (test_count >= 800) simplify_tolerance = 3.f;
        worldMesh = neko::simplify(worldMesh, simplify_tolerance);

        // 优化单像素点
        if (worldMesh.size() < 3) continue;

        world_meshes.push_back(worldMesh);

        neko::TPPLPoly poly;

        poly.Init((long)worldMesh.size());

        for (int i = 0; i < worldMesh.size(); i++) {
            poly[(int)worldMesh.size() - i - 1] = {worldMesh[i].x, worldMesh[i].y};
        }

        if (poly.GetOrientation() == TPPL_CW) {
            poly.SetHole(true);
        }

        shapes.push_back(poly);
    }

    std::list<neko::TPPLPoly> result;
    std::list<neko::TPPLPoly> result2;

    neko::TPPLPartition part;
    neko::TPPLPartition part2;

    part.RemoveHoles(&shapes, &result2);

    part2.Triangulate_EC(&result2, &result);

    /*bool* solid = new bool[10 * 10];
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            solid[x + y * width] = rand() % 2 == 0;
        }
    }*/

    // Ps::marching_squares ms = Ps::marching_squares(solid, ch->render_w, ch->render_h);

    // Ps::marching_squares ms = Ps::marching_squares(texture);
    // worldMesh = ms.extract_simple(2);

    // chunk->polys.clear();

    //    neko_vec2 position = cam;

    std::for_each(result.begin(), result.end(), [&](neko::TPPLPoly cur) {
        // 修正重叠点
        if (cur[0].x == cur[1].x && cur[1].x == cur[2].x) {
            cur[0].x += 0.01f;
        }
        if (cur[0].y == cur[1].y && cur[1].y == cur[2].y) {
            cur[0].y += 0.01f;
        }

        std::vector<neko_vec2> vec = {neko_v2((f32)cur[0].x, (f32)cur[0].y), neko_v2((f32)cur[1].x, (f32)cur[1].y), neko_v2((f32)cur[2].x, (f32)cur[2].y)};

        std::for_each(vec.begin(), vec.end(), [&](neko_vec2 &v) {
            v.x *= CL_GAME_USERDATA()->game->draw_scale;
            v.y *= CL_GAME_USERDATA()->game->draw_scale;
            //            v.x += cam.x;
            //            v.y += cam.y;
        });

        //  worldTris.push_back(vec);
        //  b2PolygonShape sh;
        //  sh.Set(&vec[0], 3);
        //  chunk->polys.push_back(sh);

        neko_idraw_trianglev(idraw, vec[0], vec[1], vec[2], NEKO_COLOR_WHITE, NEKO_GRAPHICS_PRIMITIVE_LINES);
    });

    // neko_graphics_fc_text(std::to_string(test_count).c_str(), idraw->data->font_fc_default, 500, 100);
}

void sandbox_object::clean() {
    neko_graphics_texture_destroy(this->object_texture);
    delete[] this->draw_buffer;

    if (this->edgeSeen) neko_safe_free(this->edgeSeen);
    if (this->data) neko_safe_free(this->data);
}

neko_immediate_draw_t *sandbox_game::idraw = nullptr;

sandbox_game::sandbox_game(neko_immediate_draw_t *idraw) {
    init(idraw);
    create_simulation();
    create_viewport();
    create_ui();
    reset_variables();
}

sandbox_game::~sandbox_game() {}

void sandbox_game::init(neko_immediate_draw_t *idraw) {
    this->idraw = idraw;
    count = 0;
}

void sandbox_game::create_simulation() { simulation = new sandbox_simulation(); }

void sandbox_game::create_viewport() {
    neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());
    this->fbs_scale = (float)win_size.x / CL_GAME_USERDATA()->fbs.x;
    viewport = new sandbox_object(kViewportRect);
}

void sandbox_game::create_ui() {
    auto set_material = std::bind(&sandbox_game::set_material, this, std::placeholders::_1);
    auto pause = std::bind(&sandbox_game::pause, this, std::placeholders::_1);
    auto reset = std::bind(&sandbox_game::reset_simulation, this, std::placeholders::_1);
    //    ui_ = new UI(renderer_, {Helper::ScreenWidthPoint(11), 0, (int)kScreenWidth - Helper::ScreenWidthPoint(11), (int)kScreenHeight}, neko_color(128, 128, 128, 255));
    //    ui_->AddButtonGroup({0, 0, Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(1)}, {0, 0, 120, 64}, "Pause", pause);
    //    ui_->AddButtonGroup({0, Helper::ScreenHeightPoint(1), Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(2)}, {0, 0, 120, 64}, "Reset", reset);
    //    ui_->AddButtonGroup({0, Helper::ScreenWidthPoint(2), Helper::ScreenWidthPoint(4), Helper::ScreenWidthPoint(5)}, {0, 0, 120, 64}, "Empty Rock Sand Water Steam Wood Fire", set_material);

    pixelui_init(&this->pixelui);

    this->pixelui.show_material_selection_panel = true;
    this->pixelui.show_frame_count = true;
}

void sandbox_game::set_material(int m) { material = static_cast<material_type>(material); }

void sandbox_game::pause(int x) { paused = !paused; }

void sandbox_game::reset_simulation(int x) { simulation->reset(); }

void sandbox_game::reset_variables() {
    tick_count = 0;
    draw_radius = 15;
    material = material_type::ROCK;
    draw_scale = 3.f;
    CL_GAME_USERDATA()->debug_mode = 0;
}

void sandbox_game::pre_update() {
    current_tick = neko_platform_elapsed_time();
    frame_count++;
    //    delta_time = (current_tick - last_tick) / 1000.0f;

    f32 dt = neko_platform_delta_time();

    if (neko_platform_was_key_down(NEKO_KEYCODE_W)) CL_GAME_USERDATA()->cam.y = CL_GAME_USERDATA()->cam.y - CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;
    if (neko_platform_was_key_down(NEKO_KEYCODE_S)) CL_GAME_USERDATA()->cam.y = CL_GAME_USERDATA()->cam.y + CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;
    if (neko_platform_was_key_down(NEKO_KEYCODE_A)) CL_GAME_USERDATA()->cam.x = CL_GAME_USERDATA()->cam.x - CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;
    if (neko_platform_was_key_down(NEKO_KEYCODE_D)) CL_GAME_USERDATA()->cam.x = CL_GAME_USERDATA()->cam.x + CL_GAME_USERDATA()->player_v * CL_GAME_USERDATA()->game->draw_scale * dt;

    if (neko_platform_key_pressed(NEKO_KEYCODE_F)) CL_GAME_USERDATA()->debug_mode = (CL_GAME_USERDATA()->debug_mode + 1) % 4;

    draw_radius = std::clamp<s16>(draw_radius, kMinDrawRadius, kMaxDrawRadius);
    neko_vec2 foo = neko_platform_mouse_positionv();
    neko_vec2 win_size = neko_platform_window_sizev(neko_platform_main_window());

    this->fbs_scale = (float)win_size.x / CL_GAME_USERDATA()->fbs.x;

    foo.x = foo.x / this->fbs_scale;
    foo.y = foo.y / this->fbs_scale;

    foo.x += CL_GAME_USERDATA()->cam.x;
    foo.y += CL_GAME_USERDATA()->cam.y;

    cursor = foo;

    this->material = this->pixelui.material_selection;
}

void sandbox_game::update() {
    pre_update();

    auto draw_pos = cursor;
    draw_pos.x = draw_pos.x / this->draw_scale;
    draw_pos.y = draw_pos.y / this->draw_scale;

    if (neko_platform_mouse_down(NEKO_MOUSE_LBUTTON) && !ImGui::GetIO().WantCaptureMouse) {
        simulation->SetCellInsideCircle(draw_pos, draw_radius, material);
    } else if (neko_platform_mouse_down(NEKO_MOUSE_RBUTTON) && !ImGui::GetIO().WantCaptureMouse) {
        simulation->SetCellInsideCircle(draw_pos, draw_radius, material_type::EMPTY);
    }

    neko_vec2 mw = neko_platform_mouse_wheelv();
    if (neko_platform_was_key_down(NEKO_KEYCODE_LEFT_CONTROL)) {
        if (mw.y > 0)
            this->draw_radius++;
        else if (mw.y < 0)
            this->draw_radius--;
    } else {
        if (mw.y > 0)
            this->pixelui.material_selection = (material_type)((u32)this->pixelui.material_selection - 1);
        else if (mw.y < 0)
            this->pixelui.material_selection = (material_type)((u32)this->pixelui.material_selection + 1);
    }

    if (!paused) {
        int pitch = (g_sim_chunk_width) * 8;
        //        SDL_LockTexture(viewport_->object_texture_ptr_->texture_, nullptr, (void **)&viewport_->draw_buffer_, &pitch);
        simulation->update();
        for (int i = 0; i < g_sim_chunk_width; i++) {
            for (int j = 0; j < g_sim_chunk_height; j++) {
                s64 foo = i + j * g_sim_chunk_width;
                viewport->draw_buffer[foo] = simulation->buffer_ptr_->GetCellColor(i, j);
            }
        }
        //        SDL_UnlockTexture(viewport_->object_texture_ptr_->texture_);
    }
    late_update();
}

void sandbox_game::late_update() {
    //    last_cursor_ = cursor;
    std::ostringstream stream;
    u32 last_update = neko_platform_elapsed_time();
    last_tick = current_tick;
    float frame_time = (last_update - current_tick) / 1000.0f;
    tick_count += last_update - current_tick;
    if (count++ == 60) {
        count = 0;
        stream << std::fixed << std::setprecision(2) << "Current: " << (1.0f / frame_time) << " fps Avg: " << (1000 / std::max<u32>(tick_count / frame_count, 1)) << " fps";
        //        performance_bar_->text_.back().text_ = stream.str();
        //        performance_bar_->CreateTexture();
    }
}

void sandbox_game::render() {

    viewport->render(draw_scale);

    if (CL_GAME_USERDATA()->debug_mode >= 2) {
        for (int i = 0; i < kMaxChunk; i++) {
            (*simulation->chunks_ptr_)[i].draw_debug(sandbox_game::idraw, this->draw_scale);
        }
    }
    //    neko_idraw_circle(sandbox_game::idraw, cursor.x, cursor.y, draw_radius * this->draw_scale, 20, 100, 150, 220, 255, NEKO_GRAPHICS_PRIMITIVE_LINES);

    this->pixelui.brush_size = draw_radius;

    //    neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_defaults(sandbox_game::idraw);
    neko_idraw_camera2d(sandbox_game::idraw, (u32)CL_GAME_USERDATA()->fbs.x, (u32)CL_GAME_USERDATA()->fbs.y);

    update_ui(&this->pixelui);
    neko_idraw_rect_textured_ext(sandbox_game::idraw, 0, 0, CL_GAME_USERDATA()->fbs.x, CL_GAME_USERDATA()->fbs.y, 0, 1, 1, 0, this->pixelui.tex_ui.id, NEKO_COLOR_WHITE);
}

void sandbox_game::clean() {

    pixelui_destroy(&this->pixelui);

    delete simulation;
    //    delete[] viewport->draw_buffer;

    this->viewport->clean();

    delete viewport;
}

// user data
extern neko_client_userdata_t g_client_userdata;

s32 pixelui_compute_idx(pixelui_t *pixelui, s32 x, s32 y) { return (y * pixelui->g_pixelui_texture_width + x); }

bool pixelui_in_bounds(pixelui_t *pixelui, s32 x, s32 y) {
    if (x < 0 || x > (pixelui->g_pixelui_texture_width - 1) || y < 0 || y > (pixelui->g_pixelui_texture_height - 1)) return false;
    return true;
}

void putpixel(pixelui_t *pixelui, s32 x, s32 y) {
    if (pixelui_in_bounds(pixelui, x, y)) {
        pixelui->ui_buffer[pixelui_compute_idx(pixelui, x, y)] = neko_color_t{255, 255, 255, 255};
    }
}

// 圆生成函数
// 使用 Bresenham 算法
void circle_bres(pixelui_t *pixelui, s32 xc, s32 yc, s32 r) {
    auto drawCircle = [](pixelui_t *pixelui, s32 xc, s32 yc, s32 x, s32 y) {
        putpixel(pixelui, xc + x, yc + y);
        putpixel(pixelui, xc - x, yc + y);
        putpixel(pixelui, xc + x, yc - y);
        putpixel(pixelui, xc - x, yc - y);
        putpixel(pixelui, xc + y, yc + x);
        putpixel(pixelui, xc - y, yc + x);
        putpixel(pixelui, xc + y, yc - x);
        putpixel(pixelui, xc - y, yc - x);
    };

    s32 x = 0, y = r;
    s32 d = 3 - 2 * r;
    drawCircle(pixelui, xc, yc, x, y);
    while (y >= x) {
        // For each pixel we will
        // draw all eight pixels
        x++;

        // Check for decision parameter
        // and correspondingly
        // update d, x, y
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else
            d = d + 4 * x + 6;
        drawCircle(pixelui, xc, yc, x, y);
    }
}

neko_vec2 calculate_mouse_position(pixelui_t *pixelui) {
    neko_vec2 ws = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_vec2 pmp = neko_platform_mouse_positionv();
    // Need to place mouse into frame
    f32 x_scale = pmp.x / (f32)ws.x;
    f32 y_scale = pmp.y / (f32)ws.y;
    return neko_vec2{x_scale * (f32)pixelui->g_pixelui_texture_width, y_scale * (f32)pixelui->g_pixelui_texture_height};
}

void draw_glyph_at(pixelui_t *pixelui, font_t *f, neko_color_t *buffer, s32 x, s32 y, char c, neko_color_t col) {
    u8 *font_data = (u8 *)f->data;
    font_glyph_t g = get_glyph(f, c);

    // How to accurately place? I have width and height of glyph in texture, but need to convert this to RGBA data for ui buffer
    for (s32 h = 0; h < g.height; ++h) {
        for (s32 w = 0; w < g.width; ++w) {
            s32 _w = w + g.x;
            s32 _h = h + g.y;
            u8 a = font_data[(_h * f->width + _w) * f->num_comps + 0] == 0 ? 0 : 255;
            neko_color_t c = {font_data[(_h * f->width + _w) * f->num_comps + 0], font_data[(_h * f->width + _w) * f->num_comps + 1], font_data[(_h * f->width + _w) * f->num_comps + 2], a};
            if (pixelui_in_bounds(pixelui, x + w, y + h) && a) {
                buffer[pixelui_compute_idx(pixelui, x + w, y + h)] = col;
            }
        }
    }
}

void draw_string_at(pixelui_t *pixelui, font_t *f, neko_color_t *buffer, s32 x, s32 y, const char *str, usize len, neko_color_t col) {
    u8 *font_data = (u8 *)f->data;
    for (u32 i = 0; i < len; ++i) {
        font_glyph_t g = get_glyph(f, str[i]);
        draw_glyph_at(pixelui, f, buffer, x, y, str[i], col);
        x += g.width + f->glyph_advance;  // Move by glyph width + advance
    }
}

bool in_rect(neko_vec2 p, neko_vec2 ro, neko_vec2 rd) {
    if (p.x < ro.x || p.x > ro.x + rd.x || p.y < ro.y || p.y > ro.y + rd.y) return false;
    return true;
}

bool gui_rect(pixelui_t *pixelui, neko_color_t *buffer, s32 _x, s32 _y, s32 _w, s32 _h, neko_color_t c) {
    neko_vec2 mp = calculate_mouse_position(pixelui);

    for (u32 h = 0; h < _h; ++h) {
        for (u32 w = 0; w < _w; ++w) {
            if (pixelui_in_bounds(pixelui, _x + w, _y + h)) {
                buffer[pixelui_compute_idx(pixelui, _x + w, _y + h)] = c;
            }
        }
    }

    bool clicked = neko_platform_mouse_down(NEKO_MOUSE_LBUTTON);

    return in_rect(mp, neko_vec2{(f32)_x, (f32)_y}, neko_vec2{(f32)_w, (f32)_h}) && clicked;
}

#define __gui_interaction(x, y, w, h, c, str, id)                                                                                                                          \
    do {                                                                                                                                                                   \
        if ((id) == pixelui->material_selection) {                                                                                                                         \
            const s32 b = 2;                                                                                                                                               \
            gui_rect(pixelui, pixelui->ui_buffer, x - b / 2, y - b / 2, w + b, h + b, neko_color_t{200, 150, 20, 255});                                                    \
        }                                                                                                                                                                  \
        neko_vec2 mp = calculate_mouse_position(pixelui);                                                                                                                  \
        if (in_rect(mp, neko_vec2{(f32)(x), (f32)(y)}, neko_vec2{(w), (h)})) {                                                                                             \
            interaction |= true;                                                                                                                                           \
            char _str[] = (str);                                                                                                                                           \
            neko_color_t col = neko_color_t{255, 255, 255, 255};                                                                                                           \
            neko_color_t s_col = neko_color_t{10, 10, 10, 255};                                                                                                            \
            neko_color_t r_col = neko_color_t{5, 5, 5, 170};                                                                                                               \
            /*Draw rect around text as well for easier viewing*/                                                                                                           \
            gui_rect(pixelui, pixelui->ui_buffer, pixelui->g_pixelui_texture_width / 2 - 50, 15, 100, 20, r_col);                                                          \
            draw_string_at(pixelui, &pixel_font, pixelui->ui_buffer, pixelui->g_pixelui_texture_width / 2 + 1 - (sizeof(str) * 5) / 2, 20 - 1, _str, sizeof(_str), s_col); \
            draw_string_at(pixelui, &pixel_font, pixelui->ui_buffer, pixelui->g_pixelui_texture_width / 2 - (sizeof(str) * 5) / 2, 20, _str, sizeof(_str), col);           \
        }                                                                                                                                                                  \
        if (gui_rect(pixelui, pixelui->ui_buffer, x, y, w, h, c)) {                                                                                                        \
            pixelui->material_selection = id;                                                                                                                              \
        }                                                                                                                                                                  \
    } while (0)

bool update_ui(pixelui_t *pixelui) {
    bool interaction = false;
    // neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    // neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    // Cache transformed mouse position
    neko_vec2 mp = calculate_mouse_position(pixelui);

    // Do ui stuff
    memset(pixelui->ui_buffer, 0, pixelui->g_pixelui_texture_width * pixelui->g_pixelui_texture_height * sizeof(neko_color_t));

    // Material selection panel gui
    if (pixelui->show_material_selection_panel) {
        const s32 offset = 12;
        s32 xoff = 20;
        s32 base = 10;

        auto &mat_map = MagicPixelFactory::Instance()->get_registy();
        for (auto &mat_reg : mat_map) {
            auto mat = mat_reg.second();
            __gui_interaction(pixelui->g_pixelui_texture_width - xoff, base + offset * (u32)mat->material_, 10, 10, mat->color_, "Sand", mat->material_);
        }
    }

    if (pixelui->show_frame_count) {

        char frame_time_str[256];
        neko_snprintf(frame_time_str, sizeof(frame_time_str), "frame: %6.2f ms %llu", neko_platform_frame_time(), pixelui->update_time);
        draw_string_at(pixelui, &pixel_font, pixelui->ui_buffer, 10, 10, frame_time_str, strlen(frame_time_str), neko_color_t{255, 255, 255, 255});

        // gfx->fontcache_push_x_y(std::format("test: {0} {1}", l_check, neko_buildnum()), g_basic_font, 40, 160);
    }

    pixelui->update_time = 0;

    // 围绕鼠标指针绘制圆圈
    circle_bres(pixelui, (s32)(mp.x), (s32)(mp.y), pixelui->brush_size);

    // 将更新后的纹理数据上传到 GPU
    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;

    t_desc.width = pixelui->g_pixelui_texture_width;
    t_desc.height = pixelui->g_pixelui_texture_height;

    t_desc.data[0] = pixelui->ui_buffer;

    neko_graphics_texture_update(pixelui->tex_ui, &t_desc);

    return interaction;
}

void pixelui_init(pixelui_t *pixelui) {

    pixelui->ui_buffer = (neko_color_t *)neko_safe_malloc(pixelui->g_pixelui_texture_width * pixelui->g_pixelui_texture_height * sizeof(neko_color_t));
    memset(pixelui->ui_buffer, 0, pixelui->g_pixelui_texture_width * pixelui->g_pixelui_texture_height);

    neko_graphics_texture_desc_t t_desc = neko_default_val();
    t_desc.format = NEKO_GRAPHICS_TEXTURE_FORMAT_RGBA8;
    t_desc.mag_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.min_filter = NEKO_GRAPHICS_TEXTURE_FILTER_NEAREST;
    t_desc.num_mips = 0;
    t_desc.width = pixelui->g_pixelui_texture_width;
    t_desc.height = pixelui->g_pixelui_texture_height;
    t_desc.data[0] = pixelui->ui_buffer;

    pixelui->tex_ui = neko_graphics_texture_create(&t_desc);

    // Load UI font texture data from file
    construct_font_data(g_font_data);
}

void pixelui_destroy(pixelui_t *pixelui) {
    neko_graphics_texture_destroy(pixelui->tex_ui);
    neko_safe_free(pixelui->ui_buffer);
}

void pixelui_draw(pixelui_t *pixelui) {
    neko_immediate_draw_t *idraw = &CL_GAME_USERDATA()->idraw;
    const neko_vec2 fbs = neko_platform_framebuffer_sizev(neko_platform_main_window());
    neko_idraw_rect_textured_ext(idraw, 0, 0, fbs.x, fbs.y, 0, 1, 1, 0, pixelui->tex_ui.id, NEKO_COLOR_WHITE);
}

#endif