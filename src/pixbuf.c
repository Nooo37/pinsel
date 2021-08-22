#include <gtk/gtk.h>

#include "action.h"
#include "history.h"

static GdkPixbuf *current;
static GdkPixbuf *original;
static GdkPixbuf *displayed;
static char* dest = NULL;

static void copy_pix_to_displayed()
{
    g_object_unref(displayed);
    displayed = gdk_pixbuf_copy(current);
}

extern void pix_init(GdkPixbuf *temp)
{
    current = gdk_pixbuf_copy(temp);
    original = gdk_pixbuf_copy(temp);
    displayed = gdk_pixbuf_copy(temp);
    history_init(current);
}

extern GdkPixbuf* pix_get_original()
{
    return gdk_pixbuf_copy(original);
}

extern GdkPixbuf* pix_get_current()
{
    return gdk_pixbuf_copy(current);
}

extern GdkPixbuf* pix_get_displayed()
{
    return gdk_pixbuf_copy(displayed);
}

extern GdkPixbuf* pix_undo()
{
    g_object_unref(current);
    current = history_undo_one();
    copy_pix_to_displayed();
    return gdk_pixbuf_copy(current);
}

extern GdkPixbuf* pix_redo()
{
    g_object_unref(current);
    current = history_redo_one();
    copy_pix_to_displayed();
    return gdk_pixbuf_copy(current);
}

extern GdkPixbuf* pix_undo_all()
{
    g_object_unref(current);
    current = history_undo_all();
    copy_pix_to_displayed();
    return gdk_pixbuf_copy(current);
}

extern void pix_add_action_temporarily(Action *action)
{
    g_object_unref(displayed);
    displayed = perform_action(action, current);
}

extern void pix_add_action_permanently(Action *action)
{
    GdkPixbuf *temp = gdk_pixbuf_copy(current);
    g_object_unref(current);
    current = perform_action(action, temp);
    history_add_one(current);
    g_object_unref(temp);
    copy_pix_to_displayed();
}

extern void pix_undo_temrporarily_action()
{
    copy_pix_to_displayed();
}

extern char* pix_get_dest()
{
    return dest;
}

extern void pix_set_dest(char* new_dest)
{
    dest = new_dest;
}

extern gboolean pix_save()
{
    // TODO: error handling, get format by the destintation file name
    gdk_pixbuf_save(current, dest, "png", NULL, NULL);
    return TRUE;
}

extern void pix_load_new_image(char* filename)
{
    GError *err = NULL;
    if (filename == NULL) 
        return;
    GdkPixbuf *temp = gdk_pixbuf_new_from_file(filename, &err);
    if (err) {
        g_error_free(err);
        return;
    }
    history_free();
    pix_init(temp);
    pix_set_dest(filename);
    g_object_unref(temp);
}


