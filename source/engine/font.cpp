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
        t_desc.data = data;

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

static void draw_font_line(idraw_t *idraw, FontFamily *font, float size, float *start_x, float *start_y, String line, Color256 col) {
    float x = *start_x;
    float y = *start_y;
    for (Rune r : UTF8(line)) {
        u32 tex_id = 0;
        float xx = x;
        float yy = y;
        stbtt_aligned_quad q = font->quad(&tex_id, &xx, &yy, size, r.charcode());

        // sgl_texture({atlas}, {g_renderer.sampler});
        // sgl_begin_quads();
        // renderer_push_quad(vec4(x + q.x0, y + q.y0, x + q.x1, y + q.y1), vec4(q.s0, q.t0, q.s1, q.t1));
        // sgl_end();

        neko_idraw_texture(idraw, neko_texture_t{tex_id});
        neko_idraw_rectvx(idraw, neko_v2(x + q.x0, y + q.y0), neko_v2(x + q.x1, y + q.y1), neko_v2(q.s0, q.t0), neko_v2(q.s1, q.t1), col, R_PRIMITIVE_TRIANGLES);

        x = xx;
        y = yy;
    }

    *start_y += size;
}

float draw_font(idraw_t *idraw, FontFamily *font, float size, float x, float y, String text, Color256 col) {
    PROFILE_FUNC();

    y += size;
    // sgl_enable_texture();
    // renderer_apply_color();

    for (String line : SplitLines(text)) {
        draw_font_line(idraw, font, size, &x, &y, line, col);
    }

    return y - size;
}

float draw_font_wrapped(idraw_t *idraw, FontFamily *font, float size, float x, float y, String text, Color256 col, float limit) {
    PROFILE_FUNC();

    y += size;
    // sgl_enable_texture();
    // renderer_apply_color();

    for (String line : SplitLines(text)) {
        font->sb.clear();
        Scanner scan = line;

        for (String word = scan.next_string(); word != ""; word = scan.next_string()) {

            font->sb << word;

            float width = font->width(size, String(font->sb));
            if (width < limit) {
                font->sb << " ";
                continue;
            }

            font->sb.len -= word.len;
            font->sb.data[font->sb.len] = '\0';

            draw_font_line(idraw, font, size, &x, &y, String(font->sb), col);

            font->sb.clear();
            font->sb << word << " ";
        }

        draw_font_line(idraw, font, size, &x, &y, String(font->sb), col);
    }

    return y - size;
}

// mt_font

static FontFamily *check_font_udata(lua_State *L, i32 arg) {
    FontFamily **udata = (FontFamily **)luaL_checkudata(L, arg, "mt_font");
    FontFamily *font = *udata;
    return font;
}

static int mt_font_gc(lua_State *L) {
    FontFamily *font = check_font_udata(L, 1);

    if (font != g_app->default_font) {
        font->trash();
        mem_free(font);
    }
    return 0;
}

static int mt_font_width(lua_State *L) {
    FontFamily *font = check_font_udata(L, 1);

    String text = luax_check_string(L, 2);
    lua_Number size = luaL_checknumber(L, 3);

    float w = font->width(size, text);

    lua_pushnumber(L, w);
    return 1;
}

static int mt_font_draw(lua_State *L) {
    FontFamily *font = check_font_udata(L, 1);

    String text = luax_check_string(L, 2);
    lua_Number x = luaL_optnumber(L, 3, 0);
    lua_Number y = luaL_optnumber(L, 4, 0);
    lua_Number size = luaL_optnumber(L, 5, 12);
    lua_Number wrap = luaL_optnumber(L, 6, -1);

    idraw_t *idraw = &g_app->idraw;

    float bottom = 0;
    if (wrap < 0) {
        bottom = draw_font(idraw, font, (u64)size, (float)x, (float)y, text, NEKO_COLOR_WHITE);
    } else {
        bottom = draw_font_wrapped(idraw, font, (u64)size, (float)x, (float)y, text, NEKO_COLOR_WHITE, (float)wrap);
    }

    lua_pushnumber(L, bottom);
    return 1;
}

int open_mt_font(lua_State *L) {
    luaL_Reg reg[] = {
            {"__gc", mt_font_gc},
            {"width", mt_font_width},
            {"draw", mt_font_draw},
            {nullptr, nullptr},
    };

    luax_new_class(L, "mt_font", reg);
    return 0;
}

int neko_font_load(lua_State *L) {
    String str = luax_check_string(L, 1);

    FontFamily *font = (FontFamily *)mem_alloc(sizeof(FontFamily));
    bool ok = font->load(str);
    if (!ok) {
        mem_free(font);
        return 0;
    }

    luax_ptr_userdata(L, font, "mt_font");
    return 1;
}