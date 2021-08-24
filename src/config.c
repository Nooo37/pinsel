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
        case FIT_POSITION: case QUIT_UNSAFE: case SAVE_AS: case OPEN:
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

static int config_flip_horizontally(lua_State *L)
{
    Action temp;
    temp.type = FLIP_HORIZONTALLY;
    config_perform_action(&temp);
    return 1;
}

static int config_flip_vertically(lua_State *L)
{
    Action temp;
    temp.type = FLIP_VERTICALLY;
    config_perform_action(&temp);
    return 1;
}

static int config_rotate_clockwise(lua_State *L)
{
    Action temp;
    temp.type = ROTATE_CLOCKWISE;
    config_perform_action(&temp);
    return 1;
}

static int config_rotate_counterclockwise(lua_State *L)
{
    Action temp;
    temp.type = ROTATE_COUNTERCLOCKWISE;
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

    static const luaL_Reg l[] = {
        { "zoom", config_zoom },
        { "rotate_counterclockwise", config_rotate_counterclockwise },
        { "rotate_clockwise", config_rotate_clockwise },
        { "flip_horizontally", config_flip_horizontally },
        { "flip_vertically", config_flip_vertically },
        { "move_horizontally", config_move_horizontally },
        { "move_vertically", config_move_vertically },
        { "undo_all", config_undo_all },
        { "undo", config_undo },
        { "redo", config_redo },
        { "save", config_save },
        { "save_as", config_save_as },
        { "open", config_open },
        { "quit", config_quit_unsafe },
        { NULL, NULL }
    };

    luaL_newlib(L, l);
    lua_setglobal(L, "_pinsel");

    lua_pushstring(L, "C-");
    lua_setglobal(L, "CONTROL");

    lua_pushstring(L, "M-");
    lua_setglobal(L, "ALT");

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
    // TODO: Some adjustment needed for non-letter keys
    char *keystring = g_strdup_printf("%c", ek->keyval);
    if (is_no_mod(ek))
        keypress = g_strdup_printf("%s", keystring);
    else if (is_only_control(ek))
        keypress = g_strdup_printf("C-%s", keystring);
    else if (is_only_alt(ek))
        keypress = g_strdup_printf("M-%s", keystring);
    else if (is_only_alt_control(ek))
        keypress = g_strdup_printf("C-M-%s", keystring);
    perform_lua_keybind(keypress);
}

