-- escape a string to be included in another string
function ng.safestr(s)
    return ("%q"):format(s):gsub("\010", "n"):gsub("\026", "\\026")
end
