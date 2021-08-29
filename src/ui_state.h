#ifndef UI_STATE_H
#define UI_STATE_H

#include <gtk/gtk.h>

#include "action.h"
#include "pixbuf.h"

extern void ui_state_init();

extern UIGeometry* ui_get_geo();

/* extern int ui_translate_x(int x); */

/* extern int ui_translate_y(int y); */

extern void ui_perform_action(Action* action);

extern Mode ui_get_mode();

extern void ui_set_mode(Mode mode);

extern GdkRGBA* ui_get_color1();

extern GdkRGBA* ui_get_color2();

extern void ui_switch_colors();

extern void ui_set_width(int new_width);

extern int ui_get_width();

extern PangoFontDescription* ui_get_font();

extern void ui_set_font(PangoFontDescription *new_font);

extern void ui_set_text(char* new_text);

extern char* ui_get_text();

#endif
