ns.animation = { auto_saveload = true }

ns.animation.tbl = ng.entity_table()

function ns.animation.add(ent)
    if ns.animation.tbl[ent] then return end

    ns.animation.tbl[ent] = {
        ent = ent,
        anims = {},
        curr_anim = nil,   -- name of animation
        frame = 1,         -- index of frame
        t = 1,             -- time left in this frame
    }
end
function ns.animation.remove(ent, anim)
    local entry = ns.animation.tbl[ent]
    if entry then
        if anim then
            if entry.curr_anim == anim then
                entry.curr_anim = nil
            end
            entry.anims[anim] = nil
        else
            ns.animation.tbl[ent] = nil
        end
    end
end
function ns.animation.has(ent)
    return ns.animation.tbl[ent] ~= nil
end

-- utility for contiguous strips of frames
function ns.animation.set_strips(ent, tbl)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    for anim, strip in pairs(tbl) do
        entry.anims[anim] = { n = strip.n, strip = strip }
    end
end

-- manual specification of every frame and its duration
function ns.animation.set_frames(ent, tbl)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    for anim, frames in pairs(anim) do
        entry.anims[anim] = { n = #frames, frames = frames }
    end
end

local function _enter_frame(entry, frame, anim)
    anim = anim or entry.anims[entry.curr_anim]
    entry.frame = frame
    if anim.strip then
        entry.t = anim.strip.t > 0 and anim.strip.t or 1
        local v = ng.Vec2(anim.strip.base)
        v.x = v.x + (frame - 1) * ns.sprite.get_texsize(entry.ent).x
        ns.sprite.set_texcell(entry.ent, v)
    elseif anim.frames then
        local frm = anim.frames[frame]
        entry.t = frm.t > 0 and frm.t or 1
        if frm.texcell then ns.sprite.set_texcell(entry.ent, frm.texcell) end
        if frm.texsize then ns.sprite.set_texsize(entry.ent, frm.texsize) end
    end
end

function ns.animation.get_curr_anim(ent, anim)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    return entry.curr_anim
end

function ns.animation.switch(ent, anim)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    assert(entry.anims[anim], "must have an animation with name '"
               .. anim .. "'")
    if entry.curr_anim ~= anim then
        entry.curr_anim = anim
        _enter_frame(entry, 1)
    end
end

function ns.animation.start(ent, anim)
    local entry = ns.animation.tbl[ent]
    assert(entry, 'entity must be in animation system')
    assert(entry.anims[anim], "must have an animation with name '"
               .. anim .. "'")
    entry.curr_anim = anim
    _enter_frame(entry, 1)
end

function ns.animation.update_all()
    ng.entity_table_remove_destroyed(ns.animation.tbl, ns.animation.remove)

    if ns.timing.get_paused() then return end

    for ent, entry in pairs(ns.animation.tbl) do
        if entry.curr_anim then
            local dt = ns.timing.dt
            local anim = entry.anims[entry.curr_anim]

            -- next frame?
            while entry.t <= dt do
                if anim.after and entry.frame >= anim.n then
                    assert(entry.anims[anim.after], "must have an animation "
                               .. "with name '" .. anim.after .. "'")
                    entry.curr_anim = anim.after
                    entry.frame = 1
                    anim = entry.anims[anim.after]
                end
                _enter_frame(entry, entry.frame >= anim.n
                                 and 1 or entry.frame + 1, anim)
                dt = dt - entry.t
            end

            entry.t = entry.t - dt
        end
    end
end

ns.edit_inspector.custom['animation'] = {
    add = function (inspector)
        -- current animation
        inspector.curr_anim = ng.edit_field_create {
            field_type = 'enum', parent = inspector.window_body,
            label = 'curr_anim'
        }

        -- animation list
        inspector.anim_views_container = ng.add {
            transform = { parent = inspector.window_body },
            gui = {
                padding = ng.vec2_zero,
                color = ng.color_clear,
                valign = ng.GA_TABLE,
                halign = ng.GA_MIN,
            },
            gui_rect = { hfill = true },
        }
        inspector.anim_views = {}

        -- 'add strip' button
        inspector.add_strip = ng.add {
            transform = { parent = inspector.window_body },
            gui = {
                color = ng.color(0.35, 0.15, 0.30, 1),
                valign = ng.GA_TABLE,
                halign = ng.GA_MID,
            },
            gui_textbox = {},
        }
        ns.gui_text.set_str(ns.gui_textbox.get_text(inspector.add_strip),
                            'add strip')
    end,

    post_update = function (inspector)
        local ent = inspector.ent
        local entry = ns.animation.tbl[ent]
        local anims = entry.anims

        -- current animation
        ng.edit_field_post_update(
            inspector.curr_anim, entry.curr_anim or '(none)',
            function (v) ns.animation.switch(ent, v) end,
            anims)

        -- add strip?
        if ns.gui.event_mouse_down(inspector.add_strip) == ng.MC_LEFT then
            local function new_strip(s)
                local strips = { [s] = { n = 1, t = 1, base = ng.vec2(0, 0) } }
                ns.animation.set_strips(ent, strips)
            end
            ns.edit.command_start('new strip name: ', new_strip)
        end

        -- add missing views
        for name, anim in pairs(anims) do
            if not inspector.anim_views[name] then
                local view = {}
                inspector.anim_views[name] = view

                -- view window
                view.window = ng.add {
                    transform = { parent = inspector.anim_views_container },
                    gui_window = { title = name, minimized = true },
                    gui_rect = { hfill = true },
                    gui = {
                        valign = ng.GA_TABLE,
                        halign = ng.GA_MIN,
                    }
                }
                view.window_body = ns.gui_window.get_body(view.window)

                -- 'duplicate' button
                view.dup_text = ng.add {
                    transform = {
                        parent =
                            ns.gui_window.get_title_buttons_area(view.window)
                    },
                    gui = {
                        color = ng.color_white,
                        valign = ng.GA_MAX,
                        halign = ng.GA_TABLE,
                    },
                    gui_text = { str = 'd' },
                }

                -- fields
                view.n = ng.edit_field_create {
                    field_type = 'Scalar', parent = view.window_body,
                    label = 'n'
                }
                view.t = ng.edit_field_create {
                    field_type = 'Scalar', parent = view.window_body,
                    label = 't'
                }
                view.base = ng.edit_field_create {
                    field_type = 'Vec2', parent = view.window_body,
                    label = 'base'
                }
                view.after = ng.edit_field_create {
                    field_type = 'enum', parent = view.window_body,
                    label = 'after'
                }
            end
        end

        -- remove extra views
        for name, view in pairs(inspector.anim_views) do
            if not anims[name] then
                ns.gui_window.remove(view.window)
                inspector.anim_views[name] = nil
            end
        end

        -- update views
        for name, view in pairs(inspector.anim_views) do
            if ns.entity.destroyed(view.window) then
                ns.animation.remove(ent, name)
            else
                local anim = anims[name]

                -- duplicate?
                if ns.gui.event_mouse_down(view.dup_text) == ng.MC_LEFT then
                    local function new_strip(s)
                        local strips = {
                            [s] = {
                                n = anim.n, t = anim.strip.t,
                                base = ng.Vec2(anim.strip.base),
                                after = anim.after
                            }
                        }
                        ns.animation.set_strips(ent, strips)
                    end
                    ns.edit.command_start('duplicate strip name: ', new_strip)
                end

                -- update fields
                ng.edit_field_post_update(
                    view.n, anim.n,
                    function (v) anim.n = v end)
                ng.edit_field_post_update(
                    view.t, anim.strip.t,
                    function (v)
                        anim.strip.t = v
                        entry.t = v > 0 and v or 1
                    end)
                ng.edit_field_post_update(
                    view.base, anim.strip.base,
                    function (v) anim.strip.base = v end)
                ng.edit_field_post_update(
                    view.after, anim.after or '(none)',
                    function (s) anim.after = s end, anims)
            end
        end
    end,
}
