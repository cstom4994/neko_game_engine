
cd source/api

if [ ! -d gen ]; then
    mkdir gen
    mkdir gen/dbg
    mkdir gen/typings
fi

echo "Debug output: tag raw tree. this is before being it's processed"
lua -e "print((require 'lib.tag.tag_pp')(require 'defs_neko'.raw_tree) .. '\n')" > gen/dbg/_neko_tagtree.txt
lua -e "print((require 'lib.tag.tag_pp')(require 'defs_neko_core'.raw_tree) .. '\n')" > gen/dbg/_neko_core_tagtree.txt

echo "Debug output:  kikito's inspect output (lua table format) (post-processed tree)"
lua -e "print('return ' .. (require 'lib.tag.inspect')(require'defs_neko'.tree, {indent='    '}) .. '\n')" > gen/dbg/_neko_tree.inspect.lua
lua -e "print('return ' .. (require 'lib.tag.inspect')(require'defs_neko_core'.tree, {indent='    '}) .. '\n')" > gen/dbg/_neko_core_tree.inspect.lua

echo "Typings: in Lua for VSCode"
lua -e "print(require('lib.defs_to_docstrings')(require'defs_neko'.tree))" > gen/typings/neko.lua
lua -e "print(require('lib.defs_to_docstrings')(require'defs_neko_core'.tree))" >> gen/typings/neko.lua

echo "API: Lua (boot) (source: defs_neko)"
lua -e "print(require('lib.gen_api_luaboot')(require'defs_neko'.tree))" > gen/gen_neko_api.lua

echo "API: Native C (source: defs_neko_core)"
lua -e "print(require('lib.gen_api_native')(require'defs_neko_core'.tree, 'gen'))"

# cd ../imgui

echo "API: ImGui (source: cimgui.json)"
lua gen_imgui.lua