--- 检查该路径中是否存在文件或目录
function exists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
        if code == 13 then
            -- 被拒绝 但存在
            return true
        end
    end
    return ok, err
end

--- 检查该路径中是否存在目录
function isdir(path)
    return exists(path .. "/") -- "/" 适用于 Unix 和 Windows
end

local function create_directory_recursive(path)
    local cmd = string.format('mkdir "%s"', path)
    os.execute(cmd)
    print(("创建目录 %s"):format(path))
end

function download_file(url, local_file)

    if not isdir(neko.file_path("neko_cache")) then
        create_directory_recursive(neko.file_path("neko_cache"))
    end

    -- local_file = neko.file_path("neko_cache/" .. local_file)

    if exists(local_file) then
        return
    end

    print("正在下载运行必要文件", local_file)
    -- local command, result
    -- command = string.format('curl -o %s %s', local_file, url)
    -- result = os.execute(command)

    local file = io.open(local_file, "wb")
    if not file then
        error("Could not open file for writing: " .. local_file)
    end

    local http = require 'http'

    local status_code, data = http.request(url)

    file:write(data)

    file:close()

    if status_code == 200 then
        -- print("File downloaded successfully!")
    else
        print("failed to download file. status_code: " .. status_code)
    end

end

function load_libs_from_url(url, local_file)

    local_file = neko.file_path("neko_cache/" .. local_file)

    if exists(local_file) then
        goto load_libs
    end

    download_file(url, local_file)

    ::load_libs::

    return default_require(local_file:gsub(".lua", ""))
end

function test_load_libs()
    load_libs_from_url("http://raw.gitmirror.com/bakpakin/binser/master/binser.lua", "binser.lua")
    load_libs_from_url("http://raw.gitmirror.com/kikito/bump.lua/master/bump.lua", "bump.lua")
    load_libs_from_url("http://raw.gitmirror.com/rxi/flux/master/flux.lua", "flux.lua")
    load_libs_from_url("http://raw.gitmirror.com/rxi/json.lua/master/json.lua", "json.lua")

    local inspect = load_libs_from_url("https://raw.gitmirror.com/kikito/inspect.lua/master/inspect.lua", "inspect.lua")

    print(inspect({1, 2, 3, {1, 22}}))
end

