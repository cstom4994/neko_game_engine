
cd source/api

if [ ! -d output ]; then
    mkdir gen
fi

# echo "Debug output: tag raw tree. this is before being it's processed"
# lua -e "print((require 'lib.tag.tag_pp')(require 'defs_neko'.raw_tree) .. '\n')" > output/dbg/_neko_tagtree.txt
# lua -e "print((require 'lib.tag.tag_pp')(require 'defs_neko_core'.raw_tree) .. '\n')" > output/dbg/_neko_core_tagtree.txt

# echo "Debug output:  kikito's inspect output (lua table format) (post-processed tree)"
# lua -e "print('return ' .. (require 'lib.tag.inspect')(require'defs_neko'.tree, {indent='    '}) .. '\n')" > output/dbg/_neko_tree.inspect.lua
# lua -e "print('return ' .. (require 'lib.tag.inspect')(require'defs_neko_core'.tree, {indent='    '}) .. '\n')" > output/dbg/_neko_core_tree.inspect.lua

# echo "Debug output:  fennel (fennel tree) (post-processed tree)"
# lua -e "print((require 'lib.tag.fennel_lib').view(require'defs_neko'.tree) .. '\n')" > output/dbg/_neko_tree.fennel.fnl
# lua -e "print((require 'lib.tag.fennel_lib').view(require'defs_neko_core'.tree) .. '\n')" > output/dbg/_neko_core_tree.fennel.fnl

# echo "Typings: in Lua for VSCode"
# lua -e "print(require('lib.defs_to_docstrings')(require'defs_neko'.tree))" > output/typings/neko.lua
# lua -e "print(require('lib.defs_to_docstrings')(require'defs_neko_core'.tree))" >> output/typings/neko.lua

# echo "Typings: in Typescript (documentation)"
# lua -e "print(require('lib.defs_to_typescript')(require'defs_neko'.tree))" > output/typings/neko.d.ts
# # lua -e "print(require('lib.defs_to_typescript')(require'defs_neko_core'.tree))" >> output/typings/neko.d.ts

# echo "Typings: in Teal (documentation)"
# lua -e "print(require('lib.defs_to_teal')(require'defs_neko'.tree))" > output/typings/neko.d.tl
# # lua -e "print(require('lib.defs_to_teal')(require'defs_neko_core'.tree))" >> output/typings/neko.d.tl

echo "API: Lua (boot) (source: defs_neko)"
lua -e "print(require('lib.gen_api_luaboot')(require'defs_neko'.tree))" > gen/api_neko_gen.lua

echo "API: Native C (source: defs_neko_core)"
lua -e "print(require('lib.gen_api_native')(require'defs_neko_core'.tree, 'gen'))"

# cd ../imgui

echo "API: ImGui (source: cimgui.json)"
lua gen_imgui.lua