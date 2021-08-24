#include <gtk/gtk.h>

#include "pixbuf.h"

typedef struct {
    float scale;
    int mid_x;
    int mid_y;
    int offset_x;
    int offset_y;
    int area_width;
    int area_height;
} UIGeometry;

extern void ui_state_init();

extern void ui_set_area_width(int area_width);

extern void ui_set_area_height(int area_height);

extern int ui_get_mid_x();

extern int ui_get_mid_y();

extern int ui_get_offset_x();

extern int ui_get_offset_y();

extern void ui_set_offset_x(int x);

extern void ui_set_offset_y(int y);

extern float ui_get_scale();

extern void ui_set_scale(float scale);

extern int ui_translate_x(int x);

extern int ui_translate_y(int y);

extern int ui_get_area_height();

extern int ui_get_area_width();

extern void ui_perform_action(Action* action);
