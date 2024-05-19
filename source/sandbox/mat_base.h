
#ifndef GAME_MAT_BASE_H
#define GAME_MAT_BASE_H

#include <algorithm>
#include <vector>

#include "sandbox/game_cvar.h"
#include "sandbox/helper.h"
#include "sandbox/magic_pixel.h"

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

namespace {
std::unique_ptr<MagicPixel> CreateFire() { return std::make_unique<Fire>(); }
const bool registered_fire = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::FIRE, CreateFire);
}  // namespace

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

namespace {
std::unique_ptr<MagicPixel> CreateGas() { return std::make_unique<Gas>(); }
const bool registered_gas = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::GAS, CreateGas);
}  // namespace

class Liquid : public Movable {
protected:
    int dispersion_rate_;

public:
    Liquid();
    int CanMove(sandbox_buffer &buffer, int x, int y);
    void CelularAutomata(sandbox_buffer &buffer, int x, int y);
    void Update(sandbox_buffer &buffer, int x, int y);
};

namespace {
std::unique_ptr<MagicPixel> CreateLiquid() { return std::make_unique<Liquid>(); }
const bool registered_liquid = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::LIQUID, CreateLiquid);
}  // namespace

class Solid : public Movable {
public:
    Solid();
    int CanMove(sandbox_buffer &buffer, int x, int y);
    void Update(sandbox_buffer &buffer, int x, int y);

private:
    void CelularAutomata(sandbox_buffer &buffer, int x, int y);
    void PhysicSimulation();
};

namespace {
std::unique_ptr<MagicPixel> CreateSolid() { return std::make_unique<Solid>(); }
const bool registered_solid = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::SOLID, CreateSolid);
}  // namespace

class Static : public MagicPixel {
public:
    Static();
    void Update();
};

namespace {
std::unique_ptr<MagicPixel> CreateStatic() { return std::make_unique<Static>(); }
const bool registered_static = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::STATIC, CreateStatic);
}  // namespace

class Rock : public Static {
public:
    Rock();
};

namespace {
std::unique_ptr<MagicPixel> CreateRock() { return std::make_unique<Rock>(); }
const bool registered_rock = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::ROCK, CreateRock);
}  // namespace

class Sand : public Solid {
public:
    Sand();
};

namespace {
std::unique_ptr<MagicPixel> CreateSand() { return std::make_unique<Sand>(); }
const bool registered_sand = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::SAND, CreateSand);
}  // namespace

class Steam : public Gas {
public:
    Steam();
};

namespace {
std::unique_ptr<MagicPixel> CreateSteam() { return std::make_unique<Steam>(); }
const bool registered_steam = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::STEAM, CreateSteam);
}  // namespace

class Water : public Liquid {
public:
    Water();
};

namespace {
std::unique_ptr<MagicPixel> CreateWater() { return std::make_unique<Water>(); }
const bool registered_water = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::WATER, CreateWater);
}  // namespace

class Wood : public Static {
public:
    Wood();
};

namespace {
std::unique_ptr<MagicPixel> CreateWood() { return std::make_unique<Wood>(); }
const bool registered_wood = MagicPixelFactory::Instance()->RegisterMagicPixel(material_type::WOOD, CreateWood);
}  // namespace

#endif