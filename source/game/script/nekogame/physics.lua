-- local ffi = require 'ffi'

-- ng.Collision = ffi.metatype('Collision', {})

-- local old_physics_shape_add_box = ng.physics_shape_add_box
-- function ng.physics_shape_add_box(ent, b, r)
--     r = r or 0
--     return old_physics_shape_add_box(ent, b, r)
-- end

-- local old_physics_shape_add_poly = ng.physics_shape_add_poly
-- function ng.physics_shape_add_poly(ent, verts, r)
--     r = r or 0
--     local n = #verts
--     return old_physics_shape_add_poly(ent, n, ffi.new('Vec2[?]', n, verts), r)
-- end

-- local old_physics_convex_hull = ng.physics_convex_hull
-- function ng.physics_convex_hull(verts)
--     local n = #verts
--     local c_arr = ffi.new('Vec2[?]', n, verts)
--     n = old_physics_convex_hull(n, c_arr)

--     local lua_arr = {}
--     for i = 0, n - 1 do
--         table.insert(lua_arr, ng.Vec2(c_arr[i]))
--     end
--     return lua_arr
-- end

-- local old_physics_get_collisions = ng.physics_get_collisions
-- function ng.physics_get_collisions(ent)
--     local n = ng.physics_get_num_collisions(ent)
--     local c_arr = old_physics_get_collisions(ent)
--     local lua_arr = {}
--     for i = 0, n - 1 do
--         table.insert(lua_arr, ng.Collision(c_arr[i]))
--     end
--     return lua_arr
-- end
