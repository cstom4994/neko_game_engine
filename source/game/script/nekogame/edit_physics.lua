--- custom inspector -----------------------------------------------------------
local function add_circle(ent, bbox)
    local r = 0.5 * math.min(bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y)
    local mid = 0.5 * (bbox.max + bbox.min)

    local function add_r(rs)
        local function add_midx(midxs)
            local function add_midy(midys)
                ns.physics.shape_add_circle(ent, tonumber(rs) or 0, ng.vec2(tonumber(midxs) or 0, tonumber(midys) or 0))
            end
            ns.edit.command_start('offset y: ', add_midy, nil, false, tostring(mid.y))
        end
        ns.edit.command_start('offset x: ', add_midx, nil, false, tostring(mid.x))
    end

    ns.edit.command_start('radius: ', add_r, nil, false, tostring(r))
end

local function add_box(ent, bbox)
    local mindim = math.min(bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y)

    local function add(s)
        -- reduce bbox size to account for radius, then add
        local r = math.min(tonumber(s) or 0, 0.5 * mindim)
        local bbox2 = ng.bbox(ng.vec2(bbox.min.x + r, bbox.min.y + r), ng.vec2(bbox.max.x - r, bbox.max.y - r))
        ns.physics.shape_add_box(ent, bbox2, r)
    end

    ns.edit.command_start('rounding radius: ', add)
end

local function add_poly(ent)
    local function add(s)
        ns.edit.phypoly_start(ent, tonumber(s) or 0)
    end
    ns.edit.command_start('rounding radius: ', add)
end

ns.edit_inspector.custom['physics'] = {
    add = function(inspector)
        -- add buttons
        inspector.add_container = ng.add {
            transform = {
                parent = inspector.window_body
            },
            gui = {
                padding = ng.vec2_zero,
                color = ng.color_clear,
                valign = ng.GA_TABLE,
                halign = ng.GA_MIN
            },
            gui_rect = {
                hfill = true
            }
        }

        inspector.add_box = ng.add {
            transform = {
                parent = inspector.add_container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_box), 'add box')

        inspector.add_poly = ng.add {
            transform = {
                parent = inspector.add_container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_poly), 'add poly')

        inspector.add_circle = ng.add {
            transform = {
                parent = inspector.add_container
            },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_MAX,
                halign = ng.GA_TABLE
            },
            gui_textbox = {}
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_circle), 'add circle')

        inspector.shapes = {}
    end,

    post_update = function(inspector)
        ns.physics.debug_draw(inspector.ent)

        -- 'add box' button
        if ns.gui.event_mouse_down(inspector.add_box) == ng.MC_LEFT then
            local bbox
            if ns.edit.bboxes_has(inspector.ent) then
                bbox = ns.edit.bboxes_get(inspector.ent)
            else
                bbox = ng.bbox(ng.vec2(-1, -1), ng.vec2(1, 1))
            end
            add_box(inspector.ent, bbox)
            ns.edit.undo_save()
        end

        -- 'add poly' button
        if ns.gui.event_mouse_down(inspector.add_poly) == ng.MC_LEFT then
            add_poly(inspector.ent)
        end

        -- 'add circle' button
        if ns.gui.event_mouse_down(inspector.add_circle) == ng.MC_LEFT then
            local bbox
            if ns.edit.bboxes_has(inspector.ent) then
                bbox = ns.edit.bboxes_get(inspector.ent)
            else
                bbox = ng.bbox(ng.vec2(-1, -1), ng.vec2(1, 1))
            end
            add_circle(inspector.ent, bbox)
            ns.edit.undo_save()
        end

        local nshapes = ns.physics.get_num_shapes(inspector.ent)

        while nshapes > #inspector.shapes do
            local shape = {}
            table.insert(inspector.shapes, shape)
            shape.window = ng.add {
                transform = {
                    parent = inspector.window_body
                },
                gui_window = {},
                gui_rect = {
                    hfill = true
                },
                gui = {
                    valign = ng.GA_TABLE,
                    halign = ng.GA_MIN
                }
            }
            shape.window_body = ns.gui_window.get_body(shape.window)

            shape.poly_container = ng.add {
                transform = {
                    parent = shape.window_body
                },
                gui = {
                    padding = ng.vec2_zero,
                    color = ng.color_clear,
                    valign = ng.GA_TABLE,
                    halign = ng.GA_MIN
                },
                gui_rect = {
                    hfill = true
                }
            }

            shape.poly_info = ng.add {
                transform = {
                    parent = shape.poly_container
                },
                gui = {
                    color = ng.color_white,
                    valign = ng.GA_MID,
                    halign = ng.GA_TABLE
                },
                gui_text = {}
            }
        end

        while nshapes < #inspector.shapes do
            local shape = table.remove(inspector.shapes)
            ns.gui_window.remove(shape.window)
        end

        for i = #inspector.shapes, 1, -1 do -- backwards for safe remove
            local shape = inspector.shapes[i]
            if ns.entity.destroyed(shape.window) then
                ns.physics.shape_remove(inspector.ent, i - 1)
                table.remove(inspector.shapes, i)
            else
                local t = ns.physics.shape_get_type(inspector.ent, i - 1)
                ns.gui_window.set_title(shape.window, ng.enum_tostring('PhysicsShape', t))
                if t == ng.PS_POLYGON then
                    local n = ns.physics.poly_get_num_verts(inspector.ent, i - 1)
                    ns.gui_text.set_str(shape.poly_info, n .. ' vertices')
                end
            end
        end
    end
}

--- phypoly mode (draw polygon shape) ------------------------------------------

local phypoly_ent, phypoly_verts, phypoly_radius

ns.edit.modes.phypoly = {}

local function phypoly_update_verts()
    if #phypoly_verts >= 4 then
        phypoly_verts = ns.physics.convex_hull(phypoly_verts)
    end
end

function ns.edit.phypoly_start(ent, radius)
    ns.edit.set_mode('phypoly')
    phypoly_ent = ent
    phypoly_verts = {}
    phypoly_radius = radius or 0
end
function ns.edit.phypoly_end()
    if #phypoly_verts < 3 then
        return
    end

    ns.edit.set_mode('normal')
    ns.physics.shape_add_poly(phypoly_ent, phypoly_verts, phypoly_radius)
    phypoly_ent = nil
    phypoly_verts = nil
    ns.edit.undo_save()
end
function ns.edit.phypoly_cancel()
    ns.edit.set_mode('normal')
    phypoly_ent = nil
    phypoly_verts = nil
end

function ns.edit.phypoly_add_vertex()
    local m = ns.camera.unit_to_world(ns.input.get_mouse_pos_unit())
    -- TODO: remove scaling issue
    local t = ng.mat3_inverse(ns.transform.get_world_matrix(phypoly_ent))
    table.insert(phypoly_verts, ng.Vec2(ng.mat3_transform(t, m)))
    phypoly_update_verts()
end

function ns.edit.modes.phypoly.enter()
    ns.edit.set_mode_text('phypoly')
end

function ns.edit.modes.phypoly.update_all()
    if not ns.physics.has(phypoly_ent) then
        ns.edit.phypoly_cancel()
    end

    if #phypoly_verts > 1 then
        for i = 1, #phypoly_verts do
            local j = i < #phypoly_verts and i + 1 or 1
            local a = ns.transform.local_to_world(phypoly_ent, phypoly_verts[i])
            local b = ns.transform.local_to_world(phypoly_ent, phypoly_verts[j])
            ns.edit.line_add(a, b, 5, ng.color(1.0, 0.0, 0.0, 1.0))
        end
    end
end
