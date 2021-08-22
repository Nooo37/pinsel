#include <gtk/gtk.h>

#include "action.h"
#include "draw.h"

GdkPixbuf* perform_action_brush(BrushAction *action, GdkPixbuf *base)
{
    return draw_line(base, action->positions, action->color, action->width);
}

GdkPixbuf* perform_action_text(TextAction *action, GdkPixbuf *base)
{
    return draw_text(base, action->text, action->color, action->font, action->x, action->y);
}

GdkPixbuf* perform_action_erase(EraseAction *action, GdkPixbuf *base)
{
    return erase_under_line(base, action->positions, action->width, action->alpha);
}

extern GdkPixbuf* perform_action(Action *action, GdkPixbuf *base)
{
    switch (action->type) {
        case BRUSH_ACTION:
            return perform_action_brush(action->brush, base);
        case ERASE_ACTION:
            return perform_action_erase(action->erase, base);
        case TEXT_ACTION:
            return perform_action_text(action->text, base);
        case FLIP_HORIZONTALLY:
            return gdk_pixbuf_flip(base, TRUE);
        case FLIP_VERTICALLY:
            return gdk_pixbuf_flip(base, FALSE);
        case ROTATE_CLOCKWISE:
            return gdk_pixbuf_rotate_simple(base, GDK_PIXBUF_ROTATE_CLOCKWISE);
        case ROTATE_COUNTERCLOCKWISE:
            return gdk_pixbuf_rotate_simple(base, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
    }
    return NULL;
}
