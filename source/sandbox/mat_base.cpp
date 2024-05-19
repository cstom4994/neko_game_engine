

#include "sandbox/mat_base.h"

#include "sandbox/helper.h"
#include "sandbox/simulation.h"

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
