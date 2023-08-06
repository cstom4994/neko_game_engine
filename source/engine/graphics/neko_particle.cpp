
#include "neko_particle.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

#include "engine/base/neko_engine.h"
#include "engine/gui/neko_imgui_utils.hpp"

namespace neko {

using namespace imgui;

neko_particle::neko_particle() : m_x(0.0), m_y(0.0) { init(); }

neko_particle::~neko_particle() {}

void neko_particle::init() {
    m_x = 0;
    m_y = 0;
    m_direction = 2 * neko_pi * rand() / RAND_MAX;
    m_speed = 0.04 * rand() / RAND_MAX;
    m_speed *= m_speed;
}

void neko_particle::update(int interval, float spin, float speed) {
    m_direction += interval * spin;

    double xspeed = m_speed * cos(m_direction);
    double yspeed = m_speed * sin(m_direction);

    m_x += xspeed * interval * speed;
    m_y += yspeed * interval * speed;

    if (m_x < -1 || m_x > 1 || m_y < -1 || m_y > 1) {
        init();
    }

    if (rand() < RAND_MAX / 100) {
        init();
    }
}

void neko_particle::draw(neko_command_buffer_t* cb, ImVec4 color, int particleRadius) {

    neko_graphics_i* gfx = neko_engine_instance()->ctx.graphics;
    neko_platform_i* platform = neko_engine_instance()->ctx.platform;

    neko_vec2 win_size = platform->window_size(platform->main_window());

    f32 x = (int)((m_x + 1) * (win_size.x) / 2);
    f32 y = (int)(m_y * (win_size.x) / 2 + (int)((win_size.y) / 2));

    // 暂时用立即渲染方法测试
    gfx->immediate.draw_circle(cb, neko_vec2{x, y}, particleRadius, 0, neko_color_white);
}

neko_obj_swarm::neko_obj_swarm() : lastTime(0), color(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
    for (int i = 0; i < 2500; i++) m_pParticles.push_back(new neko_particle());
}

void neko_obj_swarm::update_settings(neko_swarm_simulator_settings settings) {
    this->settings.nParticles = settings.nParticles;
    this->settings.cycleColors = settings.cycleColors;
    this->settings.particleColor = settings.particleColor;
    this->settings.particleRadius = settings.particleRadius;
    this->settings.blurRadius = settings.blurRadius;
}

void neko_obj_swarm::update(int elapsed, neko_swarm_simulator_settings settings) {
    update_settings(settings);

    int interval = elapsed - lastTime;
    lastTime = elapsed;
    for (neko_particle* particle : m_pParticles) {
        particle->update(interval, settings.particleSpin, settings.particleSpeed);
    }

    if (settings.cycleColors) {
        color.x = (float)((1 + sin(elapsed * settings.colorCyclingSpeed * 2)) * 0.5);
        color.y = (float)((1 + sin(elapsed * settings.colorCyclingSpeed)) * 0.5);
        color.z = (float)((1 + sin(elapsed * settings.colorCyclingSpeed * 3)) * 0.5);
    } else {
        color.x = settings.particleColor.x;
        color.y = settings.particleColor.y;
        color.z = settings.particleColor.z;
    }

    if (m_pParticles.size() != settings.nParticles) {
        int oldNParticles = (int)m_pParticles.size();
        m_pParticles.resize((size_t)settings.nParticles);
        if (oldNParticles < settings.nParticles) {
            int count = 0;
            for (int i = oldNParticles; i < settings.nParticles; i++) {
                m_pParticles[i] = new neko_particle();
                count++;
            }
        }
    }
}

void neko_obj_swarm::draw(neko_command_buffer_t* cb) {
    for (neko_particle* particle : m_pParticles) particle->draw(cb, color, settings.particleRadius);
}

neko_swarm_simulator_settings::neko_swarm_simulator_settings() : neko_swarm_simulator_settings(2500, 1, 0.0003f, 0.5f, 1) {}

neko_swarm_simulator_settings::neko_swarm_simulator_settings(int nParticles, int particleRadius, float particleSpin, float particleSpeed, int blurRadius)
    : nParticles(nParticles), particleRadius(particleRadius), particleSpin(particleSpin), particleSpeed(particleSpeed), blurRadius(blurRadius) {
    cycleColors = true;
    colorCyclingSpeed = 0.001;
    particleColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
}

void neko_swarm_simulator_settings::show() {
    ImGui::Begin("Settings");

    // Number of particles
    ImGui::Text("Number of particles");
    ImGui::SameLine();
    neko_imgui_help_marker(
            "Control the number of simulated particles by\n"
            "adjusting the slider.\n");
    ImGui::SliderInt("##Number of particles", &nParticles, 1, 20000);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Should colors cycle?
    static int cycle = 1;
    ImGui::Text("Cycle colors");
    ImGui::SameLine();
    neko_imgui_help_marker(
            "If set to true, particle colors are cycled automatically.\n"
            "Otherwise, the particle color may be defined manually\n"
            "through the color picker below.\n");

    ImGui::RadioButton("True ", &cycle, 1);
    ImGui::SameLine();
    static int cyclingSpeed = 5;
    ImGui::SliderInt("Cycle speed", &cyclingSpeed, 1, 20);
    ImGui::SameLine();
    neko_imgui_help_marker("Adjust the color cycling speed.\n");
    colorCyclingSpeed = cyclingSpeed * 0.0001f;

    ImGui::RadioButton("False", &cycle, 0);
    ImGui::SameLine();
    cycleColors = cycle != 0;
    static ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
    ImGui::ColorEdit3("", (float*)&color);
    if (!cycleColors) {
        particleColor = color;
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Particle radius
    ImGui::Text("Particle radius");
    ImGui::SameLine();
    neko_imgui_help_marker("The smaller the radius, the smaller the particle is.\n");
    ImGui::SliderInt("##Particle radius", &particleRadius, 1, 5);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Particle spin
    static int spin = 3;
    ImGui::Text("Particle spin");
    ImGui::SameLine();
    neko_imgui_help_marker(
            "How much should exploding particles \"spin\" relative\n"
            "to the explosion center?\n");
    ImGui::SliderInt("##Particle spin", &spin, 1, 8);
    particleSpin = spin * 0.0001f;
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Particle speed
    static int speed = 5;
    ImGui::Text("Particle speed");
    ImGui::SameLine();
    neko_imgui_help_marker("How fast should exploding particles diffuse?\n");
    ImGui::SliderInt("##Particle speed", &speed, 1, 10);
    particleSpeed = speed * 0.1f;
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Window footer
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

}  // namespace neko