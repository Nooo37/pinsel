#include <gtk/gtk.h>

#include "config.h"
#include "ui_state.h"
#include "gui.h"
#include "utils.h"

static UIGeometry geo;
static Mode mode;
static GdkRGBA color1; // primary color
static GdkRGBA color2; // secondary color
static int width;
static PangoFontDescription *font_desc;
static char* text;

extern void ui_state_init()
{
    geo.scale = 1;
    geo.area_height = 0;
    geo.area_width = 0;
    geo.offset_x = 0;
    geo.offset_y = 0;
    mode = BRUSH;
}

extern UIGeometry* ui_get_geo()
{
    return &geo;
}

/* extern int ui_translate_x(int x) */
/* { */
/*     return (x - geo.offset_x - geo.mid_x) / geo.scale; */
/* } */
/*  */
/* extern int ui_translate_y(int y) */
/* { */
/*     return (y - geo.offset_y - geo.mid_y) / geo.scale; */
/* } */

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

extern void ui_set_width(int new_width)
{
    width = new_width;
}

extern int ui_get_width()
{
    return width;
}

extern void ui_set_text(char* new_text)
{
    config_notify_text(new_text);
    text = new_text;
}

extern char* ui_get_text()
{
    return text;
}

extern PangoFontDescription* ui_get_font()
{
    return font_desc;
}

extern void ui_set_font(PangoFontDescription *new_font)
{
    font_desc = new_font;
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
                            geo.area_width, geo.area_height);
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
        case TEXT_INPUT: {
            gui_open_text_dialog();
        } break;
        case SET_GEO: {
            geo = action->geo;
        } break;
        default:
            return;
    }
}
