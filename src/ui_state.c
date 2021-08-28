#include <gtk/gtk.h>

#include "pango/pango-font.h"
#include "ui_state.h"
#include "gui.h"
#include "pinsel.h"
#include "utils.h"

static UIGeometry geo;
static Mode mode;
static int width = 10;
PangoFontDescription *font;
GdkRGBA color1; // primary color
GdkRGBA color2; // secondary color

extern void ui_state_init()
{
    geo.scale = 1;
    geo.area_height = 0;
    geo.area_width = 0;
    geo.mid_x = 0;
    geo.mid_y = 0;
    geo.offset_x = 0;
    geo.offset_y = 0;
    mode = BRUSH;
}

extern void ui_set_area_width(int area_width)
{
    geo.area_width = area_width;
    geo.mid_x = (geo.area_width - pix_get_img_width() * geo.scale) / 2;
}

extern void ui_set_area_height(int area_height)
{
    geo.area_height = area_height;
    geo.mid_y = (geo.area_height - pix_get_img_height() * geo.scale) / 2;
}

extern int ui_get_mid_x()
{
    return geo.mid_x;
}

extern int ui_get_mid_y()
{
    return geo.mid_y;
}

extern int ui_get_offset_x()
{
    return geo.offset_x;
}

extern int ui_get_offset_y()
{
    return geo.offset_y;
}

extern void ui_set_offset_x(int x)
{
    geo.offset_x = x;
}

extern void ui_set_offset_y(int y)
{
    geo.offset_y = y;
}

extern float ui_get_scale()
{
    return geo.scale;
}


extern void ui_set_scale(float scale)
{
    geo.scale = scale;
}

extern int ui_translate_x(int x)
{
    return (x - geo.offset_x - geo.mid_x) / geo.scale;
}

extern int ui_translate_y(int y)
{
    return (y - geo.offset_y - geo.mid_y) / geo.scale;
}

extern int ui_get_area_height()
{
    return geo.area_height;
}

extern int ui_get_area_width()
{
    return geo.area_width;
}

extern Mode ui_get_mode()
{
    return mode;
}

extern void ui_set_mode(Mode new_mode)
{
    mode = new_mode;
}

extern GdkRGBA* ui_get_color1()
{
    return &color1;
}

extern GdkRGBA* ui_get_color2()
{
    return &color2;
}

extern void ui_switch_colors()
{
    GdkRGBA temp;
    temp = color1;
    color1 = color2;
    color2 = temp;
}

extern void ui_set_width(int temp)
{
    width = temp;
}

extern int ui_get_width()
{
    return width;
}

extern PangoFontDescription* ui_get_font()
{
    return font;
}

extern void ui_set_font(PangoFontDescription *new_font)
{
    font = new_font;
}

extern void ui_perform_action(Action* action)
{
    switch(action->type) {
        case ZOOM: {
            geo.scale += action->zoom;
        } break;
        case MOVE_HORIZONTALLY: {
            geo.offset_x += action->move;
        } break;
        case MOVE_VERTICALLY: {
            geo.offset_y += action->move;
        } break;
        case FIT_POSITION: {
            geo.offset_x = 0;
            geo.offset_y = 0;
            geo.scale = get_sane_scale(pix_get_img_width(), pix_get_img_height(),
                            ui_get_area_width(), ui_get_area_height());
        } break;
        case QUIT_UNSAFE: {
            gtk_main_quit();
        } break;
        case SAVE_AS: {
            gui_save_as();
        } break;
        case OPEN: {
            gui_open_new_image();
        } break;
        case SWITCH_MODE: {
            ui_set_mode(action->mode);
        } break;
        case SWITCH_COLORS: {
            ui_switch_colors();
        } break;
        case SET_COLOR1: {
            color1 = action->color;
        } break;
        case SET_COLOR2: {
            color2 = action->color;
        } break;
        default:
            return;
    }
}
