#include "engine/font.h"

#include "engine/asset.h"
#include "engine/game.h"

// deps
#include <stb_image_write.h>

#include <format>

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

void FontFamily::load_default() {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, "assets/fonts/Monocraft.ttf");

    neko_assert(ok, "load default font failed");

    FontFamily f = {};
    f.ttf = contents;
    f.sb = {};
    *this = f;
}

void FontFamily::trash() {
    for (auto [k, v] : ranges) {
        // v->image.trash();
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
    i32 height = 512;

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

    static int iii = 0;
    stbi_write_png(std::format("font_bitmap_{}.png", iii).c_str(), width, height, 4, image_data, 0);

    {
        PROFILE_BLOCK("make image");

        // sg_image_desc sg_image = {};
        // sg_image.width = width;
        // sg_image.height = height;
        // sg_image.data.subimage[0][0].ptr = image;
        // sg_image.data.subimage[0][0].size = width * height * 4;

        // Texture tex = {};
        // tex.width = width;
        // tex.height = height;
        // tex.flip_image_vertical = true;

        {
            // LockGuard lock{&g_app->gpu_mtx};
            // id = sg_make(sg_image).id;

            // out->tex.id = id;
            out->tex.width = width;
            out->tex.height = height;
            // out->tex.flip_image_vertical = true;

            texture_update_data(&out->tex, image_data);
            // out->tex.id = generate_texture_handle(image_data, width, height, NULL);
        }
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