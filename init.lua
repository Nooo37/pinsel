local pinsel = _pinsel

local DELTA = 10

local keys = {
    { "h", function() pinsel.move_horizontally(DELTA) end },
    { "l", function() pinsel.move_horizontally(-DELTA) end },
    { "j", function() pinsel.move_vertically(-DELTA) end },
    { "k", function() pinsel.move_vertically(DELTA) end },
    { "q", function() pinsel.quit() end },
}

function Onkey(keypress)
    for _, entry in ipairs(keys) do
        if entry[1] == keypress then entry[2]() return end
    end
    print(keypress)
end
