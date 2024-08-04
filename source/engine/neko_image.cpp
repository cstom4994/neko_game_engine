

#include "engine/neko.hpp"
#include "engine/neko_asset.h"
#include "engine/neko_game.h"


// deps
#include <stb_image.h>
#include <stb_image_resize2.h>

#define CUTE_ASEPRITE_ASSERT NEKO_ASSERT
#define CUTE_ASEPRITE_ALLOC(size, ctx) mem_alloc(size)
#define CUTE_ASEPRITE_FREE(mem, ctx) mem_free(mem)

#define CUTE_ASEPRITE_IMPLEMENTATION
#include <cute_aseprite.h>

// template <typename T>
// inline auto sg_make(const T &d) {
//     if constexpr (std::is_same_v<T, sg_image_desc>) {
//         return sg_make_image(d);
//     } else {
//         static_assert(neko::always_false<T>, "unsupported type passed to sg_make");
//     }
// }

static std::tuple<u32, i32, i32> image_load_stbi(String contents, bool generate_mips) {
    i32 width = 0, height = 0, channels = 0;
    u32 id = 0;
    stbi_uc *data = nullptr;
    {
        PROFILE_BLOCK("stb_image load");
        data = stbi_load_from_memory((u8 *)contents.data, (i32)contents.len, &width, &height, &channels, 4);
    }
    if (!data) {
        return std::make_tuple(id, width, height);
    }
    neko_defer(stbi_image_free(data));

    // sg_image_desc desc = {};
    // desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    // desc.width = width;
    // desc.height = height;
    // desc.data.subimage[0][0].ptr = data;
    // desc.data.subimage[0][0].size = width * height * 4;

    Array<u8 *> mips = {};
    neko_defer({
        for (u8 *mip : mips) {
            mem_free(mip);
        }
        mips.trash();
    });

    if (generate_mips) {
        // mips.reserve(SG_MAX_MIPMAPS);

        u8 *prev = data;
        i32 w0 = width;
        i32 h0 = height;
        i32 w1 = w0 / 2;
        i32 h1 = h0 / 2;

        while (w1 > 1 && h1 > 1) {
            PROFILE_BLOCK("generate mip");

            u8 *mip = (u8 *)mem_alloc(w1 * h1 * 4);
            stbir_resize_uint8_linear(prev, w0, h0, 0, mip, w1, h1, 0, STBIR_RGBA);
            mips.push(mip);

            // desc.data.subimage[0][mips.len].ptr = mip;
            // desc.data.subimage[0][mips.len].size = w1 * h1 * 4;

            prev = mip;
            w0 = w1;
            h0 = h1;
            w1 /= 2;
            h1 /= 2;
        }
    }

    // desc.num_mipmaps = mips.len + 1;

    {
        PROFILE_BLOCK("make image");
        LockGuard lock{&g_app->gpu_mtx};
        // id = sg_make(desc).id;
    }

    NEKO_TRACE("created image(stbi) (%dx%d, %d channels, mipmaps: %s) with id %d", width, height, channels, generate_mips ? "true" : "false", id);

    return std::make_tuple(id, width, height);
}

