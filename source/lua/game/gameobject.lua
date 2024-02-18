local M = {}

local cstruct = require "common/cstruct"

M.test_code = [[
struct CGameObject {
	int id;
	bool active;
	bool visible;
	bool selected;
};
]]

M.s = cstruct.struct(M.test_code)

M.sz = M.s:size "struct CGameObject"

M.CGameObject_get_id = M.s:getter "struct CGameObject.id"
M.CGameObject_get_active = M.s:getter "struct CGameObject.active"
M.CGameObject_get_visible = M.s:getter "struct CGameObject.visible"
M.CGameObject_get_selected = M.s:getter "struct CGameObject.selected"

M.CGameObject_set_id = M.s:setter "struct CGameObject.id"
M.CGameObject_set_active = M.s:setter "struct CGameObject.active"
M.CGameObject_set_visible = M.s:setter "struct CGameObject.visible"
M.CGameObject_set_selected = M.s:setter "struct CGameObject.selected"

M.test_obj = __neko_cstruct_test.udata(M.sz)

M.new_obj = function(id, active, visible)
    local obj = __neko_cstruct_test.udata(M.sz)
    M.CGameObject_set_id(obj, id)
    M.CGameObject_set_active(obj, active)
    M.CGameObject_set_visible(obj, visible)
    M.CGameObject_set_selected(obj, false)
    return obj
end

M.CGameObject_set_visible(M.test_obj, false)

print(M.CGameObject_get_visible(M.test_obj))

M.s:dump()

return M
