
#ifndef NEKO_PARTICLE_H
#define NEKO_PARTICLE_H

#include <cstdlib>

#include "engine/graphics/neko_graphics.h"
#include "engine/gui/neko_imgui_utils.hpp"

namespace neko {

struct neko_particle {
private:
    f64 m_x;
    f64 m_y;
    f64 m_speed;
    f64 m_direction;
    void init();

public:
    neko_particle();
    virtual ~neko_particle();
    void update(int interval, float spin, float speed);
    void draw(neko_command_buffer_t* cb, ImVec4 color, int particleRadius);
};

struct neko_swarm_simulator_settings {
public:
    int nParticles;
    bool cycleColors;
    float colorCyclingSpeed;
    ImVec4 particleColor;
    int particleRadius;
    float particleSpin;
    float particleSpeed;
    int blurRadius;

public:
    neko_swarm_simulator_settings();
    neko_swarm_simulator_settings(int nParticles, int particleRadius, float particleSpin, float particleSpeed, int blurRadius);
    void show();
};

class neko_obj_swarm {
private:
    std::vector<neko_particle*> m_pParticles;
    int lastTime;                            // 上次更新屏幕的时间
    ImVec4 color;                            // 粒子颜色
    neko_swarm_simulator_settings settings;  // 粒子模拟设置
private:
    void update_settings(neko_swarm_simulator_settings settings);

public:
    neko_obj_swarm();
    void update(int elapsed, neko_swarm_simulator_settings settings);
    void draw(neko_command_buffer_t* cb);
};

}  // namespace neko

#endif