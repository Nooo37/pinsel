#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "config.h"
#include "action.h"
#include "ui_state.h"
#include "pixbuf.h"
#include "utils.h"

lua_State *L;

extern void config_perform_action(Action *action)
{
    switch (action->type) {
        case ZOOM: case MOVE_HORIZONTALLY: case MOVE_VERTICALLY:
        case FIT_POSITION: case QUIT_UNSAFE: case SAVE_AS: case OPEN:
        case SWITCH_MODE: case SWITCH_COLORS: case SET_COLOR1: case SET_COLOR2:
            ui_perform_action(action);
        default:
            pix_perform_action(action);
    }
}

extern void config_perform_self_contained_action(ActionType type)
{
    Action temp;
    temp.type = type;
    config_perform_action(&temp);
}

static int config_zoom(lua_State *L)
{
    double zoom = luaL_checknumber(L, 1);
    Action temp;
    temp.type = ZOOM;
    temp.zoom = zoom;
    config_perform_action(&temp);
    return 1;
}

static int config_move(lua_State *L)
{
    double delta_y = luaL_checknumber(L, 1);
    double delta_x = luaL_checknumber(L, 2);
    Action temp;
    temp.type = MOVE_VERTICALLY;
    temp.move = delta_x;
    config_perform_action(&temp);
    temp.type = MOVE_HORIZONTALLY;
    temp.move = delta_y;
    config_perform_action(&temp);
    return 1;
}

static int config_flip(lua_State *L)
{
    gboolean vertically = lua_toboolean(L, 1);
    Action temp;
    temp.type = vertically ? FLIP_HORIZONTALLY : FLIP_VERTICALLY;
    config_perform_action(&temp);
    return 1;
}

static int config_rotate(lua_State *L)
{
    gboolean counterclockwise = lua_toboolean(L, 1);
    Action temp;
    temp.type = counterclockwise ? ROTATE_COUNTERCLOCKWISE : ROTATE_CLOCKWISE;
    config_perform_action(&temp);
    return 1;
}

static int config_undo_all(lua_State *L)
{
    Action temp;
    temp.type = UNDO_ALL;
    config_perform_action(&temp);
    return 1;
}

static int config_undo(lua_State *L)
{
    Action temp;
    temp.type = UNDO;
    config_perform_action(&temp);
    return 1;
}

static int config_redo(lua_State *L)
{
    Action temp;
    temp.type = REDO;
    config_perform_action(&temp);
    return 1;
}

static int config_quit_unsafe(lua_State *L)
{
    Action temp;
    temp.type = QUIT_UNSAFE;
    config_perform_action(&temp);
    return 1;
}

static int config_save(lua_State *L)
{
    Action temp;
    temp.type = SAVE;
    config_perform_action(&temp);
    return 1;
}

static int config_save_as(lua_State *L)
{
    Action temp;
    temp.type = SAVE_AS;
    config_perform_action(&temp);
    return 1;
}

static int config_open(lua_State *L)
{
    Action temp;
    temp.type = OPEN;
    config_perform_action(&temp);
    return 1;
}

static int config_set_mode(lua_State *L)
{
    double mode = luaL_checknumber(L, 1);
    Action temp;
    temp.type = SWITCH_MODE;
    temp.mode = (Mode) (int) mode;
    config_perform_action(&temp);
    return 1;
}

static int config_set_color1(lua_State *L)
{
    double r = luaL_checknumber(L, 1);
    double g = luaL_checknumber(L, 2);
    double b = luaL_checknumber(L, 3);
    double a = luaL_checknumber(L, 4);
    GdkRGBA color;
    color.red = r;
    color.green = g;
    color.blue = b;
    color.alpha = a;
    Action temp;
    temp.type = SET_COLOR1;
    temp.color = color;
    config_perform_action(&temp);
    return 1;
}