void neko_aseprite_default_blend_bind(ase_t *ase) {

    NEKO_ASSERT(ase);
    NEKO_ASSERT(ase->frame_count);

    // 为了方便起见，将所有单元像素混合到各自的帧中
    for (int i = 0; i < ase->frame_count; ++i) {
        ase_frame_t *frame = ase->frames + i;
        if (frame->pixels != NULL) mem_free(frame->pixels);
        frame->pixels = (ase_color_t *)mem_alloc((size_t)(sizeof(ase_color_t)) * ase->w * ase->h);
        memset(frame->pixels, 0, sizeof(ase_color_t) * (size_t)ase->w * (size_t)ase->h);
        ase_color_t *dst = frame->pixels;

        NEKO_DEBUG_LOG("neko_aseprite_default_blend_bind: frame: %d cel_count: %d", i, frame->cel_count);

        for (int j = 0; j < frame->cel_count; ++j) {  //

            ase_cel_t *cel = frame->cels + j;

            if (!(cel->layer->flags & ASE_LAYER_FLAGS_VISIBLE) || (cel->layer->parent && !(cel->layer->parent->flags & ASE_LAYER_FLAGS_VISIBLE))) {
                continue;
            }

            while (cel->is_linked) {
                ase_frame_t *frame = ase->frames + cel->linked_frame_index;
                int found = 0;
                for (int k = 0; k < frame->cel_count; ++k) {
                    if (frame->cels[k].layer == cel->layer) {
                        cel = frame->cels + k;
                        found = 1;
                        break;
                    }
                }
                NEKO_ASSERT(found);
            }
            void *src = cel->pixels;
            u8 opacity = (u8)(cel->opacity * cel->layer->opacity * 255.0f);
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -NEKO_MIN(cx, 0);
            int ct = -NEKO_MIN(cy, 0);
            int dl = NEKO_MAX(cx, 0);
            int dt = NEKO_MAX(cy, 0);
            int dr = NEKO_MIN(ase->w, cw + cx);
            int db = NEKO_MIN(ase->h, ch + cy);
            int aw = ase->w;
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++) {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++) {
                    int dst_index = aw * dy + dx;
                    ase_color_t src_color = s_color(ase, src, cw * sy + sx);
                    ase_color_t dst_color = dst[dst_index];
                    ase_color_t result = s_blend(src_color, dst_color, opacity);
                    dst[dst_index] = result;
                }
            }
        }
    }
}

static std::tuple<u32, i32, i32> image_load_ase(String contents, bool generate_mips) {
    i32 width = 0, height = 0, channels = 0;
    u32 id = 0;

    ase_t *ase;
    {
        PROFILE_BLOCK("ase_image load");
        ase = cute_aseprite_load_from_memory(contents.data, contents.len, nullptr);

        NEKO_ASSERT(ase->frame_count == 1);  // image_load_ase 用于加载简单的单帧 aseprite
        // neko_aseprite_default_blend_bind(ase);

        width = ase->w;
        height = ase->h;
    }

    if (width == 0 || height == 0) {
        return std::make_tuple(id, width, height);
    }
    neko_defer(cute_aseprite_free(ase));

    u8 *data = reinterpret_cast<u8 *>(ase->frames->pixels);

    // sg_image_desc desc = {};
    // desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    // desc.width = width;
    // desc.height = height;
    // desc.data.subimage[0][0].ptr = data;
    // desc.data.subimage[0][0].size = width * height * 4;

    Array<u8 *> mips = {};
    neko_defer({
        for (u8 *mip : mips) {
            mem_free(mip);
        }
        mips.trash();
    });

    if (generate_mips) {
        // mips.reserve(SG_MAX_MIPMAPS);

        u8 *prev = data;
        i32 w0 = width;
        i32 h0 = height;
        i32 w1 = w0 / 2;
        i32 h1 = h0 / 2;

        while (w1 > 1 && h1 > 1) {
            PROFILE_BLOCK("generate mip");

            u8 *mip = (u8 *)mem_alloc(w1 * h1 * 4);
            stbir_resize_uint8_linear(prev, w0, h0, 0, mip, w1, h1, 0, STBIR_RGBA);
            mips.push(mip);

            // desc.data.subimage[0][mips.len].ptr = mip;
            // desc.data.subimage[0][mips.len].size = w1 * h1 * 4;

            prev = mip;
            w0 = w1;
            h0 = h1;
            w1 /= 2;
            h1 /= 2;
        }
    }

    // desc.num_mipmaps = mips.len + 1;

    {
        PROFILE_BLOCK("make image");
        LockGuard lock{&g_app->gpu_mtx};
        // id = sg_make(desc).id;
    }

    NEKO_TRACE("created image(ase) (%dx%d, %d channels, mipmaps: %s) with id %d", width, height, channels, generate_mips ? "true" : "false", id);

    return std::make_tuple(id, width, height);
}

bool Image::load(String filepath, bool generate_mips) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    u32 id = 0;
    i32 width = 0, height = 0;
    if (filepath.ends_with(".ase")) {
        std::tie(id, width, height) = image_load_ase(contents, generate_mips);
    } else {
        std::tie(id, width, height) = image_load_stbi(contents, generate_mips);
    }

    if (id == 0) {
        NEKO_WARN("unsupported image type: %s", filepath.cstr());
        return false;
    }

    Image img = {};
    img.id = id;
    img.width = width;
    img.height = height;
    img.has_mips = generate_mips;
    *this = img;

    return true;
}

