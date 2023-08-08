
#ifndef NEKO_LABEL_HPP
#define NEKO_LABEL_HPP

#include <string>

#include "engine/graphics/neko_fontcache.h"
#include "engine/math/neko_math.h"
#include "engine/utility/module.hpp"

namespace neko {

using neko_font_index = ve_font_id;

class text_renderer final : public module<text_renderer> /*: public ME::props_auto_reg<ME_fontcache>*/ {
    // public:
    //     static void reg() {
    //         ME::registry::class_<ME_fontcache>()
    //                 // screen
    //                 .prop("screen_w", &ME_fontcache::screen_w, false)
    //                 .prop("screen_h", &ME_fontcache::screen_h, false);
    //     }
public:
    text_renderer() noexcept { __init(); };
    ~text_renderer() noexcept { __end(); };

    neko_move_only(text_renderer);

    void __init();
    void __end();

public:
    void drawcmd();
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