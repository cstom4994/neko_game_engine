local GenDir = "."

package.path = GenDir .. "/lib/?.lua"

local status = {
    GenDir = GenDir
}
local meta = require "imgui_meta"
meta.init(status, "cimgui.json")

local apis_file<close> = assert(io.open(GenDir .. "/gen/imgui_lua_funcs.cpp", "wb"))
local docs_file<close> = assert(io.open(GenDir .. "/gen/imgui.lua", "wb"))
status.apis_file = apis_file
status.docs_file = docs_file

assert(loadfile(GenDir .. "/lib/imgui_funcs.lua"))(status)
assert(loadfile(GenDir .. "/lib/imgui_doc.lua"))(status)