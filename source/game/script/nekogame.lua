-- 任何导出 C 函数/变量 f 都可以作为 ng.f 使用
-- 例如C函数 vec2(...) 可用为 ng.vec2(...) 在 Lua 中
local ffi = require 'ffi'
ng = setmetatable({}, {
    __index = ffi.C
})

-- 命令行参数
ng.args = nekogame_args

-- 有用的 ffi 函数
ng.string = ffi.string

-- 导入所有内容

require 'nekogame.util'
require 'nekogame.struct'
require 'nekogame.entity_table'

require 'nekogame.system'
require 'nekogame.entity'
require 'nekogame.name'
require 'nekogame.group'
require 'nekogame.input'
require 'nekogame.gui'
require 'nekogame.edit'
require 'nekogame.animation'
require 'nekogame.bump'
require 'nekogame.physics'
require 'nekogame.sound'

require 'nekogame.console'
require 'nekogame.fps'
require 'nekogame.inspector'
