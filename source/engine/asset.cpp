#include "engine/asset.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <cstdio>
#include <filesystem>

#include "engine/asset.h"
#include "engine/base.hpp"
#include "base/common/os.hpp"
#include "base/common/profiler.hpp"
#include "base/common/vfs.hpp"
#include "engine/bootstrap.h"
#include "engine/draw.h"
#include "engine/edit.h"
#include "base/scripting/lua_wrapper.hpp"
#include "extern/glad/glad.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// deps
#include "extern/stb_image.h"

// miniz
#include "extern/miniz.h"

#include "extern/cute_aseprite.h"

using namespace Neko::luabind;

#if 0

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
                StringScanner scan = line;
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

                StringScanner scan = line;
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

    LOG_INFO("created atlas with image id: {} and {} entries", img.id, (unsigned long long)by_name.load);

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

Assets g_assets = {};

static void hot_reload_thread(void *) {
    u32 reload_interval = gBase.reload_interval.load() * 1000;

    while (true) {
        PROFILE_BLOCK("hot reload");

        {
            LockGuard<Mutex> lock{g_assets.shutdown_mtx};
            if (g_assets.shutdown) {
                break;
            }

            bool signaled = g_assets.shutdown_notify.wait_for(lock, std::chrono::milliseconds(reload_interval), [] { return g_assets.shutdown; });
            if (signaled) {
                break;
            }
        }

        {
            PROFILE_BLOCK("check for updates");

            g_assets.rw_lock.shared_lock();
            neko_defer(g_assets.rw_lock.shared_unlock());

            g_assets.tmp_changes.len = 0;

            for (auto [k, v] : g_assets.table) {
                PROFILE_BLOCK("read modtime");

                u64 modtime = vfs_file_modtime(v->name);
                if (modtime > v->modtime) {
                    FileChange change = {};
                    change.key = v->hash;
                    change.modtime = modtime;

                    g_assets.tmp_changes.push(change);
                }
            }
        }

        if (g_assets.tmp_changes.len > 0) {
            LockGuard<Mutex> lock{g_assets.changes_mtx};
            for (FileChange change : g_assets.tmp_changes) {
                g_assets.changes.push(change);
            }
        }
    }

    LOG_INFO("hot_reload_thread end");
}

int assets_perform_hot_reload_changes(App *app, event_t evt) {
    LockGuard<Mutex> lock{g_assets.changes_mtx};

    if (g_assets.changes.len == 0) {
        return 0;
    }

    PROFILE_BLOCK("perform hot reload");

    auto L = ENGINE_LUA();

    for (FileChange change : g_assets.changes) {
        Asset a = {};
        bool exists = asset_read(change.key, &a);
        neko_assert(exists);

        a.modtime = change.modtime;

        bool ok = false;
        switch (a.kind) {
            case AssetKind_LuaRef: {
                luaL_unref(L, LUA_REGISTRYINDEX, a.lua_ref);
                a.lua_ref = luax_require_script(L, a.name);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                ok = texture_update(&a.texture, a.name);
                break;
            }
            case AssetKind_AseSprite: {
                a.sprite.trash();
                ok = a.sprite.load(a.name);
                break;
            }
            case AssetKind_Tiledmap: {
                tiled_unload(&a.tiledmap);
                ok = tiled_load(&a.tiledmap, a.name.cstr(), NULL);
                break;
            }
            case AssetKind_Shader: {
                GLuint save_sid = a.shader.id;
                neko_unload_shader(&a.shader);
                ok = neko_load_shader(&a.shader, a.name);
                neko_assert(save_sid == a.shader.id);
                break;
            }
            default:
                continue;
                break;
        }

        if (!ok) {
            fatal_error(tmp_fmt("failed to hot reload: %s", a.name.data));
            return 0;
        }

        asset_write(a);
        LOG_INFO("reloaded: {}", a.name.data);
    }

    g_assets.changes.len = 0;

    return 0;
}

