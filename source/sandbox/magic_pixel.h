

#ifndef GAME_MAGIC_PIXEL_H
#define GAME_MAGIC_PIXEL_H

#include <map>
#include <memory>

#include "engine/neko.hpp"
#include "sandbox/helper.h"

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
    virtual void Update(sandbox_buffer& buffer, int x, int y) {}
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
    static MagicPixelFactory* Instance() {
        if (!ptr_instance_) ptr_instance_ = new MagicPixelFactory;
        return ptr_instance_;
    }

    neko_inline const std::map<material_type, CreateMagicPixelCallback>& get_registy() const { return callback_map; }

private:
    std::map<material_type, CreateMagicPixelCallback> callback_map;
    MagicPixelFactory() = default;
    MagicPixelFactory(const MagicPixelFactory&);
    MagicPixelFactory& operator=(const MagicPixelFactory&);
    static MagicPixelFactory* ptr_instance_;
};

#endif