local pinsel = pinsel
local CONTROL, ALT = CONTROL, ALT
local BRUSH_MODE, ERASER_MODE, TEXT_MODE = BRUSH_MODE, ERASER_MODE, TEXT_MODE

local DELTA = 10
local ZOOM = 0.4

local temp_keys, keys, mode_keys

mode_keys = {
    { "b", function() pinsel.set_mode(BRUSH_MODE); pinsel.set_keys(temp_keys) end },
    { "e", function() pinsel.set_mode(ERASER_MODE); pinsel.set_keys(temp_keys) end },
    { "t", function() pinsel.set_mode(TEXT_MODE); pinsel.set_keys(temp_keys) end },
}

keys = {
    { "d", function() pinsel.set_width(20) end},
    { "f", function() pinsel.draw({{0, 0}, {1000, 1000}}) end},
    { "v", function() pinsel.draw({{0, 0}, {1000, 1000}}) end},
    { "g", function() pinsel.erase({{0, 100}, {100, 0}}) end},
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
    { "a",   function () pinsel.set_color1(math.random(), math.random(), math.random(), math.random()) end, "GUI", "randomize the current color" },
    { "s",   pinsel.switch_colors, "GUI", "switch the two colors" },
    { "C-t", function() temp_keys = keys; pinsel.set_keys(mode_keys) end, "GUI", "modal binding for modes" },
    { "M-x", pinsel.undo_all, "History", "undo all changes" },
    { "C-z", pinsel.undo, "History", "undo" },
    { "C-Z", pinsel.redo, "History", "redo" },
    { "q",   function() pinsel.save() pinsel.quit() end, "Quit, Save, Open", "quit with saving" },
    { "w",   pinsel.quit,    "Quit, Save, Open", "quit without saving" },
    { "C-o", pinsel.open,    "Quit, Save, Open", "open a new image" },
    { "C-s", pinsel.save,    "Quit, Save, Open", "save the current image" },
    { "C-S", pinsel.save_as, "Quit, Save, Open", "save the image to a (new) file" },
    { "scroll_up", function() pinsel.zoom( ZOOM) end, "Navigation", "zoom in" },
    { "scroll_down", function() pinsel.zoom(-ZOOM) end, "Navigation", "zoom out" },
}
pinsel.set_keys(keys)

pinsel.set_mode(ERASER_MODE);

pinsel.set_width(30)
pinsel.set_color1(0, 0, 0, 1)
pinsel.set_color2(1, 0, 0, 1)

local function is_erase(mod) return mod.button3 end
local function is_brush(mod) return mod.button1 end
local function is_drag(mod) return mod.button2 end

local activity = "idle"
local start_x, start_y
local coords = {}

pinsel.on_mouse = function(x, y, mod)

    if is_brush(mod) then activity = "brush" end
    if is_erase(mod) then activity = "erase" end
    if activity == "brush" then
        coords[#coords + 1] = { x, y }
        if is_brush(mod) then
            pinsel.discard()
            pinsel.draw(coords)
        else
            activity = "idle"
            pinsel.discard()
            pinsel.draw(coords)
            pinsel.flush()
            coords = {}
        end
    end

    if activity == "erase" then
        coords[#coords + 1] = { x, y }
        if is_erase(mod) then
            pinsel.discard()
            pinsel.erase(coords)
        else
            activity = "idle"
            pinsel.discard()
            pinsel.erase(coords)
            pinsel.flush()
            coords = {}
        end
    end

    if is_drag(mod) then
        if activity == "drag" then
            pinsel.move(x - start_x, y - start_y)
        else
            start_x = x
            start_y = y
            activity = "drag"
        end
    elseif activity == "drag" then
        activity = "idle"
    end
end

pinsel.on_click = function(a, x, y, mod)
        -- TODO
        -- print(a, x, y, mod.control)
end