void assets_shutdown() {
    if (gBase.hot_reload_enabled.load()) {
        {
            LockGuard<Mutex> lock{g_assets.shutdown_mtx};
            g_assets.shutdown = true;
        }

        g_assets.shutdown_notify.notify_one();
        g_assets.reload_thread.join();
        g_assets.changes.trash();
        g_assets.tmp_changes.trash();
    }

    for (auto [k, v] : g_assets.table) {
        mem_free(v->name.data);

        switch (v->kind) {
            case AssetKind_Image:
                // v->image.trash();
                break;
            case AssetKind_AseSprite:
                v->sprite.trash();
                break;
            case AssetKind_Shader:
                neko_unload_shader(&v->shader);
                break;
            case AssetKind_Tiledmap:
                tiled_unload(&v->tiledmap);
                break;
            default:
                break;
        }
    }
    g_assets.table.trash();

    g_assets.rw_lock.trash();
}

void assets_start_hot_reload() {

    g_assets.rw_lock.make();

    if (gBase.hot_reload_enabled.load()) {
        g_assets.reload_thread.make(hot_reload_thread, nullptr);
    }
}

bool asset_load_kind(AssetKind kind, String filepath, Asset *out) {
    AssetLoadData data = {};
    data.kind = kind;

    return asset_load(data, filepath, out);
}

bool asset_load(AssetLoadData desc, String filepath, Asset *out) {
    PROFILE_FUNC();

    u64 key = fnv1a(filepath);

    {
        Asset asset = {};
        if (asset_read(key, &asset)) {
            if (out != nullptr) {
                *out = asset;
            }
            return true;
        }
    }

    {
        PROFILE_BLOCK("load new asset");

        Asset asset = {};
        asset.name = to_cstr(filepath);
        asset.hash = key;
        {
            PROFILE_BLOCK("asset modtime")
            asset.modtime = vfs_file_modtime(asset.name);
        }
        asset.kind = desc.kind;

        bool ok = false;
        switch (desc.kind) {
            case AssetKind_LuaRef: {
                asset.lua_ref = LUA_REFNIL;
                asset_write(asset);
                asset.lua_ref = luax_require_script(ENGINE_LUA(), filepath);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                ok = texture_load(&asset.texture, filepath.cstr(), desc.flip_image_vertical);
                break;
            }
            case AssetKind_AseSprite: {
                ok = asset.sprite.load(filepath);
                break;
            }
            case AssetKind_Shader: {
                ok = neko_load_shader(&asset.shader, filepath);
                break;
            }
            case AssetKind_Tiledmap: {
                ok = tiled_load(&asset.tiledmap, filepath.cstr(), NULL);
                break;
            }
            // case AssetKind_Pak:
            //     ok = asset.pak.load(filepath.data, 0, false);
            //     break;
            default:
                LOG_INFO("asset_load {} undefined", (int)desc.kind);
                break;
        }

        if (!ok) {
            mem_free(asset.name.data);
            return false;
        }

        asset_write(asset);

        if (out != nullptr) {
            *out = asset;
        }
        return true;
    }
}

bool asset_read(u64 key, Asset *out) {
    g_assets.rw_lock.shared_lock();
    neko_defer(g_assets.rw_lock.shared_unlock());

    const Asset *asset = g_assets.table.get(key);
    if (asset == nullptr) {
        return false;
    }

    *out = *asset;
    return true;
}

void asset_write(Asset asset) {
    g_assets.rw_lock.unique_lock();
    neko_defer(g_assets.rw_lock.unique_unlock());

    g_assets.table[asset.hash] = asset;
}

Asset check_asset(lua_State *L, u64 key) {
    Asset asset = {};
    if (!asset_read(key, &asset)) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}

Asset check_asset_mt(lua_State *L, i32 arg, const char *mt) {
    u64 *udata = (u64 *)luaL_checkudata(L, arg, mt);

    Asset asset = {};
    bool ok = asset_read(*udata, &asset);
    if (!ok) {
        luaL_error(L, "cannot read asset");
    }

    return asset;
}

char *file_pathabs(const char *pathfile) {
    String out = tmp_fmt("%*.s", DIR_MAX + 1, "");
#ifdef NEKO_IS_WIN32
    _fullpath(out.data, pathfile, DIR_MAX);
#else
    realpath(pathfile, out.data);
#endif
    return out.data;
}

