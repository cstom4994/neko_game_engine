-- rebind Lua 'print' to print to console
function console_print(...)
    local args = table.pack(...)
    local sargs = {}
    for i = 1, args.n do
        sargs[i] = tostring(args[i])
    end
    ns.console.puts(table.concat(sargs, '    '))
end
