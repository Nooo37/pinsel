pinsel = pinsel

-- helper functions

local modifiers = { "button1", "button2", "button3", "shift", "alt", "control" }

local function merge_mods(mod1, mod2)
    local mod = {}
    for _, modifier in ipairs(modifiers) do
        mod[modifier] = mod1[modifier] or mod2[modifier]
    end
    return mod
end

local function are_modifier_equal(mod1, mod2)
    for _, modifier in ipairs(modifiers) do
        if (mod1[modifier] or nil) ~= (mod2[modifier] or nil) then
            return false
        end
    end
    return true
end

local function table_contains(t, v)
    for _, w in ipairs(t) do
        if w == v then return true end
    end
    return false
end

-- Key class and key global variables

Key = { mt = { } }

function Key.new(key, mod)
    local ret = {}
    ret.__index = Key
    ret.key = key
    ret.mod = mod
    return setmetatable(ret, Key.mt)
end

function Key.mt:__eq(other)
    return self.key == other.key and are_modifier_equal(self.mod, other.mod)
end

function Key.mt:__add(other)
    assert((not self.key) or (not other.key), "Adding keys only works for one modifier-only and one non-modifier-only key")
    return Key.new(self.key or other.key, merge_mods(self.mod, other.mod))
end

function Key.mt:__tostring()
    local res = ""
    if self.mod.shift then res = res .. "&lt;shift&gt;" end
    if self.mod.control then res = res .. "&lt;ctrl&gt;" end
    if self.mod.alt then res = res .. "&lt;Mod1&gt;" end
    -- if self.mod.shift then res = res .. "<shift>" end
    -- if self.mod.control then res = res .. "<ctrl>" end
    -- if self.mod.alt then res = res .. "<Mod1>" end
    if self.key then res = res .. self.key end
    return res
end

setmetatable(Key, Key.mt)

A_ = Key.new("a", {})
B_ = Key.new("b", {})
C_ = Key.new("c", {})
D_ = Key.new("d", {})
E_ = Key.new("e", {})
F_ = Key.new("f", {})
G_ = Key.new("g", {})
H_ = Key.new("h", {})
I_ = Key.new("i", {})
J_ = Key.new("j", {})
K_ = Key.new("k", {})
L_ = Key.new("l", {})
M_ = Key.new("m", {})
N_ = Key.new("n", {})
O_ = Key.new("o", {})
P_ = Key.new("p", {})
Q_ = Key.new("q", {})
R_ = Key.new("r", {})
S_ = Key.new("s", {})
T_ = Key.new("t", {})
U_ = Key.new("u", {})
V_ = Key.new("v", {})
W_ = Key.new("w", {})
X_ = Key.new("x", {})
Y_ = Key.new("y", {})
Z_ = Key.new("z", {})
ScrollUp_ = Key.new("scroll_up", {})
ScrollDown_ = Key.new("scroll_down", {})
Shift_ = Key.new(nil, { shift = true })
Alt_ = Key.new(nil, { alt = true })
Control_ = Key.new(nil, { control = true })

-- providing functions to the user by adding to the pinsel global

local keys = {}

pinsel.translate = function(x_raw, y_raw)
    local geo = pinsel.get_geo()
    local x = (x_raw - geo.offset_x - geo.mid_x) / geo.scale
    local y = (y_raw - geo.offset_y - geo.mid_y) / geo.scale
    return x, y
end

pinsel.move = function(delta_x, delta_y)
    local geo = pinsel.get_geo()
    pinsel.set_geo({
        offset_x = geo.offset_x + delta_x,
        offset_y = geo.offset_y + delta_y,
    })
end

pinsel.zoom = function(delta)
    local geo = pinsel.get_geo()
    pinsel.set_geo({ scale = geo.scale + delta })
end

pinsel.set_keys = function(t) keys = t end

local function get_all_categories()
    local res = { }
    for _, key in ipairs(keys) do
        if key and key[3] then
            if not table_contains(res, key[3]) then
                res[#res + 1] = key[3]
            end
        end
    end
    return res
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
            if key[1] and key[3] and key[3] == cat and key[1].key ~= "scroll_up" and key[1].key ~= "scroll_down" then
                local desc = key[4] or "no description provided"
                res = res .. [[
              <child>
                <object class="GtkShortcutsShortcut">
                  <property name="visible">1</property>
                  <property name="title" translatable="yes" context="shortcut window">]] .. desc .. [[</property>
                  <property name="accelerator">]] .. tostring(key[1]) .. [[</property>
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

pinsel.history_limit = 20

