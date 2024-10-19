
#include "engine/ecs/entity.h"

#include <stdbool.h>
#include <stdlib.h>

#include "editor/editor.hpp"
#include "engine/asset.h"
#include "engine/base.hpp"
#include "engine/base/os.hpp"
#include "engine/base/profiler.hpp"
#include "engine/bootstrap.h"
#include "engine/component.h"
#include "engine/ecs/lua_ecs.hpp"
#include "engine/edit.h"
#include "engine/graphics.h"
#include "engine/imgui.hpp"
#include "engine/input.h"
#include "engine/scripting/lua_wrapper.hpp"
#include "engine/scripting/scripting.h"
#include "engine/test.h"
#include "engine/ui.h"
#include "lua.h"

using namespace neko::luabind;
using namespace neko::ecs;

typedef enum SaveFilter {
    SF_SAVE,     // 保存此实体
    SF_NO_SAVE,  // 不要保存此实体
    SF_UNSET,    // 未设置过滤器 -- 使用默认值
} SaveFilter;

NativeEntity entity_nil = {0};
static ecs_id_t counter = 1;

// typedef struct ExistsPoolElem {
//     EntityPoolElem pool_elem;
// } ExistsPoolElem;

NativeEntity entity_create() {
    NativeEntity ent;

    auto L = ENGINE_LUA();

    LuaRef table = LuaRef::NewTable(L);
    table["testing"] = "Hello C";

    Entity* e = EcsEntityNew(L, table, NULL);

    ent.id = e - ENGINE_ECS()->entity_buf;

    return ent;
}

static void entity_remove(NativeEntity ent) { EcsEntityDel(ENGINE_LUA(), ent.id); }

void entity_destroy(NativeEntity ent) { entity_remove(ent); }

void entity_destroy_all() {
    // ExistsPoolElem* exists;
    // entitypool_foreach(exists, exists_pool) entity_destroy(exists->pool_elem.ent);
}

bool entity_destroyed(NativeEntity ent) {
    // return entitymap_get(destroyed_map, ent);
    return NULL == EcsGetEnt(ENGINE_LUA(), ENGINE_ECS(), ent.id);
}

void entity_set_save_filter(NativeEntity ent, bool filter) {
    // if (filter) {
    //     entitymap_set(save_filter_map, ent, SF_SAVE);
    //     save_filter_default = SF_NO_SAVE;
    // } else
    //     entitymap_set(save_filter_map, ent, SF_NO_SAVE);
}

bool entity_get_save_filter(NativeEntity ent) {
    // SaveFilter filter = (SaveFilter)entitymap_get(save_filter_map, ent);
    // if (filter == SF_UNSET) filter = save_filter_default;
    // return filter == SF_SAVE;
    return true;
}

void entity_clear_save_filters() {
    // entitymap_clear(save_filter_map);
    // save_filter_default = SF_SAVE;
}

void entity_init() {
    PROFILE_FUNC();

    auto L = ENGINE_LUA();

    EcsRegister(L, "C");

    {
        LuaRef table = LuaRef::NewTable(L);
        table["testing"] = "iNIT C";

        EcsEntityNew(L, table, NULL);
    }

    // exists_pool = entitypool_new(ExistsPoolElem);
    // destroyed_map = entitymap_new(false);
    // destroyed = array_new(DestroyEntry);
    // unused_map = entitymap_new(false);
    // unused = array_new(NativeEntity);
    // save_filter_map = entitymap_new(SF_UNSET);
}

void entity_fini() {
    // entitymap_free(save_filter_map);
    // array_free(unused);
    // entitymap_free(unused_map);
    // array_free(destroyed);
    // entitymap_free(destroyed_map);
    // entitypool_free(exists_pool);
}

int entity_update_all(App* app, event_t evt) {
    // ecs_id_t i;
    // DestroyEntry* entry;

    // for (i = 0; i < array_length(destroyed);) {
    //     entry = (DestroyEntry*)array_get(destroyed, i);
    //     if (entry->pass == 0) {
    //         ++entry->pass;
    //         ++i;
    //     } else {
    //         entity_remove(entry->ent);
    //         array_quick_remove(destroyed, i);
    //     }
    // }

    return 0;
}

void entity_save(NativeEntity* ent, const char* n, Store* s) {
    Store* t;

    if (!native_entity_eq(*ent, entity_nil) && !entity_get_save_filter(*ent)) error("filtered-out entity referenced in save!");

    if (store_child_save(&t, n, s)) uint_save(&ent->id, "id", t);
}

