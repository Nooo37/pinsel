local pinsel = pinsel
local BRUSH_MODE, ERASER_MODE, TEXT_MODE = BRUSH_MODE, ERASER_MODE, TEXT_MODE

local DELTA = 10
local ZOOM = 0.1

local temp_keys, keys, mode_keys

local activity = "idle"

local function is_brush(mod)
    local mode = pinsel.get_mode()
    return (mode == BRUSH_MODE and mod.button1 and not mod.shift) or
           (mode == ERASER_MODE and mod.button3)
end
local function is_erase(mod)
    local mode = pinsel.get_mode()
    return (mode == BRUSH_MODE and mod.button3) or
           (mode == ERASER_MODE and mod.button1)
end
local function is_line(mod)
    return pinsel.get_mode() == BRUSH_MODE and mod.button1 and mod.shift
end
local function is_drag(mod) return mod.button2 end
local x_start, y_start
local x_line, y_line
local x_text = 0
local y_text = 0
local text = ""

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
    { "f", function() pinsel.set_geo({scale = 0.1, offset_x = 0, offset_y = 0}) end, "Test", "hey"},
    { "scroll_up", function() pinsel.zoom( ZOOM) end, "Navigation", "zoom in" },
    { "scroll_down", function() pinsel.zoom(-ZOOM) end, "Navigation", "zoom out" },
    { "u",   function() pinsel.rotate(false) end, "Manipulation", "rotate clockwise" },
    { "i",   function() pinsel.rotate(true) end,  "Manipulation", "rotate counterclockwise" },
    { "y",   function() pinsel.flip(false) end,   "Manipulation", "flip horizontally" },
    { "o",   function() pinsel.flip(true) end,    "Manipulation", "flip vertically" },
    { "a",   function() pinsel.set_color1(math.random(), math.random(), math.random(), 1) end, "GUI", "random color" },
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

pinsel.on_key = function(key, mod)
    if mod.alt then key = "M-" .. key end
    if mod.control then key = "C-" .. key end
    for _, k in ipairs(keys) do
        if k[1] == key then k[2]() end
    end
end

pinsel.on_click = function(b, x, y, mod)
    if pinsel.get_mode() == TEXT_MODE then
        x_text = x
        y_text = y
        if activity == "text" then
            pinsel.discard()
            pinsel.text(text, x_text, y_text)
        else
            activity = "text"
            pinsel.open_text_input()
        end
    end
end

pinsel.on_motion = function(x, y, mod)
    if is_brush(mod) then activity = "brush" end
    if is_erase(mod) then activity = "erase" end

    if activity == "brush" then
        pinsel.discard()
        pinsel.path_add(x, y)
        pinsel.draw()
        if not is_brush(mod) then
            pinsel.apply()
            pinsel.path_clear()
            activity = "idle"
        end
    end

    if activity == "erase" then
        pinsel.discard()
        pinsel.path_add(x, y)
        pinsel.erase()
        if not is_erase(mod) then
            pinsel.apply()
            pinsel.path_clear()
            activity = "idle"
        end
    end

    if is_line(mod) or activity == "line" then
        if is_line(mod) and activity ~= "line" then
            x_line = x
            y_line = y
            activity = "line"
        end
        pinsel.discard()
        pinsel.path_clear()
        pinsel.path_add(x_line, y_line)
        pinsel.path_add(x, y)
        pinsel.draw()
        if not is_line(mod) then
            pinsel.apply()
            pinsel.path_clear()
            activity = "idle"
        end
    end

    if is_drag(mod) or activity == "drag" then
        if is_drag(mod) and activity == "drag" then
            pinsel.move(x - x_start, y - y_start)
        elseif is_drag(mod) then
            x_start = x
            y_start = y
            activity = "drag"
        else
            activity = "idle"
        end
    end

    if pinsel.get_mode() == TEXT_MODE and mod.button1 then
        x_text = x
        y_text = y
        if activity == "text" then
            pinsel.discard()
            pinsel.text(text, x_text, y_text)
        end
    end
end

pinsel.on_text_change = function(t)
    text = t
    pinsel.text(text, x_text, y_text)
end

pinsel.on_text_close = function(accepted)
    pinsel.discard()
    if accepted then
        pinsel.text(text, x_text, y_text)
        pinsel.apply()
        pinsel.set_mode(BRUSH_MODE)
        activity = "idle"
        text = ""
    end
end

pinsel.set_color1(0, 0, 0, 1)
pinsel.set_color2(0.5, 0, 0, 0.5)
pinsel.set_width(20)
pinsel.history_limit = 1

