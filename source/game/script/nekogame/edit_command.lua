--- command mode ---------------------------------------------------------------
ns.edit.modes.command = {}

local command_end_callback, command_completion_func, command_completions
local command_completions_index, command_always_complete

local function command_update_completions_text()
    ns.gui_text.set_str(ns.edit.command_completions_text, table.concat(command_completions, ' | '))
end
local function command_update_completions()
    local s = ns.gui_text.get_str(ns.edit.command_text)
    command_completions = command_completion_func(s)
    command_update_completions_text()
end

-- whether sub is a subsequence of seq
local function subseq(seq, sub)
    local j = 1
    local lsub = #sub + 1
    if lsub == 1 then
        return true
    end
    for i = 1, #seq do
        if string.byte(seq, i) == string.byte(sub, j) then
            j = j + 1
            if j == lsub then
                return true
            end
        end
    end
    return false
end

-- returns a completion function that uses substring search
-- case insensitive
function ns.edit.command_completion_substr(t)
    return function(s)
        local comps = {}
        local s = string.lower(s)
        for k in pairs(t) do
            if subseq(string.lower(k), s) then
                table.insert(comps, k)
            end
        end
        return comps
    end
end

-- completion function that uses file system search, case
-- insensitive
function ns.edit.command_completion_fs(s)
    local comps = {}
    local s = string.lower(s)

    local dir_path = string.match(s, '(.*/)') or './'
    local suffix = string.match(s, '.*/(.*)') or s
    local dir = ns.fs.dir_open(dir_path)
    if dir == nil then
        return {}
    end
    while true do
        local f = ns.fs.dir_next_file(dir)
        if f == nil then
            break
        end
        f = ng.string(f)
        if f ~= '.' and f ~= '..' and subseq(string.lower(dir_path .. f), s) then
            table.insert(comps, dir_path .. f)
        end
    end
    ns.fs.dir_close(dir)

    return comps
end

local function run_string(s)
    local r, e = loadstring(s)
    if r then
        r()
    else
        error(e)
    end
end