bool entity_load(NativeEntity* ent, const char* n, NativeEntity d, Store* s) {
    // Store* t;
    // ecs_id_t id;

    // if (!store_child_load(&t, n, s)) {
    //     *ent = d;
    //     return false;
    // }

    // uint_load(&id, "id", entity_nil.id, t);
    // *ent = _entity_resolve_saved_id(id);
    return true;
}

void entity_load_all_begin() {
    // load_map = entitymap_new(entity_nil.id);
}
void entity_load_all_end() {
    // entitymap_free(load_map);
    // entity_clear_save_filters();
}

#undef native_entity_eq
bool native_entity_eq(NativeEntity e, NativeEntity f) { return e.id == f.id; }

void entity_save_all(Store* s) {
    // DestroyEntry* entry;
    // ExistsPoolElem* exists;
    // Store *entity_s, *exists_s, *destroyed_s, *entry_s;

    // if (store_child_save(&entity_s, "entity", s)) {
    //     entitypool_save_foreach(exists, exists_s, exists_pool, "exists_pool", entity_s);

    //     if (store_child_save(&destroyed_s, "destroyed", entity_s)) array_foreach(entry, destroyed) if (entity_get_save_filter(entry->ent)) if (store_child_save(&entry_s, NULL, destroyed_s)) {
    //             entity_save(&entry->ent, "ent", entry_s);
    //             uint_save(&entry->pass, "pass", entry_s);
    //         }
    // }
}

void entity_load_all(Store* s) {
    // DestroyEntry* entry;
    // ExistsPoolElem* exists;
    // Store *entity_s, *exists_s, *destroyed_s, *entry_s;

    // if (store_child_load(&entity_s, "entity", s)) {
    //     entitypool_load_foreach(exists, exists_s, exists_pool, "exists_pool", entity_s);

    //     if (store_child_load(&destroyed_s, "destroyed", entity_s))
    //         while (store_child_load(&entry_s, NULL, destroyed_s)) {
    //             entry = (DestroyEntry*)array_add(destroyed);
    //             error_assert(entity_load(&entry->ent, "ent", entity_nil, entry_s));
    //             uint_load(&entry->pass, "pass", 0, entry_s);
    //         }
    // }
}

#define MIN_CAPACITY 2

static void entitymap_init_w(NativeEntityMap* emap) {
    ecs_id_t i;

    emap->bound = 0;                // bound <= capacity (so that maximum key < capacity)
    emap->capacity = MIN_CAPACITY;  // MIN_CAPACITY <= capacity
    emap->arr = (int*)mem_alloc(emap->capacity * sizeof(*emap->arr));

    for (i = 0; i < emap->capacity; ++i) emap->arr[i] = emap->def;
}

NativeEntityMap* entitymap_new(int def) {
    NativeEntityMap* emap;

    emap = (NativeEntityMap*)mem_alloc(sizeof(NativeEntityMap));
    emap->def = def;

    entitymap_init_w(emap);

    return emap;
}
void entitymap_clear(NativeEntityMap* emap) {
    mem_free(emap->arr);
    entitymap_init_w(emap);
}
void entitymap_free(NativeEntityMap* emap) {
    mem_free(emap->arr);
    mem_free(emap);
}

static void _grow(NativeEntityMap* emap) {
    ecs_id_t new_capacity, i, bound;

    // find next power of 2 (TODO: use log?)
    bound = emap->bound;
    for (new_capacity = emap->capacity; new_capacity < bound; new_capacity <<= 1);

    // grow, clear new
    emap->arr = (int*)mem_realloc(emap->arr, new_capacity * sizeof(*emap->arr));
    for (i = emap->capacity; i < new_capacity; ++i) emap->arr[i] = emap->def;
    emap->capacity = new_capacity;
}
static void _shrink(NativeEntityMap* emap) {
    ecs_id_t new_capacity, bound_times_4;

    if (emap->capacity <= MIN_CAPACITY) return;

    // halve capacity while bound is less than a fourth
    bound_times_4 = emap->bound << 2;
    if (bound_times_4 >= emap->capacity) return;
    for (new_capacity = emap->capacity; new_capacity > MIN_CAPACITY && bound_times_4 < new_capacity; new_capacity >>= 1);
    if (new_capacity < MIN_CAPACITY) new_capacity = MIN_CAPACITY;

    emap->arr = (int*)mem_realloc(emap->arr, new_capacity * sizeof(*emap->arr));
    emap->capacity = new_capacity;
}