void Image::trash() {
    LockGuard lock{&g_app->gpu_mtx};
    // sg_destroy_image({id});
}

#if 0

bool SpriteData::load(String filepath) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    ase_t *ase = nullptr;
    {
        PROFILE_BLOCK("aseprite load");
        ase = cute_aseprite_load_from_memory(contents.data, (i32)contents.len, nullptr);
    }
    neko_defer(cute_aseprite_free(ase));

    Arena arena = {};

    i32 rect = ase->w * ase->h * 4;

    Slice<SpriteFrame> frames = {};
    frames.resize(&arena, ase->frame_count);

    Array<char> pixels = {};
    pixels.reserve(ase->frame_count * rect);
    neko_defer(pixels.trash());

    for (i32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t &frame = ase->frames[i];

        SpriteFrame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (float)i / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (float)(i + 1) / ase->frame_count;

        frames[i] = sf;
        memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
    }

    // sg_image_desc desc = {};
    // desc.width = ase->w;
    // desc.height = ase->h * ase->frame_count;
    // desc.data.subimage[0][0].ptr = pixels.data;
    // desc.data.subimage[0][0].size = ase->frame_count * rect;

    u32 id = 0;
    {
        PROFILE_BLOCK("make image");
        LockGuard lock{&g_app->gpu_mtx};
        // id = sg_make(desc).id;
    }

    Image img = {};
    img.id = id;
    // img.width = desc.width;
    // img.height = desc.height;

    HashMap<SpriteLoop> by_tag = {};
    by_tag.reserve(ase->tag_count);

    for (i32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t &tag = ase->tags[i];

        u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

        SpriteLoop loop = {};

        loop.indices.resize(&arena, len);
        for (i32 j = 0; j < len; j++) {
            loop.indices[j] = j + tag.from_frame;
        }

        by_tag[fnv1a(tag.name)] = loop;
    }

    NEKO_TRACE("created sprite with image id: %d and %llu frames", img.id, (unsigned long long)frames.len);

    SpriteData s = {};
    s.arena = arena;
    s.img = img;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *this = s;
    return true;
}

void SpriteData::trash() {
    by_tag.trash();
    arena.trash();
}

bool Sprite::play(String tag) {
    u64 key = fnv1a(tag);
    bool same = loop == key;
    loop = key;
    return same;
}

void Sprite::update(float dt) {
    SpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    i32 index = view.frame();
    SpriteFrame frame = view.data.frames[index];

    elapsed += dt * 1000;
    if (elapsed > frame.duration) {
        if (current_frame == view.len() - 1) {
            current_frame = 0;
        } else {
            current_frame++;
        }

        elapsed -= frame.duration;
    }
}

void Sprite::set_frame(i32 frame) {
    SpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    if (0 <= frame && frame < view.len()) {
        current_frame = frame;
        elapsed = 0;
    }
}

bool SpriteView::make(Sprite *spr) {
    Asset a = {};
    bool ok = asset_read(spr->sprite, &a);
    if (!ok) {
        return false;
    }

    SpriteData data = a.sprite;
    const SpriteLoop *res = data.by_tag.get(spr->loop);

    SpriteView view = {};
    view.sprite = spr;
    view.data = data;

    if (res != nullptr) {
        view.loop = *res;
    }

    *this = view;
    return true;
}

i32 SpriteView::frame() {
    if (loop.indices.data != nullptr) {
        return loop.indices[sprite->current_frame];
    } else {
        return sprite->current_frame;
    }
}

u64 SpriteView::len() {
    if (loop.indices.data != nullptr) {
        return loop.indices.len;
    } else {
        return data.frames.len;
    }
}

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
    bool ok = vfs_read_entire_file(&contents, NEKO_PACKS::DEFAULT_FONT);

    NEKO_ASSERT(ok, "load default font failed");

    FontFamily f = {};
    f.ttf = contents;
    f.sb = {};
    *this = f;
}

