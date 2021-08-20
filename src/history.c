#include <gtk/gtk.h>

#include "history.h"

static GdkPixbuf *original;
static GdkPixbuf *current;
static GQueue *undo_history;
static GQueue *redo_history;

extern void history_init(GdkPixbuf *layer)
{
    original = gdk_pixbuf_copy(layer);
    current = layer;
    undo_history = g_queue_new();
    redo_history = g_queue_new();
    history_add_one(current);
}

extern void history_add_one(GdkPixbuf *layer)
{
    g_queue_clear_full(redo_history, g_object_unref);
    g_queue_push_head(undo_history, current);
    current = gdk_pixbuf_copy(layer);
}

extern GdkPixbuf* history_redo_one()
{
    if (g_queue_is_empty(redo_history))
        return current;
    g_queue_push_head(undo_history, (gpointer) current);
    current = (GdkPixbuf*) g_queue_pop_head(redo_history);
    return current;
}

extern GdkPixbuf* history_undo_one()
{
    if (g_queue_is_empty(undo_history))
        return current;
    g_queue_push_head(redo_history, (gpointer) current);
    current = (GdkPixbuf*) g_queue_pop_head(undo_history);
    return current;
}

extern GdkPixbuf* history_undo_all()
{
    g_queue_clear_full(undo_history, g_object_unref);
    g_queue_clear_full(redo_history, g_object_unref);
    history_add_one(original);
    return gdk_pixbuf_copy(original);
}