void entitymap_set(NativeEntityMap* emap, NativeEntity ent, int val) {
    if (val == emap->def)  // 判断删除操作
    {
        emap->arr[ent.id] = val;
        // 可能会向下移动并_shrink
        if (emap->bound == ent.id + 1) {
            while (emap->bound > 0 && emap->arr[emap->bound - 1] == emap->def) --emap->bound;
            _shrink(emap);
        }
    } else {
        // 可能会受到限制并_grow
        if (ent.id + 1 > emap->bound) {
            emap->bound = ent.id + 1;
            if (ent.id >= emap->capacity) _grow(emap);
        }
        emap->arr[ent.id] = val;
    }
}
int entitymap_get(NativeEntityMap* emap, NativeEntity ent) {
    if (ent.id >= emap->capacity) return emap->def;
    return emap->arr[ent.id];
}

static void load_all_lua_scripts(lua_State* L) {
    PROFILE_FUNC();

    Array<String> files = {};
    neko_defer({
        for (String str : files) {
            mem_free(str.data);
        }
        files.trash();
    });

    bool ok = false;

#if defined(_DEBUG)
    ok = vfs_list_all_files(NEKO_PACKS::LUACODE, &files);
#else
    ok = vfs_list_all_files(NEKO_PACKS::GAMEDATA, &files);
#endif

    if (!ok) {
        neko_panic("failed to list all files");
    }
    std::qsort(files.data, files.len, sizeof(String), [](const void* a, const void* b) -> int {
        String* lhs = (String*)a;
        String* rhs = (String*)b;
        return std::strcmp(lhs->data, rhs->data);
    });

    for (String file : files) {
        if (file.starts_with("script/") && !file.ends_with("nekomain.lua") && file.ends_with(".lua")) {
            asset_load_kind(AssetKind_LuaRef, file, nullptr);
        } else {
        }
    }
}

static void _key_down(KeyCode key, int scancode, int mode) {
    gui_key_down(key);
    script_key_down(key);
}
static void _key_up(KeyCode key, int scancode, int mode) {
    gui_key_up(key);
    script_key_up(key);
}
static void _char_down(unsigned int c) { gui_char_down(c); }
static void _mouse_down(MouseCode mouse) {
    gui_mouse_down(mouse);
    script_mouse_down(mouse);
}
static void _mouse_up(MouseCode mouse) {
    gui_mouse_up(mouse);
    script_mouse_up(mouse);
}
static void _mouse_move(vec2 pos) { script_mouse_move(pos); }
static void _scroll(vec2 scroll) { script_scroll(scroll); }

void system_init() {
    PROFILE_FUNC();

#if defined(_DEBUG)
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, "../gamedir");
    MountResult mount_luacode = vfs_mount(NEKO_PACKS::LUACODE, "../source/game");
#else
    MountResult mount = vfs_mount(NEKO_PACKS::GAMEDATA, nullptr);
    MountResult mount_luacode = {true};
