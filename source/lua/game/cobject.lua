local M = {}

local cstruct = require "common/cstruct"

M.s = {}

M.s["CGameObject"] = cstruct.struct([[
    struct CGameObject {
        int id;
        bool active;
        bool visible;
        bool selected;
    };
    ]])

-- M.sz = M.s:size "struct CGameObject"

M.CGameObject_get_id = M.s["CGameObject"]:getter "struct CGameObject.id"
M.CGameObject_get_active = M.s["CGameObject"]:getter "struct CGameObject.active"
M.CGameObject_get_visible = M.s["CGameObject"]:getter "struct CGameObject.visible"
M.CGameObject_get_selected = M.s["CGameObject"]:getter "struct CGameObject.selected"

M.CGameObject_set_id = M.s["CGameObject"]:setter "struct CGameObject.id"
M.CGameObject_set_active = M.s["CGameObject"]:setter "struct CGameObject.active"
M.CGameObject_set_visible = M.s["CGameObject"]:setter "struct CGameObject.visible"
M.CGameObject_set_selected = M.s["CGameObject"]:setter "struct CGameObject.selected"

-- M.test_obj = __neko_cstruct_test.udata(M.sz)

M.new_obj = function(id, active, visible)
    local obj = __neko_cstruct_test.udata(M.s["CGameObject"]:size "struct CGameObject")
    M.CGameObject_set_id(obj, id)
    M.CGameObject_set_active(obj, active)
    M.CGameObject_set_visible(obj, visible)
    M.CGameObject_set_selected(obj, false)
    return obj
end

-- M.CGameObject_set_visible(M.test_obj, false)
-- print(M.CGameObject_get_visible(M.test_obj))
-- M.s:dump()

M.s["neko_sprite_t"] = cstruct.struct([[
    struct neko_sprite_t {
        uint64_t image_id;
        int depth;
        float x;
        float y;
        float sx;
        float sy;
        float c;
        float s;
    };
    ]])

M.s["neko_vec4_t"] = cstruct.struct([[
    struct neko_vec4_t {
        float x;
        float y;
        float z;
        float w;
    };
    ]])

M.s["__lua_quad_idata_t"] = cstruct.struct([[
    struct __lua_quad_idata_t {
        uint32_t v1;
        uint32_t v2;
        uint32_t v3;
        uint32_t v4;
        uint32_t v5;
        uint32_t v6;
    };
    ]])

M.s["__lua_quad_vdata_t"] = cstruct.struct([[
    struct __lua_quad_vdata_t {
        float v1;
        float v2;
        float v3;
        float v4;
        float v5;
        float v6;
        float v7;
        float v8;
        float v9;
        float v10;
        float v11;
        float v12;
        float v13;
        float v14;
        float v15;
        float v16;
    };
    ]])

M.s["__lua_tex_t"] = cstruct.struct([[
    struct __lua_tex_t {
        uint32_t id;
    };
    ]])

M.getter = function(struct_name, field)
    return M.s[struct_name]:getter("struct " .. struct_name .. "." .. field)
end

M.setter = function(struct_name, field)
    return M.s[struct_name]:setter("struct " .. struct_name .. "." .. field)
end

return M