bool AseSpriteData::load(String filepath) {
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

    Slice<AseSpriteFrame> frames = {};
    frames.resize(&arena, ase->frame_count);

    Array<char> pixels = {};
    pixels.reserve(ase->frame_count * rect);
    neko_defer(pixels.trash());

    for (i32 i = 0; i < ase->frame_count; i++) {
        ase_frame_t &frame = ase->frames[i];

        AseSpriteFrame sf = {};
        sf.duration = frame.duration_milliseconds;

        sf.u0 = 0;
        sf.v0 = (float)i / ase->frame_count;
        sf.u1 = 1;
        sf.v1 = (float)(i + 1) / ase->frame_count;

        frames[i] = sf;
        memcpy(pixels.data + (i * rect), &frame.pixels[0].r, rect);
    }

    // sg_image_desc desc = {};
    int ase_width = ase->w;
    int ase_height = ase->h * ase->frame_count;
    // desc.data.subimage[0][0].ptr = pixels.data;
    // desc.data.subimage[0][0].size = ase->frame_count * rect;

    u8 *data = reinterpret_cast<u8 *>(pixels.data);

    // stbi_write_png("h.png", ase_width, ase_height, 4, data, 0);

    AssetTexture new_tex = NEKO_DEFAULT_VAL();
    {
        PROFILE_BLOCK("make image");

        neko_init_texture_from_memory_uncompressed(&new_tex, data, ase_width, ase_height, 4, NEKO_TEXTURE_ALIASED);
    }

    HashMap<AseSpriteLoop> by_tag = {};
    by_tag.reserve(ase->tag_count);

    for (i32 i = 0; i < ase->tag_count; i++) {
        ase_tag_t &tag = ase->tags[i];

        u64 len = (u64)((tag.to_frame + 1) - tag.from_frame);

        AseSpriteLoop loop = {};

        loop.indices.resize(&arena, len);
        for (i32 j = 0; j < len; j++) {
            loop.indices[j] = j + tag.from_frame;
        }

        by_tag[fnv1a(tag.name)] = loop;
    }

    LOG_INFO("created sprite with image id: {} and {} frames", new_tex.id, (unsigned long long)frames.len);

    AseSpriteData s = {};
    s.arena = arena;
    s.tex = new_tex;
    s.frames = frames;
    s.by_tag = by_tag;
    s.width = ase->w;
    s.height = ase->h;
    *this = s;
    return true;
}

void AseSpriteData::trash() {
    by_tag.trash();
    arena.trash();
}

void AseSprite::make() {}

bool AseSprite::play(String tag) {
    u64 key = fnv1a(tag);
    bool same = loop == key;
    loop = key;
    return same;
}

void AseSprite::update(float dt) {
    AseSpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    i32 index = view.frame();
    AseSpriteFrame frame = view.data.frames[index];

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

void AseSprite::set_frame(i32 frame) {
    AseSpriteView view = {};
    bool ok = view.make(this);
    if (!ok) {
        return;
    }

    if (0 <= frame && frame < view.len()) {
        current_frame = frame;
        elapsed = 0;
    }
}

bool AseSpriteView::make(AseSprite *spr) {
    Asset a = {};
    bool ok = asset_read(spr->sprite, &a);
    if (!ok) {
        return false;
    }

    AseSpriteData data = a.sprite;
    const AseSpriteLoop *res = data.by_tag.get(spr->loop);

    AseSpriteView view = {};
    view.sprite = spr;
    view.data = data;

    if (res != nullptr) {
        view.loop = *res;
    }

    *this = view;
    return true;
}

i32 AseSpriteView::frame() {
    if (loop.indices.data != nullptr) {
        return loop.indices[sprite->current_frame];
    } else {
        return sprite->current_frame;
    }
}

u64 AseSpriteView::len() {
    if (loop.indices.data != nullptr) {
        return loop.indices.len;
    } else {
        return data.frames.len;
    }
}
