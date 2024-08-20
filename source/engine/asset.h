#pragma once

#include "engine/base.h"
#include "engine/math.h"
#include "engine/prelude.h"
#include "engine/sprite.h"
#include "engine/texture.h"
#include "engine/tilemap.h"

enum AssetKind : i32 {
    AssetKind_None,
    AssetKind_LuaRef,
    AssetKind_Image,
    AssetKind_AseSprite,
    AssetKind_Tilemap,
    // AssetKind_Pak,
};

struct AssetLoadData {
    AssetKind kind;
    bool flip_image_vertical;
};

struct Asset {
    String name;
    u64 hash;
    u64 modtime;
    AssetKind kind;
    union {
        i32 lua_ref;
        AssetTexture texture;
        AseSpriteData sprite;
        MapLdtk tilemap;
        // Pak pak;
    };
};

struct FileChange {
    u64 key;
    u64 modtime;
};

struct Assets {
    HashMap<Asset> table;
    RWLock rw_lock;

    Mutex shutdown_mtx;
    Cond shutdown_notify;
    bool shutdown;

    Thread reload_thread;

    Mutex changes_mtx;
    Array<FileChange> changes;
    Array<FileChange> tmp_changes;
};

void assets_shutdown();
void assets_start_hot_reload();
void assets_perform_hot_reload_changes();

bool asset_load_kind(AssetKind kind, String filepath, Asset *out);
bool asset_load(AssetLoadData desc, String filepath, Asset *out);

bool asset_read(u64 key, Asset *out);
void asset_write(Asset asset);

struct lua_State;
Asset check_asset(lua_State *L, u64 key);
Asset check_asset_mt(lua_State *L, i32 arg, const char *mt);

char *file_pathabs(const char *pathfile);