#include "engine/font.h"

#include "engine/api.hpp"
#include "engine/asset.h"
#include "engine/game.h"

bool FontFamily::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }

    FontFamily f = {};
    f.ttf = contents;
    f.sb = {};
    *this = f;
    return true;
}

void FontFamily::trash() {
    for (auto [k, v] : ranges) {
        // gfx_fini会释放所有texture
        // destroy_texture_handle(v->tex.id, NULL);
    }
    sb.trash();
    ranges.trash();

    mem_free(ttf.data);
}

struct FontKey {
    float size;
    i32 ch;
};

static FontKey font_key(float size, i32 charcode) {
    FontKey fk = {};
    fk.size = size;
    fk.ch = (charcode / array_size(FontRange::chars)) * array_size(FontRange::chars);
    return fk;
}

static void make_font_range(FontRange *out, FontFamily *font, FontKey key) {
    PROFILE_FUNC();

    i32 width = 512;
    i32 height = 256;

    u8 *bitmap = nullptr;
    while (bitmap == nullptr) {
        PROFILE_BLOCK("try bake");

        bitmap = (u8 *)mem_alloc(width * height);
        i32 res = stbtt_BakeFontBitmap((u8 *)font->ttf.data, 0, key.size, bitmap, width, height, key.ch, array_size(out->chars), out->chars);
        if (res < 0) {  // 如果图片大小不够就扩大
            mem_free(bitmap);
            bitmap = nullptr;
            width *= 2;
            height *= 2;
        }
    }
    neko_defer(mem_free(bitmap));

    u8 *image_data = (u8 *)mem_alloc(width * height * 4);
    neko_defer(mem_free(image_data));

    {
        PROFILE_BLOCK("convert rgba");

        for (i32 i = 0; i < width * height * 4; i += 4) {
            image_data[i + 0] = 255;
            image_data[i + 1] = 255;
            image_data[i + 2] = 255;
            image_data[i + 3] = bitmap[i / 4];
        }
    }

    // static int iii = 0;
    // stbi_write_png(std::format("font_bitmap_{}.png", iii++).c_str(), width, height, 4, image_data, 0);

    {
        PROFILE_BLOCK("make image");

        u8 *data = reinterpret_cast<u8 *>(image_data);

        gfx_texture_desc_t t_desc = {};

        t_desc.format = R_TEXTURE_FORMAT_RGBA8;
        t_desc.mag_filter = R_TEXTURE_FILTER_NEAREST;
        t_desc.min_filter = R_TEXTURE_FILTER_NEAREST;
        t_desc.num_mips = 0;
        t_desc.width = width;
        t_desc.height = height;
        // t_desc.num_comps = 4;
        t_desc.data[0] = data;

        // neko_tex_flip_vertically(width, height, (u8 *)(t_desc.data[0]));
        neko_texture_t tex = gfx_texture_create(t_desc);

        out->tex.id = tex.id;
        out->tex.width = width;
        out->tex.height = height;
    }

    console_log("created font range with id %d", out->tex.id);
}

static FontRange *get_range(FontFamily *font, FontKey key) {
    u64 hash = *(u64 *)&key;
    FontRange *range = font->ranges.get(hash);
    if (range == nullptr) {
        range = &font->ranges[hash];
        make_font_range(range, font, key);
    }

    return range;
}

stbtt_aligned_quad FontFamily::quad(u32 *img, float *x, float *y, float size, i32 ch) {
    FontRange *range = get_range(this, font_key(size, ch));
    assert(range != nullptr);

    ch = ch % array_size(FontRange::chars);

    float xpos = 0;
    float ypos = 0;
    stbtt_aligned_quad q = {};
    stbtt_GetBakedQuad(range->chars, (i32)range->tex.width, (i32)range->tex.height, ch, &xpos, &ypos, &q, 1);

    stbtt_bakedchar *baked = range->chars + ch;
    *img = range->tex.id;
    *x = *x + baked->xadvance;
    return q;
}

float FontFamily::width(float size, String text) {
    float width = 0;
    for (Rune r : UTF8(text)) {
        u32 code = r.charcode();
        FontRange *range = get_range(this, font_key(size, code));
        assert(range != nullptr);

        const stbtt_bakedchar *baked = range->chars + (code % array_size(FontRange::chars));
        width += baked->xadvance;
    }
    return width;
}

FontFamily *neko_default_font() {
    if (g_app->default_font == nullptr) {
        CVAR_REF(conf_default_font, String);
        g_app->default_font = (FontFamily *)mem_alloc(sizeof(FontFamily));
        g_app->default_font->load(conf_default_font.data.str);
    }
    return g_app->default_font;
}