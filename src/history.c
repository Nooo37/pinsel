#include <gtk/gtk.h>

#include "history.h"

static GdkPixbuf *original;
static GdkPixbuf *current;
static GQueue *undo_history;
static GQueue *redo_history;
static int history_limit;

void history_add_one_to_undo(GdkPixbuf *layer)
{
    g_queue_push_head(undo_history, (gpointer) gdk_pixbuf_copy(layer));
    if (g_queue_get_length(undo_history) > history_limit) {
        GdkPixbuf *temp = (GdkPixbuf*) g_queue_pop_tail(undo_history);
        g_object_unref(temp);
    }
}

void history_add_one_to_redo(GdkPixbuf *layer)
{
    g_queue_push_head(redo_history, (gpointer) gdk_pixbuf_copy(layer));
    if (g_queue_get_length(redo_history) > history_limit) {
        GdkPixbuf *temp = (GdkPixbuf*) g_queue_pop_tail(redo_history);
        g_object_unref(temp);
    }
}

extern void history_init(GdkPixbuf *layer, int limit)
{
    if (undo_history != NULL)
        g_queue_clear_full(undo_history, g_object_unref);
    if (redo_history != NULL)
        g_queue_clear_full(redo_history, g_object_unref);
    original = gdk_pixbuf_copy(layer);
    current = gdk_pixbuf_copy(layer);
    undo_history = g_queue_new();
    redo_history = g_queue_new();
    history_limit = limit;
}

extern void history_add_one(GdkPixbuf *layer)
{
    g_queue_clear_full(redo_history, g_object_unref);
    history_add_one_to_undo(current);
    /* g_object_unref(current); */
    current = gdk_pixbuf_copy(layer);
}

extern GdkPixbuf* history_redo_one()
{
    if (g_queue_is_empty(redo_history))
        return gdk_pixbuf_copy(current);
    history_add_one_to_undo(current);
    g_object_unref(current);
    current = (GdkPixbuf*) g_queue_pop_head(redo_history);
    return gdk_pixbuf_copy(current);
}

extern GdkPixbuf* history_undo_one()
{
    if (g_queue_is_empty(undo_history))
        return gdk_pixbuf_copy(current);
    history_add_one_to_redo(current);
    g_object_unref(current);
    current = (GdkPixbuf*) g_queue_pop_head(undo_history);
    return gdk_pixbuf_copy(current);
}

extern GdkPixbuf* history_undo_all()
{
    g_queue_clear_full(undo_history, g_object_unref);
    g_queue_clear_full(redo_history, g_object_unref);
    history_add_one(original);
    return gdk_pixbuf_copy(original);
}

extern void history_free()
{
    g_queue_clear_full(undo_history, g_object_unref);
    g_queue_clear_full(redo_history, g_object_unref);
    g_object_unref(original);
    g_object_unref(current);
}

extern gboolean history_has_undo()
{
    return !g_queue_is_empty(undo_history);
}

extern gboolean history_has_redo()
{
    return !g_queue_is_empty(redo_history);
}
