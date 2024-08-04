local bump = require 'libs/bump'

local world = bump.newWorld()

ns.bump = ng.simple_sys()

ng.simple_prop(ns.bump, 'bbox', ng.bbox(ng.vec2(-0.5, -0.5),
                                        ng.vec2(0.5, 0.5)))

local function _update_rect(obj, add)
    if world:hasItem(obj.ent.id)
    and obj.last_dirty == ns.transform.get_dirty_count(obj.ent) then
        return
    end

    local lt = obj.bbox.min + ns.transform.get_position(obj.ent)
    local wh = obj.bbox.max - obj.bbox.min
    if world:hasItem(obj.ent.id) then
        world:move(obj.ent.id, lt.x, lt.y, wh.x, wh.y)
    else
        world:add(obj.ent.id, lt.x, lt.y, wh.x, wh.y)
    end

    obj.last_dirty = ns.transform.get_dirty_count(obj.ent)
end

function ns.bump.create(obj)
    ns.transform.add(obj.ent)
    _update_rect(obj, true)
end
function ns.bump.destroy(obj)
    if world:hasItem(obj.ent.id) then
        world:remove(obj.ent.id)
    end
end

function ns.bump.set_position(ent, pos)
    local obj = ns.bump.tbl[ent]
    assert(obj, 'entity must be in bump system')
    ns.transform.set_position(ent, pos)
    _update_rect(obj)
end

local function _filter_wrap(filter)
    return filter and function (id)
        return filter(ng.Entity { id = id })
    end
end

function ns.bump.sweep(ent, p, filter)
    local obj = ns.bump.tbl[ent]
    assert(obj, 'entity must be in bump system')
    _update_rect(obj)

    p = obj.bbox.min + ns.transform.get_position(ent) + (p or ng.vec2_zero)

    local wfilter = _filter_wrap(filter)
    local cols, len = world:check(obj.ent.id, p.x, p.y, wfilter)
    local min = obj.bbox.min
    local ecols = {}
    for i = 1, len do
        local col = cols[i]
        local tl, tt, nx, ny, sl, st = col:getSlide()
        table.insert(ecols, {
            other = ng.Entity { id = col.other },
            touch = ng.vec2(tl, tt) - min,
            normal = ng.vec2(nx, ny),
            slide = ng.vec2(sl, st) - min,
        })
    end
    return ecols
end

function ns.bump.slide(ent, p, filter)
    local obj = ns.bump.tbl[ent]
    assert(obj, 'entity must be in bump system')
    _update_rect(obj)

    p = ns.transform.get_position(ent) + p

    local min = obj.bbox.min
    local wfilter = _filter_wrap(filter)

    local function rslide(ax, ay, bx, by, depth)
        if depth > 3 then return ax, ay, {} end

        world:move(obj.ent.id, ax, ay)
        local cols, len = world:check(obj.ent.id, bx, by, wfilter)
        if len == 0 then return bx, by, {} end

        -- find best next collision recursively
        local m = -1
        local mcols, mcol, mtx, mty, mnx, mny, msx, msy
        for i = 1, len do
            world:move(obj.ent.id, ax, ay) -- TODO: avoid re-moving
            local tx, ty, nx, ny, sx, sy = cols[i]:getSlide()
            local px, py, pcols = rslide(tx, ty, sx, sy, depth + 1)
            local dx, dy = px - ax, py - ay
            local d = dx * dx + dy * dy
            if d > m then
                m = d
                bx, by = px, py
                mcols, mcol = pcols, cols[i]
                mtx, mty, mnx, mny, msx, msy = tx, ty, nx, ny, sx, sy
            end
        end

        -- add next collision and return
        table.insert(mcols, {
            other = ng.Entity { id = mcol.other },
            touch = ng.vec2(mtx, mty) - min,
            normal = ng.vec2(mnx, mny),
            slide = ng.vec2(msx, msy) - min,
        })
        return bx, by, mcols
    end

    local a = ns.transform.get_position(obj.ent) + min
    local bx, by, cols = rslide(a.x, a.y, p.x + min.x, p.y + min.y, 0)
    ns.transform.set_position(ent, ng.vec2(bx - min.x, by - min.y))
    _update_rect(obj)
    return cols
end

function ns.bump.update(obj)
    _update_rect(obj)
    ns.edit.bboxes_update(obj.ent, obj.bbox)
end
