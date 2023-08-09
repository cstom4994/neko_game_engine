
#ifndef NEKO_LABEL_HPP
#define NEKO_LABEL_HPP

#include <string>

#include "engine/graphics/neko_fontcache.h"
#include "engine/math/neko_math.h"
#include "engine/utility/module.hpp"

namespace neko {

using neko_font_index = ve_font_id;

class text_renderer final : public module<text_renderer> {
public:
    text_renderer() noexcept { __init(); };
    ~text_renderer() noexcept { __end(); };

    neko_move_only(text_renderer);

    void __init();
    void __end();
    void __update() { draw(); };

public:
    void draw();
    neko_font_index load(const void* data, size_t data_size, f32 font_size = 42.0f);

    void push(const std::string& text, const neko_font_index font, const neko_vec2 pos);
    void push(const std::string& text, const neko_font_index font, const f32 x, const f32 y);
    void resize(neko_vec2 size);
    neko_vec2 calc_pos(f32 x, f32 y) const;

private:
    s32 screen_w;
    s32 screen_h;
};
}  // namespace neko

#endif