function ns.edit.command_start(prompt, callback, completion_func, always_complete, initial)
    ns.edit.set_mode('command')

    -- default is eval script
    prompt = prompt or 'lua: '
    command_end_callback = callback or run_string
    command_completion_func = completion_func or function()
        return {}
    end
    command_always_complete = always_complete and true or false

    initial = initial or ''
    ns.gui_text.set_str(ns.edit.command_text, initial or '')
    ns.gui_textedit.set_cursor(ns.edit.command_text, #initial)

    ns.gui_text.set_str(ns.edit.command_text_colon, prompt)
    command_update_completions()
end
function ns.edit.command_end()
    if command_always_complete then
        if #command_completions == 0 then
            return
        end -- no completions
        ns.edit.command_complete()
    end

    ns.edit.set_mode('normal')

    local s = ns.gui_text.get_str(ns.edit.command_text)
    if command_end_callback then
        command_end_callback(s)
    else
        print('no command callback for \'' .. s .. '\'')
    end
end
function ns.edit.command_cancel()
    ns.edit.set_mode('normal')
end

-- actually pick a completion
function ns.edit.command_complete()
    if #command_completions > 0 then
        local comp = command_completions[1]
        ns.gui_text.set_str(ns.edit.command_text, comp)
        ns.gui_textedit.set_cursor(ns.edit.command_text, #comp)
        command_update_completions()
    end
end

function ns.edit.modes.command.enter()
    ns.edit.set_mode_text('command')

    ns.gui.set_visible(ns.edit.command_bar, true)

    ns.gui_text.set_str(ns.edit.command_completions_text, '')
end
function ns.edit.modes.command.exit()
    ns.gui.set_visible(ns.edit.command_bar, false)

    ns.gui.set_focus(ns.edit.command_text, false)
    command_completions = {}
end

function ns.edit.modes.command.update_all()
    -- done?
    if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_ENTER then
        ns.edit.command_end()
        return
    elseif ns.gui.event_focus_exit(ns.edit.command_text) then
        ns.edit.command_cancel()
        return
    end

    if ns.gui.event_changed(ns.edit.command_text) then
        command_update_completions()
    end
    if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_TAB then
        ns.edit.command_complete()
    end

    -- next/prev completion
    if ns.gui.event_key_down(ns.edit.command_text) == ng.KC_DOWN then
        table.insert(command_completions, table.remove(command_completions, 1))
        command_update_completions_text()
    elseif ns.gui.event_key_down(ns.edit.command_text) == ng.KC_UP then
        table.insert(command_completions, 1, table.remove(command_completions))
        command_update_completions_text()
    end

    ns.gui.set_focus(ns.edit.command_text, true)
end

--- built-in prompts -----------------------------------------------------------

-- asks for grid size -- first x then y
local function command_gridx(x)
    local function gridy(y)
        ns.edit.set_grid_size(ng.vec2(tonumber(x) or 0, tonumber(y) or 0))
    end
    ns.edit.command_start('grid y: ', gridy)
end
function ns.edit.command_grid()
    ns.edit.command_start('grid x: ', command_gridx)
end

-- inspects system on selected entities, or creates entity if none selected
function ns.edit.command_inspect()
    local add = ng.entity_table_empty(ns.edit.select)

    local function system(s)
        if add then
            local e = ng.entity_create()
            ns.edit_inspector.add(e, s)
            ns.edit.select[e] = true
        elseif not ng.entity_table_empty(ns.edit.select) then
            for ent in pairs(ns.edit.select) do
                ns.edit_inspector.add(ent, s)
            end
        end
        ns.edit.undo_save()
    end

    -- complete to systems that have properties listed
    local syss = ns.edit_inspector.get_systems()
    local comp = ns.edit.command_completion_substr(syss)

    ns.edit.command_start(add and 'new entity: ' or 'edit system: ', system, comp, true)
end

local last_save = nekogame_usr_path .. 'levels/'
function ns.edit.command_save()
    local function save(f)
        ns.console.printf("edit: saving group 'default' to file '" .. f .. "' ... ")
        ns.group.set_save_filter('default', true)
        local s = ng.store_open()
        ns.system.save_all(s)
        ng.store_write_file(s, f)
        ng.store_close(s)
        print("done")

        ns.edit.stop_save()

        last_save = f
    end

    ns.edit.command_start('save to file: ', save, ns.edit.command_completion_fs, false, last_save)
end

local last_load = nekogame_usr_path .. 'levels/'
function ns.edit.command_load()
    local function load(f)
        ns.group.destroy('default')

        ns.console.printf("edit: loading from file '" .. f .. "' ... ")
        local s = ng.store_open_file(f)
        ns.system.load_all(s)
        ng.store_close(s)
        print("done")

        ns.edit.stop_save()
        ns.timing.set_paused(true)
        ns.edit.stopped = true

        last_load = f
    end

    ns.edit.command_start('load from file: ', load, ns.edit.command_completion_fs, true, last_load)
end

function ns.edit.set_default_file(s)
    last_save = s
    last_load = s
end

local last_save_prefab = nekogame_usr_path .. 'prefabs/'
function ns.edit.command_save_prefab()
    if ng.entity_table_empty(ns.edit.select) then
        return
    end

    local function save(f)
        for ent in pairs(ns.edit.select) do
            if ns.transform.has(ent) then
                ns.transform.set_save_filter_rec(ent, true)
            else
                ns.entity.set_save_filter(ent, true)
            end
        end

        ns.prefab.save(f, ns.edit.select_get_first())

        last_save_prefab = f
    end

    ns.edit.command_start('save prefab: ', save, ns.edit.command_completion_fs, false, last_save_prefab)
end

local last_load_prefab = nekogame_usr_path .. 'prefabs/'
function ns.edit.command_load_prefab()
    local function load(f)
        ns.edit.select_clear()
        local ent = ns.prefab.load(f)
        ns.edit.select[ent] = true
        if ns.transform.has(ent) then
            -- move to center of view
            local w = ns.transform.local_to_world(ns.edit.camera, ng.vec2_zero)
            w.x = math.floor(w.x + 0.5)
            w.y = math.floor(w.y + 0.5)
            ns.transform.set_position(ent, w)
        end
        ns.edit.undo_save()

        last_load_prefab = f
    end

    ns.edit.command_start('load prefab: ', load, ns.edit.command_completion_fs, true, last_load_prefab)
end

function ns.edit.set_default_prefab_file(s)
    last_save_prefab = s
    last_load_prefab = s
end