void FontFamily::trash() {
    for (auto [k, v] : ranges) {
        v->image.trash();
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

    i32 width = 256;
    i32 height = 256;

    u8 *bitmap = nullptr;
    while (bitmap == nullptr) {
        PROFILE_BLOCK("try bake");

        bitmap = (u8 *)mem_alloc(width * height);
        i32 res = stbtt_BakeFontBitmap((u8 *)font->ttf.data, 0, key.size, bitmap, width, height, key.ch, array_size(out->chars), out->chars);
        if (res < 0) {
            mem_free(bitmap);
            bitmap = nullptr;
            width *= 2;
            height *= 2;
        }
    }
    neko_defer(mem_free(bitmap));

    u8 *image = (u8 *)mem_alloc(width * height * 4);
    neko_defer(mem_free(image));

    {
        PROFILE_BLOCK("convert rgba");

        for (i32 i = 0; i < width * height * 4; i += 4) {
            image[i + 0] = 255;
            image[i + 1] = 255;
            image[i + 2] = 255;
            image[i + 3] = bitmap[i / 4];
        }
    }

    u32 id = 0;
    {
        PROFILE_BLOCK("make image");

        // sg_image_desc sg_image = {};
        // sg_image.width = width;
        // sg_image.height = height;
        // sg_image.data.subimage[0][0].ptr = image;
        // sg_image.data.subimage[0][0].size = width * height * 4;

        {
            LockGuard lock{&g_app->gpu_mtx};
            // id = sg_make(sg_image).id;
        }
    }

    out->image.id = id;
    out->image.width = width;
    out->image.height = height;

    NEKO_TRACE("created font range with id %d", id);
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
    stbtt_GetBakedQuad(range->chars, (i32)range->image.width, (i32)range->image.height, ch, &xpos, &ypos, &q, 1);

    stbtt_bakedchar *baked = range->chars + ch;
    *img = range->image.id;
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

bool Atlas::load(String filepath, bool generate_mips) {
    PROFILE_FUNC();

    String contents = {};
    bool ok = vfs_read_entire_file(&contents, filepath);
    if (!ok) {
        return false;
    }
    neko_defer(mem_free(contents.data));

    Image img = {};
    HashMap<AtlasImage> by_name = {};

    for (String line : SplitLines(contents)) {
        switch (line.data[0]) {
            case 'a': {
                Scanner scan = line;
                scan.next_string();  // discard 'a'
                String filename = scan.next_string();

                StringBuilder sb = {};
                neko_defer(sb.trash());
                sb.swap_filename(filepath, filename);
                bool ok = img.load(String(sb), generate_mips);
                if (!ok) {
                    return false;
                }
                break;
            }
            case 's': {
                if (img.id == 0) {
                    return false;
                }

                Scanner scan = line;
                scan.next_string();  // discard 's'
                String name = scan.next_string();
                scan.next_string();  // discard origin x
                scan.next_string();  // discard origin y
                i32 x = scan.next_int();
                i32 y = scan.next_int();
                i32 width = scan.next_int();
                i32 height = scan.next_int();
                i32 padding = scan.next_int();
                i32 trimmed = scan.next_int();
                scan.next_int();  // discard trim x
                scan.next_int();  // discard trim y
                i32 trim_width = scan.next_int();
                i32 trim_height = scan.next_int();

                AtlasImage atlas_img = {};
                atlas_img.img = img;
                atlas_img.u0 = (x + padding) / (float)img.width;
                atlas_img.v0 = (y + padding) / (float)img.height;

                if (trimmed != 0) {
                    atlas_img.width = (float)trim_width;
                    atlas_img.height = (float)trim_height;
                    atlas_img.u1 = (x + padding + trim_width) / (float)img.width;
                    atlas_img.v1 = (y + padding + trim_height) / (float)img.height;
                } else {
                    atlas_img.width = (float)width;
                    atlas_img.height = (float)height;
                    atlas_img.u1 = (x + padding + width) / (float)img.width;
                    atlas_img.v1 = (y + padding + height) / (float)img.height;
                }

                by_name[fnv1a(name)] = atlas_img;

                break;
            }
            default:
                break;
        }
    }

    NEKO_TRACE("created atlas with image id: %d and %llu entries", img.id, (unsigned long long)by_name.load);

    Atlas a;
    a.by_name = by_name;
    a.img = img;
    *this = a;

    return true;
}

void Atlas::trash() {
    by_name.trash();
    img.trash();
}

AtlasImage *Atlas::get(String name) {
    u64 key = fnv1a(name);
    return by_name.get(key);
}

#endif