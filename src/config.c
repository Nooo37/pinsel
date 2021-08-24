#include <gtk/gtk.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "action.h"
#include "ui_state.h"
#include "pixbuf.h"
#include "utils.h"

lua_State *L;

extern void config_perform_action(Action *action)
{
    switch (action->type) {
        case ZOOM: case MOVE_HORIZONTALLY: case MOVE_VERTICALLY:
        case FIT_POSITION: case QUIT_UNSAFE:
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

static int config_move_horizontally(lua_State *L)
{
    double delta = luaL_checknumber(L, 1);
    Action temp;
    temp.type = MOVE_HORIZONTALLY;
    temp.move = delta;
    config_perform_action(&temp);
    return 1;
}

static int config_move_vertically(lua_State *L)
{
    double delta = luaL_checknumber(L, 1);
    Action temp;
    temp.type = MOVE_VERTICALLY;
    temp.move = delta;
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

static void perform_lua_keybind(char* keypress)
{
    lua_getglobal(L, "Onkey");
    lua_pushstring(L, keypress);
    lua_call(L, 1, 0);
}

extern int config_init()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_newtable(L);

    lua_pushstring(L, "zoom");
    lua_pushcfunction(L, config_zoom);
    lua_settable(L, -3);

    lua_pushstring(L, "move_horizontally");
    lua_pushcfunction(L, config_move_horizontally);
    lua_settable(L, -3);

    lua_pushstring(L, "move_vertically");
    lua_pushcfunction(L, config_move_vertically);
    lua_settable(L, -3);

    lua_pushstring(L, "quit");
    lua_pushcfunction(L, config_quit_unsafe);
    lua_settable(L, -3);

    lua_setglobal(L, "_pinsel");

    int status = luaL_dofile(L, "init.lua");
    if (status) {
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        return 1;
    }

    return 0;
}

extern void config_perform_event(gpointer event)
{
    GdkEventKey *ek = (GdkEventKey*) event;
    char *keypress;
    if (is_no_mod(ek))
        keypress = g_strdup_printf("%c", ek->keyval);
    else 
        keypress = g_strdup_printf("C-%c", ek->keyval);
    perform_lua_keybind(keypress);
}

