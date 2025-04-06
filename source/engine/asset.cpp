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

using namespace Neko::luabind;

static void hot_reload_thread(void *) {
    u32 reload_interval = gBase.reload_interval.load() * 1000;

    Assets &g_assets = the<Assets>();

    while (true) {
        PROFILE_BLOCK("hot reload");

        {
            LockGuard<Mutex> lock{g_assets.shutdown_mtx};
            if (g_assets.shutdown) {
                break;
            }

            bool signaled = g_assets.shutdown_notify.wait_for(lock, std::chrono::milliseconds(reload_interval), [&] { return g_assets.shutdown; });
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
                if (v->is_internal) continue;  // 内部资源不参与热更新

                u64 modtime = the<VFS>().file_modtime(v->name);
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

int assets_perform_hot_reload_changes(Event evt) {
    Assets &g_assets = the<Assets>();

    LockGuard<Mutex> lock{g_assets.changes_mtx};

    if (g_assets.changes.len == 0) {
        return 0;
    }

    PROFILE_BLOCK("perform hot reload");

    auto L = ENGINE_LUA();

    for (FileChange change : g_assets.changes) {
        Asset asset = {};
        bool exists = asset_read(change.key, &asset);
        neko_assert(exists);

        asset.modtime = change.modtime;

        bool ok = false;
        switch (asset.kind) {
            case AssetKind_LuaRef: {
                luaL_unref(L, LUA_REGISTRYINDEX, assets_get<LuaRefID>(asset));
                asset.data = luax_require_script(L, asset.name);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                ok = texture_update(&assets_get<AssetTexture>(asset), asset.name);
                break;
            }
            case AssetKind_AseSprite: {
                assets_get<AseSpriteData>(asset).trash();
                ok = assets_get<AseSpriteData>(asset).load(asset.name);
                break;
            }
            case AssetKind_Tiledmap: {
                tiled_unload(&assets_get<TiledMap>(asset));
                ok = tiled_load(&assets_get<TiledMap>(asset), asset.name.cstr(), NULL);
                break;
            }
            case AssetKind_Shader: {
                GLuint save_sid = assets_get<AssetShader>(asset).id;
                neko_unload_shader(&assets_get<AssetShader>(asset));
                ok = neko_load_shader(&assets_get<AssetShader>(asset), asset.name);
                neko_assert(save_sid == assets_get<AssetShader>(asset).id);
                break;
            }
            case AssetKind_Text: {
                mem_free(assets_get<String>(asset).data);
                ok = the<VFS>().read_entire_file(&assets_get<String>(asset), asset.name);
                break;
            }
            default:
                continue;
                break;
        }

        if (!ok) {
            gBase.fatal_error(tmp_fmt("failed to hot reload: %s", asset.name.data));
            return 0;
        }

        asset_write(asset);
        LOG_INFO("reloaded: {}", asset.name.data);
    }

    g_assets.changes.len = 0;

    return 0;
}

void assets_shutdown() {
    Assets &g_assets = the<Assets>();

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

        Asset &asset = *v;
        switch (asset.kind) {
            case AssetKind_Image:
                texture_release(&assets_get<AssetTexture>(asset));
                break;
            case AssetKind_AseSprite:
                assets_get<AseSpriteData>(asset).trash();
                break;
            case AssetKind_Shader:
                neko_unload_shader(&assets_get<AssetShader>(asset));
                break;
            case AssetKind_Tiledmap:
                tiled_unload(&assets_get<TiledMap>(asset));
                break;
            case AssetKind_Text:
                mem_free(assets_get<String>(asset).data);
                break;
            default:
                break;
        }
    }
    g_assets.table.trash();

    g_assets.rw_lock.trash();
}

void assets_start_hot_reload() {
    Assets &g_assets = the<Assets>();

    g_assets.rw_lock.make();

    if (gBase.hot_reload_enabled.load()) {
        g_assets.reload_thread.make(hot_reload_thread, nullptr);
    }
}

bool asset_sync_internal(String name, Asset sync, AssetKind kind) {
    PROFILE_FUNC();

    u64 key = fnv1a(name);

    sync.name = to_cstr(name);
    sync.is_internal = true;
    sync.hash = key;
    sync.kind = kind;
    sync.modtime = 0;

    {
        Asset asset = {};
        if (asset_read(key, &asset)) {
            if (!asset.is_internal) LOG_WARN("a non-internal asset {} is replaced", asset.name);
        }
    }

    asset_write(sync);

    return true;
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
            asset.modtime = the<VFS>().file_modtime(asset.name);
        }
        asset.kind = desc.kind;
        asset.is_internal = false;

        bool ok = false;
        switch (desc.kind) {
            case AssetKind_LuaRef: {
                asset.data = LUA_REFNIL;
                asset_write(asset);
                asset.data = luax_require_script(ENGINE_LUA(), filepath);
                ok = true;
                break;
            }
            case AssetKind_Image: {
                ok = texture_load(&assets_get<AssetTexture>(asset), filepath.cstr(), desc.flip_image_vertical);
                break;
            }
            case AssetKind_AseSprite: {
                ok = assets_get<AseSpriteData>(asset).load(filepath);
                break;
            }
            case AssetKind_Shader: {
                ok = neko_load_shader(&assets_get<AssetShader>(asset), filepath);
                break;
            }
            case AssetKind_Tiledmap: {
                ok = tiled_load(&assets_get<TiledMap>(asset), filepath.cstr(), NULL);
                break;
            }
            case AssetKind_Text: {
                ok = the<VFS>().read_entire_file(&assets_get<String>(asset), filepath);
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

Array<Asset> asset_view(AssetKind kind) {
    Assets &g_assets = the<Assets>();

    g_assets.rw_lock.shared_lock();
    neko_defer(g_assets.rw_lock.shared_unlock());

    Array<Asset> views{};
    for (auto &v : g_assets.table) {
        if (v.value->kind == kind) views.push(*v.value);
    }
    return views;
}

bool asset_read(u64 key, Asset *out) {
    Assets &g_assets = the<Assets>();

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
    Assets &g_assets = the<Assets>();

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
