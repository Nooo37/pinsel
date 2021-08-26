pinsel = pinsel_api

local keys

local function table_contains(t, v)
    for _, w in ipairs(t) do
        if w == v then return true end
    end
    return false
end

pinsel.on_key = function(keypress)
    for _, entry in ipairs(keys) do
        if entry[1] == keypress then entry[2]() return end
    end
end

pinsel.set_keys = function(t) keys = t end

local function get_all_categories()
    local res = { }
    for _, key in ipairs(keys) do
        if key[3] then
            if not table_contains(res, key[3]) then
                    res[#res + 1] = key[3]
            end
        end
    end
    return res
end

local function notation_to_accel(nota)
    local sing = string.match(nota, "-(.*)")
    if string.sub(nota,1,string.len("C-M-")) == "C-M-" then
        return "&lt;ctrl&gt;" .. sing
    elseif string.sub(nota,1,string.len("C-")) == "C-" then
        return "&lt;ctrl&gt;" .. sing
    elseif  string.sub(nota,1,string.len("M-")) == "M-" then
        return "&lt;Mod1&gt;" .. sing
    else
        return nota
    end
end

function pinsel.get_shortcut_dialog()
    local res = [[
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <object class="GtkShortcutsWindow" id="shortcuts_dialog">
    <property name="modal">1</property>
    <child>
      <object class="GtkShortcutsSection">
        <property name="visible">1</property>
        <property name="section-name">editor</property>
        <property name="title" translatable="yes" context="shortcut window">Editor Shortcuts</property>
    ]]

    local categories = get_all_categories()
    for _, cat in ipairs(categories) do
        res = res .. [[
        <child>
        <object class="GtkShortcutsGroup">
        <property name="visible">1</property>
        <property name="title" translatable="yes" context="shortcut window">]] .. cat .. [[</property>
        ]]
        for _, key in ipairs(keys) do
            if key[3] and key[3] == cat then
                local desc = key[4] or "no description provided"
                res = res .. [[
                    <child>
                      <object class="GtkShortcutsShortcut">
                        <property name="visible">1</property>
                        <property name="title" translatable="yes" context="shortcut window">]] .. desc .. [[</property>
                        <property name="accelerator">]] ..notation_to_accel(key[1]) .. [[</property>
                      </object>
                    </child>
                ]]
            end
        end
        res = res .. [[
        </object>
        </child>
        ]]
    end

    res = res .. [[
      </object>
    </child>
  </object>
</interface>
    ]]
    return res
end

