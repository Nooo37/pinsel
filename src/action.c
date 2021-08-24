#include <gtk/gtk.h>

#include "action.h"
#include "draw.h"

extern GdkPixbuf* perform_action_brush(BrushAction *action, GdkPixbuf *base)
{
    return draw_line(base, action->positions, action->color, action->width);
}

extern GdkPixbuf* perform_action_text(TextAction *action, GdkPixbuf *base)
{
    return draw_text(base, action->text, action->color, action->font, action->x, action->y);
}

extern GdkPixbuf* perform_action_erase(EraseAction *action, GdkPixbuf *base)
{
    return erase_under_line(base, action->positions, action->width, action->alpha);
}

