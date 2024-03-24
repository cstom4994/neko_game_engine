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

M.getter = function(struct_name, field)
    return M.s[struct_name]:getter("struct " .. struct_name .. "." .. field)
end

M.setter = function(struct_name, field)
    return M.s[struct_name]:setter("struct " .. struct_name .. "." .. field)
end

return M
