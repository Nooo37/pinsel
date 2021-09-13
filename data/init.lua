-- default pinsel config

local pinsel = pinsel
local BRUSH_MODE, ERASER_MODE, TEXT_MODE = BRUSH_MODE, ERASER_MODE, TEXT_MODE

-- state variables

local DELTA = 10
local ZOOM = 0.1
local x_start, y_start
local old_offset_x, old_offset_y
local x_text = 0
local y_text = 0
local text = ""
local temp_keys, keys, mode_keys
local activity = "idle"

-- helper functions

local function is_brush(mod)
    local mode = pinsel.get_mode()
    return (mode == BRUSH_MODE and mod.button1 and not mod.shift)
end

local function is_erase(mod)
    local mode = pinsel.get_mode()
    return (mode == BRUSH_MODE and mod.button3)
end

local function is_crop(mod)
    local mode = pinsel.get_mode()
    return (mode == ERASER_MODE and mod.button1)
end

local function is_line(mod)
    return pinsel.get_mode() == BRUSH_MODE and mod.button1 and mod.shift
end

local function is_drag(mod) return mod.button2 end

-- keybindings and other callbacks

mode_keys = {
    { B_, function() pinsel.set_mode(BRUSH_MODE); keys = temp_keys end },
    { E_, function() pinsel.set_mode(ERASER_MODE); keys = temp_keys end },
    { T_, function() pinsel.set_mode(TEXT_MODE); keys = temp_keys end },
}

keys = {
    { Q_,   function() pinsel.save() pinsel.quit() end, "Quit, Save, Open", "quit with saving" },
    { W_,   pinsel.quit, "Quit, Save, Open", "quit without saving" },
    { S_,   pinsel.save, "Quit, Save, Open", "save the current image" },
    { H_,   function() pinsel.move(   DELTA,       0 ) end, "Navigation", "move left" },
    { L_,   function() pinsel.move( - DELTA,       0 ) end, "Navigation", "move right" },
    { J_,   function() pinsel.move(       0, - DELTA ) end, "Navigation", "move down" },
    { K_,   function() pinsel.move(       0,   DELTA ) end, "Navigation", "move up" },
    { U_,   function() pinsel.rotate(false) end, "Manipulation", "rotate clockwise" },
    { I_,   function() pinsel.rotate(true) end,  "Manipulation", "rotate counterclockwise" },
    { Y_,   function() pinsel.flip(false) end,   "Manipulation", "flip horizontally" },
    { O_,   function() pinsel.flip(true) end,    "Manipulation", "flip vertically" },
    { A_,   function() pinsel.set_color1(math.random(), math.random(), math.random(), 1) end, "GUI", "random color" },
    { ScrollUp_, function() pinsel.zoom(ZOOM) end, "Navigation", "zoom in" },
    { ScrollDown_, function() pinsel.zoom(-ZOOM) end, "Navigation", "zoom out" },
    { Control_ + T_, function() temp_keys = keys; keys = mode_keys end, "GUI", "modal binding for modes (b, e or t should follow)" },
    { Alt_ + X_, pinsel.undo_all, "History", "undo all changes" },
    { Control_ + Z_, pinsel.undo, "History", "undo" },
    { Control_ + Shift_ + Z_, pinsel.redo, "History", "redo" },
    { Control_ + O_, pinsel.open, "Quit, Save, Open", "open a new image" },
    { Control_ + S_, pinsel.save, "Quit, Save, Open", "save the current image" },
    { Control_ + Shift_ + S_, pinsel.save_as, "Quit, Save, Open", "save the image to a (new) file" },
}

pinsel.set_keys(keys)

pinsel.on_key = function(key, mod)
    local pressed_key = Key.new(key, mod)
    for _, k in ipairs(keys) do
        if k[1] == pressed_key then k[2]() end
    end
end

pinsel.on_click = function(b, x_raw, y_raw, mod)
    local x, y = pinsel.translate(x_raw, y_raw)
    mod.button1 = (b == 1)
    if pinsel.get_mode() == BRUSH_MODE and is_brush(mod) then
        pinsel.on_motion(x_raw, y_raw, mod)
    elseif pinsel.get_mode() == TEXT_MODE then
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

pinsel.on_motion = function(x_raw, y_raw, mod)
    local x, y = pinsel.translate(x_raw, y_raw)
    if is_brush(mod) then activity = "brush" end
    if is_erase(mod) then activity = "erase" end
    local is_drag_bool = is_drag(mod)
    local is_line_bool = is_line(mod)

    if activity == "brush" then
        pinsel.discard()
        pinsel.path_add(x, y)
        pinsel.draw()
        if not is_brush(mod) then
            pinsel.apply()
            pinsel.path_clear()
            activity = "idle"
        end
        return
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
        return
    end

    if is_line_bool or activity == "line" then
        if is_line_bool and activity ~= "line" then
            x_start = x
            y_start = y
            activity = "line"
        end
        pinsel.discard()
        pinsel.path_clear()
        pinsel.path_add(x_start, y_start)
        pinsel.path_add(x, y)
        pinsel.draw()
        if not is_line_bool then
            pinsel.apply()
            pinsel.path_clear()
            activity = "idle"
        end
        return
    end

    if is_crop(mod) or activity == "crop" then
        if is_crop(mod) and activity ~= "crop" then
            x_start = x
            y_start = y
            activity = "crop"
        end
        pinsel.discard()
        pinsel.path_clear()
        pinsel.path_add(x_start, y_start)
        pinsel.path_add(x_start, y)
        pinsel.path_add(x, y)
        pinsel.path_add(x, y_start)
        pinsel.path_add(x_start, y_start)
        pinsel.draw()
        if not is_crop(mod) then
            pinsel.discard()
            pinsel.path_clear()
            pinsel.crop(x_start, y_start, x - x_start, y - y_start)
            pinsel.apply()
            activity = "idle"
        end
        return
    end

    if is_drag_bool or activity == "drag" then
        if is_drag_bool and activity == "drag" then
            pinsel.move(x_raw - x_start, y_raw - y_start)
            x_start = x_raw
            y_start = y_raw
        elseif is_drag_bool then
            x_start = x_raw
            y_start = y_raw
            activity = "drag"
        else
            activity = "idle"
        end
        return
    end

    if pinsel.get_mode() == TEXT_MODE and mod.button1 then
        x_text = x
        y_text = y
        if activity == "text" then
            pinsel.discard()
            pinsel.text(text, x_text, y_text)
        end
        return
    end
end

pinsel.on_text_change = function(t)
    text = t
    pinsel.discard()
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
pinsel.set_width(10)
pinsel.history_limit = 20

