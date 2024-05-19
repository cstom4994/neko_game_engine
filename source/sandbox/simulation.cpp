#include "simulation.h"

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
