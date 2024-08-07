-- 任何导出 C 函数/变量 f 都可以作为 ng.f 使用
-- 例如C函数 vec2(...) 可用为 ng.vec2(...) 在 Lua 中
local ffi = FFI
ng = setmetatable({}, {
    __index = ffi.C
})

-- 命令行参数
ng.args = nekogame_args

-- 有用的 ffi 函数
ng.string = ffi.string

-- 导入所有内容

hot_require 'nekogame.util'
hot_require 'nekogame.struct'
hot_require 'nekogame.entity_table'

hot_require 'nekogame.system'
hot_require 'nekogame.entity'
hot_require 'nekogame.name'
hot_require 'nekogame.group'
hot_require 'nekogame.input'
hot_require 'nekogame.gui'
hot_require 'nekogame.edit'
hot_require 'nekogame.animation'
hot_require 'nekogame.bump'
hot_require 'nekogame.physics'
hot_require 'nekogame.sound'

hot_require 'nekogame.console'
hot_require 'nekogame.fps'
hot_require 'nekogame.inspector'

print("nekogame.lua loaded")