#endif

    script_init();

    lua_State* L = ENGINE_LUA();

    g_app->is_fused.store(mount.is_fused);

    if (!g_app->error_mode.load() && mount.ok && mount_luacode.ok) {
        asset_load_kind(AssetKind_LuaRef, "conf.lua", nullptr);
    }

    lua_newtable(L);
    i32 conf_table = lua_gettop(L);

    if (!g_app->error_mode.load()) {
        luax_neko_get(L, "conf");
        lua_pushvalue(L, conf_table);
        luax_pcall(L, 1, 0);
    }

    g_app->win_console = g_app->win_console || luax_boolean_field(L, -1, "win_console", true);

    neko::reflection::Any v = engine_cfg_t{.title = "NekoEngine", .hot_reload = true, .startup_load_scripts = true, .fullscreen = false};
    checktable_refl(ENGINE_LUA(), "app", v);
    g_app->cfg = v.cast<engine_cfg_t>();

    console_log("load game: %s %f %f", g_app->cfg.title.cstr(), g_app->cfg.width, g_app->cfg.height);

    lua_pop(L, 1);  // conf table

    auto& game = neko::the<Game>();

    // 刷新状态
    game.set_window_size(luavec2(g_app->cfg.width, g_app->cfg.height));
    game.set_window_title(g_app->cfg.title.cstr());

    if (fnv1a(g_app->cfg.game_proxy) == "default"_hash) {
        // neko::neko_lua_run_string(L, R"lua(
        // )lua");
        console_log("using default game proxy");
    }

    g_render = gfx_create();
    gfx_init(g_render);

    game.SplashScreen();

    neko_default_font();

    auto& input = neko::the<Input>();
    input.init();

    entity_init();
    transform_init();
    camera_init();
    g_app->batch = batch_init(g_app->cfg.batch_vertex_capacity);
    sprite_init();
    tiled_init();
    font_init();
    gui_init();
    console_init();
    sound_init();
    physics_init();
    edit_init();
    imgui_init();

    g_app->hot_reload_enabled.store(mount.can_hot_reload && g_app->cfg.hot_reload);
    g_app->reload_interval.store(g_app->cfg.reload_interval);

    luax_run_nekogame(L);

    neko::luainspector::luainspector_init(ENGINE_LUA());
    lua_setglobal(L, "__neko_inspector");

    if (!g_app->error_mode.load() && g_app->cfg.startup_load_scripts && mount.ok && mount_luacode.ok) {
        load_all_lua_scripts(L);
    }

    // run main.lua
    asset_load_kind(AssetKind_LuaRef, "script/nekomain.lua", nullptr);

    luax_get(ENGINE_LUA(), "neko", "__define_default_callbacks");
    luax_pcall(ENGINE_LUA(), 0, 0);

    {
        PROFILE_BLOCK("neko.args");

        lua_State* L = ENGINE_LUA();

        if (!g_app->error_mode.load()) {
            luax_neko_get(L, "args");

            Slice<String> args = g_app->args;
            lua_createtable(L, args.len - 1, 0);
            for (u64 i = 1; i < args.len; i++) {
                lua_pushlstring(L, args[i].data, args[i].len);
                lua_rawseti(L, -2, i);
            }

            luax_pcall(L, 1, 0);
        }
    }

    luax_get(ENGINE_LUA(), "neko", "game_init");
    luax_pcall(ENGINE_LUA(), 0, 0);

    // fire init event
    script_push_event("init");
    errcheck(luax_pcall_nothrow(L, 1, 0));

    input_add_key_down_callback(_key_down);
    input_add_key_up_callback(_key_up);
    input_add_char_down_callback(_char_down);
    input_add_mouse_down_callback(_mouse_down);
    input_add_mouse_up_callback(_mouse_up);
    input_add_mouse_move_callback(_mouse_move);
    input_add_scroll_callback(_scroll);

    if (g_app->cfg.target_fps != 0) {
        get_timing_instance()->target_ticks = 1000000000 / g_app->cfg.target_fps;
    }

#ifdef NEKO_IS_WIN32
    if (!g_app->win_console) {
        FreeConsole();
    }
#endif
}

void system_fini() {
    PROFILE_FUNC();

    edit_fini();
    script_fini();
    physics_fini();
    sound_fini();
    console_fini();
    tiled_fini();
    sprite_fini();
    batch_fini(g_app->batch);
    gui_fini();
    camera_fini();
    transform_fini();
    entity_fini();
    imgui_fini();
    auto& input = neko::the<Input>();
    input.fini();

    if (g_app->default_font != nullptr) {
        g_app->default_font->trash();
        mem_free(g_app->default_font);
    }

    gfx_fini(g_render);

    {
        PROFILE_BLOCK("destroy assets");

        lua_channels_shutdown();

        // if (g_app->default_font != nullptr) {
        //     g_app->default_font->trash();
        //     mem_free(g_app->default_font);
        // }

        assets_shutdown();
    }
}

// 以相同的顺序 保存/加载
static void _saveload_all(void* s, bool save) {
#define saveload(sys)              \
    if (save)                      \
        sys##_save_all((Store*)s); \
    else                           \
        sys##_load_all((Store*)s)

    entity_load_all_begin();

    saveload(entity);
    saveload(prefab);
    saveload(timing);
    saveload(transform);
    saveload(camera);
    saveload(sprite);
    saveload(physics);
    saveload(gui);
    saveload(edit);
    saveload(sound);

    saveload(script);

    entity_load_all_end();
}

void system_save_all(Store* s) { _saveload_all(s, true); }

void system_load_all(Store* s) { _saveload_all(s, false); }

static NativeEntity saved_root;

void prefab_save(const char* filename, NativeEntity root) {
    Store* s;

    saved_root = root;
    s = store_open();
    system_save_all(s);
    store_write_file(s, filename);
    store_close(s);
    saved_root = entity_nil;
}
NativeEntity prefab_load(const char* filename) {
    Store* s;
    NativeEntity root;

    s = store_open_file(filename);
    system_load_all(s);
    store_close(s);
    root = saved_root;
    saved_root = entity_nil;

    return root;
}

void prefab_save_all(Store* s) {
    Store* t;

    if (store_child_save(&t, "prefab", s)) entity_save(&saved_root, "saved_root", t);
}
void prefab_load_all(Store* s) {
    Store* t;

    if (store_child_load(&t, "prefab", s)) entity_load(&saved_root, "saved_root", entity_nil, t);
}
