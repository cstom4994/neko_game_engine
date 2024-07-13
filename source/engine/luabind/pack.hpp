
#include "engine/neko_luabind.hpp"

struct neko_lua_handle_t {
    const_str name;
    size_t size;
    void* data;
};

LUA_FUNCTION(__neko_bind_pack_construct) {
    const_str name = lua_tostring(L, 1);
    const_str path = lua_tostring(L, 2);

    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_newuserdata(L, sizeof(neko_lua_handle_t));

    userdata_ptr->name = name;
    userdata_ptr->size = 0;

    userdata_ptr->data = neko_safe_malloc(sizeof(neko_pak));

    bool ok = (static_cast<neko_pak*>(userdata_ptr->data))->load(path, 0, false);

    if (!ok) {
        const_str error_message = "neko_pak_check failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // 获取元表并设置
    // luaL_newmetatable(L, "neko_lua_handle__pack");  // 供测试的元表
    // lua_setmetatable(L, -2);

    NEKO_INFO("__neko_bind_pack_construct %s", userdata_ptr->name);
    return 1;
}

LUA_FUNCTION(__neko_bind_pack_destroy) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);

    neko_pak* pack = (neko_pak*)(userdata_ptr->data);
    pack->fini();

    neko_safe_free(userdata_ptr->data);

    NEKO_INFO("__neko_bind_pack_destroy %s", userdata_ptr->name);
    return 0;
}

LUA_FUNCTION(__neko_bind_pack_build) {

    const_str path = lua_tostring(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);  // 检查是否为table
    lua_len(L, 2);                     // 获取table的长度
    int n = lua_tointeger(L, -1);      //
    lua_pop(L, 1);                     // 弹出长度值

    const_str* item_paths = (const_str*)neko_safe_malloc(n * sizeof(const_str));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, 2, i);                 // 将index=i的元素压入堆栈顶部
        const_str str = lua_tostring(L, -1);  // # -1
        if (str != NULL) {
            item_paths[i - 1] = str;
        }
        lua_pop(L, 1);  // # -1
    }

    bool ok = neko_pak_build(path, n, item_paths, true);

    neko_safe_free(item_paths);

    if (!ok) {
        const_str error_message = "__neko_bind_pack_build failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    return 0;
}

LUA_FUNCTION(__neko_bind_pack_info) {

    const_str path = lua_tostring(L, 1);

    u8 pack_version;
    bool is_little_endian;
    u64 item_count;

    bool ok = neko_pak_info(path, &pack_version, &is_little_endian, &item_count);

    lua_pushinteger(L, pack_version);
    lua_pushboolean(L, is_little_endian);
    lua_pushinteger(L, item_count);

    return 3;
}

LUA_FUNCTION(__neko_bind_pack_items) {

    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);

    neko_pak* pack = (neko_pak*)(userdata_ptr->data);

    u64 item_count = pack->get_item_count();

    lua_newtable(L);  // # -2
    for (int i = 0; i < item_count; ++i) {
        lua_pushstring(L, pack->get_item_path(i));  // # -1
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

LUA_FUNCTION(__neko_bind_pack_assets_load) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);
    const_str path = lua_tostring(L, 2);
    neko_pak* pack = (neko_pak*)(userdata_ptr->data);

    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_newuserdata(L, sizeof(neko_lua_handle_t));
    assets_user_handle->name = path;
    assets_user_handle->size = 0;

    bool ok = pack->get_data(path, (const u8**)&assets_user_handle->data, (u32*)&assets_user_handle->size);

    if (!ok) {
        const_str error_message = "__neko_bind_pack_assets_load failed";
        lua_pushstring(L, error_message);  // 将错误信息压入堆栈
        return lua_error(L);               // 抛出lua错误
    }

    // NEKO_INFO("__neko_bind_pack_assets_load %llu", assets_user_handle->size);

    // 获取元表并设置
    // luaL_newmetatable(L, "neko_lua_handle__assets");  // 供测试的元表
    // lua_setmetatable(L, -2);

    return 1;
}

LUA_FUNCTION(__neko_bind_pack_assets_unload) {
    neko_lua_handle_t* userdata_ptr = (neko_lua_handle_t*)lua_touserdata(L, 1);
    neko_pak* pack = (neko_pak*)(userdata_ptr->data);
    neko_lua_handle_t* assets_user_handle = (neko_lua_handle_t*)lua_touserdata(L, 2);
    if (assets_user_handle && assets_user_handle->data)
        pack->free_item(assets_user_handle->data);
    else
        NEKO_WARN("unknown assets unload %p", assets_user_handle);
    return 0;
}

namespace neko::lua::__pack {

LUABIND_MODULE() {
    luaL_Reg libs[] = {{"construct", __neko_bind_pack_construct},
                       {"destroy", __neko_bind_pack_destroy},
                       {"build", __neko_bind_pack_build},
                       {"info", __neko_bind_pack_info},
                       {"items", __neko_bind_pack_items},
                       {"assets_load", __neko_bind_pack_assets_load},
                       {"assets_unload", __neko_bind_pack_assets_unload},
                       {NULL, NULL}};
    luaL_newlib(L, libs);
    return 1;
}

}  // namespace neko::lua::__pack

DEFINE_LUAOPEN(pack)