#ifndef NEKO_ASSET_H
#define NEKO_ASSET_H

#include <bitset>

#include "engine/base.hpp"
#include "base/common/color.hpp"
#include "base/common/vfs.hpp"
#include "base/common/xml.hpp"
#include "engine/scripting/scripting.h"
#include "engine/ecs/entity.h"
#include "engine/event.h"
#include "engine/graphics.h"
#include "engine/sprite.h"
#include "engine/renderer/shader.h"

using namespace Neko;
using namespace Neko::luabind;

typedef struct tile_t {
    u32 id;
    u32 tileset_id;
} tile_t;

typedef struct tileset_t {
    AssetTexture texture;
    u32 tile_count;
    u32 tile_width;
    u32 tile_height;
    u32 first_gid;

    u32 width, height;
} tileset_t;

typedef struct layer_t {
    tile_t* tiles;
    u32 width;
    u32 height;

    Color256 tint;
} layer_t;

struct object_property_t {
    String key;
    String value;
};

typedef struct object_t {
    u32 id;
    String name;
    String class_name;
    i32 x, y, width, height, visible;
    Array<object_property_t> properties;
} object_t;

typedef struct object_group_t {
    Array<object_t> objects;

    Color256 color;

    String name;
} object_group_t;

typedef struct TiledMap {
    XMLDoc doc;  // xml doc
    Array<tileset_t> tilesets;
    Array<object_group_t> object_groups;
    Array<layer_t> layers;
} TiledMap;

enum AssetKind : i32 {
    AssetKind_None,
    AssetKind_LuaRef,
    AssetKind_Image,
    AssetKind_AseSprite,
    AssetKind_Tiledmap,
    AssetKind_Shader,
    AssetKind_Text,
};

struct AssetLoadData {
    AssetKind kind;
    bool flip_image_vertical;
};

using AssetVariant = std::variant<std::monostate, LuaRefID, AssetTexture, AseSpriteData, TiledMap, AssetShader, String>;

struct Asset {
    String name;
    u64 hash;
    u64 modtime;
    AssetKind kind;
    bool is_internal;
    AssetVariant data;
};

template <typename T>
T& assets_get(Asset& asset) {
    if (!std::holds_alternative<T>(asset.data)) {
        asset.data = T{};
    }
    return std::get<T>(asset.data);
}

template <typename T>
const T& assets_get(const Asset& asset) {
    if (!std::holds_alternative<T>(asset.data)) {
        throw std::runtime_error("failed to get asset");
    }
    return std::get<T>(asset.data);
}

struct FileChange {
    u64 key;
    u64 modtime;
};

class Assets : public SingletonClass<Assets> {
public:
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
int assets_perform_hot_reload_changes(Event evt);

bool asset_sync_internal(String name, Asset sync, AssetKind kind);
bool asset_load_kind(AssetKind kind, String filepath, Asset* out);
bool asset_load(AssetLoadData desc, String filepath, Asset* out);

Array<Asset> asset_view(AssetKind kind);

template <std::invocable<const Asset&> F>
inline void asset_view_each(F&& f) {
    Assets& g_assets = the<Assets>();

    g_assets.rw_lock.shared_lock();
    neko_defer(g_assets.rw_lock.shared_unlock());

    for (auto& [key, asset] : g_assets.table) {
        std::invoke(f, *asset);
    }
}

bool asset_read(u64 key, Asset* out);
void asset_write(Asset asset);

struct lua_State;
Asset check_asset(lua_State* L, u64 key);
Asset check_asset_mt(lua_State* L, i32 arg, const char* mt);

char* file_pathabs(const char* pathfile);

#endif