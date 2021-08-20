#include <gtk/gtk.h>
#include "config.h"

extern float get_sane_scale(int img_width, int img_height, int area_width, int area_height) 
{
    if ((img_width / img_height) > (area_width / area_height))
        return (area_width / (float) img_width) * (1 - SANE_SCALE_MARGIN);
    else
        return (area_height / (float) img_height) * (1 - SANE_SCALE_MARGIN);
}

extern gboolean is_only_control(GdkEventKey* key)
{
    return (key->state | GDK_SHIFT_MASK) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK);
}

extern gboolean is_only_alt(GdkEventKey* key)
{
    return (key->state | GDK_SHIFT_MASK) == (GDK_MOD1_MASK | GDK_SHIFT_MASK);
}

extern gboolean is_only_alt_control(GdkEventKey* key)
{
    return (key->state | GDK_SHIFT_MASK) == (GDK_MOD1_MASK | GDK_CONTROL_MASK | GDK_SHIFT_MASK);
}

extern gboolean is_no_mod(GdkEventKey* key)
{
    return (key->state | GDK_SHIFT_MASK) == (GDK_SHIFT_MASK);
}
