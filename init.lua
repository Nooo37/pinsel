local pinsel = pinsel
local CONTROL, ALT = CONTROL, ALT
local BRUSH_MODE, ERASER_MODE, TEXT_MODE = BRUSH_MODE, ERASER_MODE, TEXT_MODE

local DELTA = 10
local ZOOM = 0.4

local temp_keys, keys, mode_keys

mode_keys = {
    { "b", function() pinsel.set_mode(BRUSH_MODE); keys = temp_keys end },
    { "e", function() pinsel.set_mode(ERASER_MODE); keys = temp_keys end },
    { "t", function() pinsel.set_mode(TEXT_MODE); keys = temp_keys end },
}

keys = {
    { "h",   function() pinsel.move(   DELTA,       0 ) end, "Navigation", "move left" },
    { "l",   function() pinsel.move( - DELTA,       0 ) end, "Navigation", "move right" },
    { "j",   function() pinsel.move(       0, - DELTA ) end, "Navigation", "move down" },
    { "k",   function() pinsel.move(       0,   DELTA ) end, "Navigation", "move up" },
    { "C-m", function() pinsel.zoom( ZOOM) end,              "Navigation", "zoom in" },
    { "C-n", function() pinsel.zoom(-ZOOM) end,              "Navigation", "zoom out" },
    { "u",   function() pinsel.rotate(false) end, "Manipulation", "rotate clockwise" },
    { "i",   function() pinsel.rotate(true) end,  "Manipulation", "rotate counterclockwise" },
    { "y",   function() pinsel.flip(false) end,   "Manipulation", "flip horizontally" },
    { "o",   function() pinsel.flip(true) end,    "Manipulation", "flip vertically" },
    { "a",   function () pinsel.set_color1(0, 0, 0, 1) end, "GUI", "make the current color black" },
    { "s",   pinsel.switch_colors, "GUI", "switch the two colors" },
    { "C-t", function() temp_keys = keys; keys = mode_keys end, "GUI", "modal binding for modes" },
    { "M-x", pinsel.undo_all, "History", "undo all changes" },
    { "C-z", pinsel.undo, "History", "undo" },
    { "C-Z", pinsel.redo, "History", "redo" },
    { "q",   function() pinsel.save() pinsel.quit() end, "Quit, Save, Open", "quit with saving" },
    { "w",   pinsel.quit,    "Quit, Save, Open", "quit without saving" },
    { "C-o", pinsel.open,    "Quit, Save, Open", "open a new image" },
    { "C-s", pinsel.save,    "Quit, Save, Open", "save the current image" },
    { "C-S", pinsel.save_as, "Quit, Save, Open", "save the image to a (new) file" },
}
pinsel.set_keys(keys)

pinsel.set_color1(0.5, 0, 0, 0.5)
pinsel.set_color2(1, 1, 1, 1)

