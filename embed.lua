local outputDir = "source/gen"

local luaFiles = {"source/engine/bootstrap.lua", "source/engine/nekogame.lua", "source/engine/nekoeditor.lua"}

local function runembed()
    for _, luaFilePath in ipairs(luaFiles) do
        local file = luaFilePath:match("([^/]+)%.lua$")
        local cppFilePath = outputDir .. "/" .. file .. "_embedded.cpp"
    
        local luaFile = io.open(luaFilePath, "r")
        local content = luaFile:read("*a")
        luaFile:close()
    
        local cppFile = io.open(cppFilePath, "w")
        cppFile:write("#include <cstdint>\n\n")
        local varName = file .. "_lua"
        cppFile:write("static const uint8_t " .. varName .. "[] = {")
    
        for i = 1, #content do
            local byte = string.byte(content, i)
            cppFile:write(string.format("0x%02X", byte))
            if i < #content then
                cppFile:write(", ")
            end
            if i % 15 == 1 then
                cppFile:write("\n")
            end
        end
        cppFile:write(", 0x00\n};\n\n")
        cppFile:write("const uint8_t* get_" .. varName .. "() { return " .. varName .. "; }")
        cppFile:close()
    
        print("embed", luaFilePath, cppFilePath)
    end
end

local function runlua(name)
    return function(config)
        assert(config.input, "Missing 'input' in config")
        assert(config.output, "Missing 'output' in config")

        local lua_cmd = {config.exe or "lua", config.script}

        if config.args then
            for _, arg in ipairs(config.args) do
                if type(arg) == "table" then
                    for _, flag in ipairs(arg) do -- 展开嵌套 table
                        table.insert(lua_cmd, flag)
                    end
                else
                    table.insert(lua_cmd, arg)
                end
            end
        end

        for i, v in ipairs(lua_cmd) do
            lua_cmd[i] = tostring(v):gsub("%$out", config.output):gsub("%$in", config.input)
        end

        local cmd = table.concat(lua_cmd, " ")
        print("runlua", table.unpack(lua_cmd))
        local success, exit_type, code = os.execute(cmd)
        if not success or exit_type ~= "exit" or code ~= 0 then
            error(string.format("Error running %s: %s", name, cmd))
        end
    end
end

local function runluaot()
    runlua("luaot_nekogame") {
        exe = "D:/Projects/Neko/DevNew/bin/luaot",
        args = {"$in", "-o", "$out"},
        input = "source/engine/nekogame.lua",
        output = outputDir .. "/" .. "nekogame_luaot.c"
    }

    runlua("luaot_nekoeditor") {
        exe = "D:/Projects/Neko/DevNew/bin/luaot",
        args = {"$in", "-o", "$out"},
        input = "source/engine/nekoeditor.lua",
        output = outputDir .. "/" .. "nekoeditor_luaot.c"
    }
end

runembed()
-- runluaot()