static int config_set_color2(lua_State *L)
{
    double r = luaL_checknumber(L, 1);
    double g = luaL_checknumber(L, 2);
    double b = luaL_checknumber(L, 3);
    double a = luaL_checknumber(L, 4);
    GdkRGBA color;
    color.red = r;
    color.green = g;
    color.blue = b;
    color.alpha = a;
    Action temp;
    temp.type = SET_COLOR2;
    temp.color = color;
    config_perform_action(&temp);
    return 1;
}


static int config_switch_colors(lua_State *L)
{
    Action temp;
    temp.type = SWITCH_COLORS;
    config_perform_action(&temp);
    return 1;
}

static int config_get_mode(lua_State *L)
{
    lua_pushinteger(L, ui_get_mode());
    return 1;
}


static void perform_lua_keybind(char* keypress, Modifiers mods)
{
    lua_settop(L, 0);
    lua_getglobal(L, "pinsel_api");
    lua_pushstring(L, "on_key");
    lua_gettable(L, -2);
    if (lua_isnil(L, 1))
        return;
    lua_pushstring(L, keypress);
    lua_newtable(L);
    lua_pushstring(L, "shift");
    lua_pushboolean(L, mods.shift);
    lua_settable( L, -3 );
    lua_pushstring(L, "alt");
    lua_pushboolean(L, mods.alt);
    lua_settable( L, -3 );
    lua_pushstring(L, "control");
    lua_pushboolean(L, mods.control);
    lua_settable( L, -3 );
    lua_call(L, 2, 0);
    lua_pop(L, 1);
}

extern char* config_get_shortcut_ui()
{
    lua_settop(L, 0);
    lua_getglobal(L, "pinsel");
    lua_pushstring(L, "get_shortcut_dialog");
    lua_gettable(L, -2);
    if (lua_isnil(L, 1))
        exit(1);
    lua_call(L, 0, 1);
    if (!lua_isstring(L, -1))
        exit(1);
    const char* res = lua_tostring(L, -1);
    return res;
}


extern int config_init(char* config_file)
{
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_newtable(L);

    static const luaL_Reg l[] = {
        /* { "discard",       config_zoom }, */
        /* { "flush",       config_zoom }, */
        { "zoom",       config_zoom },
        { "rotate",     config_rotate},
        { "flip",       config_flip},
        { "move",       config_move},
        { "set_mode",   config_set_mode },
        { "undo_all",   config_undo_all },
        { "undo",       config_undo },
        { "redo",       config_redo },
        { "save",       config_save },
        { "save_as",    config_save_as },
        { "open",       config_open },
        { "quit",       config_quit_unsafe },
        { "set_color1", config_set_color1 },
        { "set_color2", config_set_color2 },
        { "switch_colors", config_switch_colors },
        /* to be removed */
        { "get_mode", config_get_mode },
        { NULL, NULL }
    };

    luaL_newlib(L, l);
    lua_setglobal(L, "pinsel_api");

    lua_pushstring(L, "C-");
    lua_setglobal(L, "CONTROL");

    lua_pushstring(L, "M-");
    lua_setglobal(L, "ALT");

    lua_pushinteger(L, BRUSH);
    lua_setglobal(L, "BRUSH_MODE");
    lua_pushinteger(L, ERASER);
    lua_setglobal(L, "ERASER_MODE");
    lua_pushinteger(L, TEXT);
    lua_setglobal(L, "TEXT_MODE");

    GInputStream* pinsel_stream = g_resources_open_stream("/data/pinsel.lua", 
                    G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    int buffer_size = 10000; // TODO: change
    char buffer[buffer_size];
    gsize count;
    g_input_stream_read_all(pinsel_stream, buffer, buffer_size, &count, NULL,  NULL);

    luaL_dostring(L, buffer);
    int status = luaL_dofile(L, config_file);
    if (status) {
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        return 0;
    }

    return 1;
}

extern void config_perform_key_event(char *key, Modifiers mod)
{
    perform_lua_keybind(key, mod);
}

