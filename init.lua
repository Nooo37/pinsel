local pinsel = _pinsel
local CONTROL, ALT = CONTROL, ALT

local DELTA = 10
local ZOOM = 0.1

local keys = {
    { "h",            function() pinsel.move_horizontally(DELTA) end },
    { "l",            function() pinsel.move_horizontally(-DELTA) end },
    { "j",            function() pinsel.move_vertically(-DELTA) end },
    { "k",            function() pinsel.move_vertically(DELTA) end },
    { "+",            function() pinsel.zoom(ZOOM) end },
    { "-",            function() pinsel.zoom(ZOOM) end },
    { "u",            pinsel.rotate_clockwise },
    { "i",            pinsel.rotate_counterclockwise },
    { "y",            pinsel.flip_horizontally },
    { "o",            pinsel.flip_vertically },
    { "w",            pinsel.quit },
    { "q",            function() pinsel.save() pinsel.quit() end },
    { CONTROL .. "o", pinsel.open },
    { CONTROL .. "s", pinsel.save },
    { CONTROL .. "z", pinsel.undo },
    { CONTROL .. "Z", pinsel.redo },
    { CONTROL .. "S", pinsel.save_as },
}

function Onkey(keypress)
    for _, entry in ipairs(keys) do
        if entry[1] == keypress then entry[2]() return end
    end
    print(keypress)
end
