-- normal mode
ns.edit.modes.normal['S-;'] = ns.edit.command_start
ns.edit.modes.normal['u'] = ns.edit.undo

ns.edit.modes.normal['s'] = ns.edit.command_save
ns.edit.modes.normal['l'] = ns.edit.command_load
ns.edit.modes.normal['\''] = ns.edit.command_save_prefab
ns.edit.modes.normal['.'] = ns.edit.command_load_prefab

ns.edit.modes.normal['p'] = ns.edit.pause_toggle
ns.edit.modes.normal['S-p'] = ns.edit.stop

ns.edit.modes.normal['a'] = ns.edit.select_clear
ns.edit.modes.normal['<mouse_1>'] = ns.edit.select_click_single
ns.edit.modes.normal['C-<mouse_1>'] = ns.edit.select_click_multi

ns.edit.modes.normal['x'] = ns.edit.destroy
ns.edit.modes.normal['S-x'] = ns.edit.destroy_rec
ns.edit.modes.normal['S-d'] = ns.edit.duplicate

ns.edit.modes.normal['S-<mouse_1>'] = ns.edit.camera_drag_start
ns.edit.modes.normal['^S-<mouse_1>'] = ns.edit.camera_drag_end
ns.edit.modes.normal['<mouse_3>'] = ns.edit.camera_drag_start
ns.edit.modes.normal['^<mouse_3>'] = ns.edit.camera_drag_end
ns.edit.modes.normal['-'] = ns.edit.camera_zoom_out
ns.edit.modes.normal['='] = ns.edit.camera_zoom_in

ns.edit.modes.normal['g'] = ns.edit.grab_start
ns.edit.modes.normal['r'] = ns.edit.rotate_start
ns.edit.modes.normal['b'] = ns.edit.boxsel_start

ns.edit.modes.normal[','] = ns.edit.command_inspect
ns.edit.modes.normal['S-g'] = ns.edit.command_grid

-- grab mode
ns.edit.modes.grab['<enter>'] = ns.edit.grab_end
ns.edit.modes.grab['<escape>'] = ns.edit.grab_cancel
ns.edit.modes.grab['<mouse_1>'] = ns.edit.grab_end
ns.edit.modes.grab['<mouse_2>'] = ns.edit.grab_cancel
ns.edit.modes.grab['g'] = ns.edit.grab_snap_on
ns.edit.modes.grab['<left>'] = ns.edit.grab_move_left
ns.edit.modes.grab['<right>'] = ns.edit.grab_move_right
ns.edit.modes.grab['<up>'] = ns.edit.grab_move_up
ns.edit.modes.grab['<down>'] = ns.edit.grab_move_down
ns.edit.modes.grab['S-<left>'] = function()
    ns.edit.grab_move_left(5)
end
ns.edit.modes.grab['S-<right>'] = function()
    ns.edit.grab_move_right(5)
end
ns.edit.modes.grab['S-<up>'] = function()
    ns.edit.grab_move_up(5)
end
ns.edit.modes.grab['S-<down>'] = function()
    ns.edit.grab_move_down(5)
end

-- rotate mode
ns.edit.modes.rotate['<enter>'] = ns.edit.rotate_end
ns.edit.modes.rotate['<escape>'] = ns.edit.rotate_cancel
ns.edit.modes.rotate['<mouse_1>'] = ns.edit.rotate_end
ns.edit.modes.rotate['<mouse_2>'] = ns.edit.rotate_cancel

-- boxsel mode
ns.edit.modes.boxsel['<mouse_1>'] = ns.edit.boxsel_begin
ns.edit.modes.boxsel['C-<mouse_1>'] = ns.edit.boxsel_begin
ns.edit.modes.boxsel['^<mouse_1>'] = ns.edit.boxsel_end
ns.edit.modes.boxsel['^C-<mouse_1>'] = ns.edit.boxsel_end_add

-- phypoly mode
ns.edit.modes.phypoly['<enter>'] = ns.edit.phypoly_end
ns.edit.modes.phypoly['<escape>'] = ns.edit.phypoly_cancel
ns.edit.modes.phypoly['<mouse_1>'] = ns.edit.phypoly_add_vertex
