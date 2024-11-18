local outputDir = "source/gen"
local luaFiles = {"source/engine/bootstrap.lua"}
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

    print("Embedded Lua file: " .. luaFilePath .. " as u8 array in " .. cppFilePath)
end
