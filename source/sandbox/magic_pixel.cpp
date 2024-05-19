
#include "magic_pixel.h"

MagicPixel::MagicPixel() {}


MagicPixelFactory* MagicPixelFactory::ptr_instance_ = nullptr;

bool MagicPixelFactory::RegisterMagicPixel(material_type material, CreateMagicPixelCallback fn) {
    callback_map[material] = fn;
    return true;
}
std::unique_ptr<MagicPixel> MagicPixelFactory::CreateMagicPixel(material_type material) { return callback_map[material](); }
