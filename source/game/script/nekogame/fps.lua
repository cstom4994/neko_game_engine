ns.fps = {}

-- number of frames and elapsed time per round
local nframes = 0;
local timer = 0;

-- display gui entity
local display = ng.add {
    group = {
        groups = 'builtin'
    },
    edit = {
        editable = false
    },
    gui_text = {
        str = 'fps: ...'
    },
    gui = {
        color = ng.color(0, 0.4, 0.1, 1),
        halign = ng.GA_MAX,
        valign = ng.GA_MIN,
        padding = ng.vec2(1, 1)
    }
}

function ns.fps.update_all()
    -- timing
    nframes = nframes + 1
    timer = timer + ng.get_timing_instance().true_dt
    if timer > 2.5 then -- recalculate every 2.5 seconds
        ns.gui_text.set_str(display, string.format('fps: %.2f', nframes / timer))
        timer = 0
        nframes = 0
    end
end
