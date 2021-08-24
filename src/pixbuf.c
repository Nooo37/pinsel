#include <gtk/gtk.h>

#include "action.h"
#include "draw.h"
#include "history.h"

static GdkPixbuf *draw_layer;
static GdkPixbuf *displayed;
static GdkPixbuf *original;
static char* dest = NULL;
static gboolean is_saved = FALSE;
static int width, height;

static void copy_pix_to_displayed()
{
    g_object_unref(displayed);
    displayed = gdk_pixbuf_copy(draw_layer);
}

static gboolean should_save_in_history(Action* action)
{
    if (action->type == BRUSH_ACTION)
        return !action->brush->is_temporary;
    if (action->type == ERASE_ACTION)
        return !action->erase->is_temporary;
    if (action->type == TEXT_ACTION)
        return !action->text->is_temporary;
    return action->type != UNDO && action->type != REDO;
}

static void update_geo()
{
    width = gdk_pixbuf_get_width(draw_layer);
    height = gdk_pixbuf_get_height(draw_layer);
}

extern void pix_init(GdkPixbuf *temp)
{
    displayed = gdk_pixbuf_copy(temp);
    original = gdk_pixbuf_copy(temp);
    draw_layer = gdk_pixbuf_copy(temp);
    update_geo();
    history_init(draw_layer);
}

extern gboolean pix_has_undo()
{
    return history_has_undo();
}

extern gboolean pix_has_redo()
{
    return history_has_redo();
}

extern int pix_get_img_width()
{
    return width;
}

extern int pix_get_img_height()
{
    return height;
}

extern GdkPixbuf* pix_get_original()
{
    return gdk_pixbuf_copy(original);
}

extern GdkPixbuf* pix_get_current()
{
    return gdk_pixbuf_copy(draw_layer);
}

extern GdkPixbuf* pix_get_displayed()
{
    return gdk_pixbuf_copy(displayed);
}

extern void pix_undo_temrporarily_action()
{
    copy_pix_to_displayed();
}

extern char* pix_get_dest()
{
    return dest;
}

extern gboolean pix_is_saved()
{
    return is_saved;
}

extern void pix_set_dest(char* new_dest)
{
    dest = new_dest;
}

extern gboolean pix_save()
{
    // TODO: error handling, get format by the destintation file name
    // char* format = get_format_by_filename(dest);
    gdk_pixbuf_save(pix_get_current(), dest, "png", NULL, NULL);
    is_saved = TRUE;
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

extern void pix_perform_action(Action *action)
{
    switch (action->type) {
        case BRUSH_ACTION: {
                if (action->brush->is_temporary) {
                    g_object_unref(displayed);
                    GdkPixbuf *temp = pix_get_current();
                    displayed = perform_action_brush(action->brush, temp);
                    g_object_unref(temp);
                } else {
                    GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                    g_object_unref(draw_layer);
                    draw_layer = perform_action_brush(action->brush, temp);
                    g_object_unref(temp);
                    copy_pix_to_displayed();
                }
            } break;
        case ERASE_ACTION: { 
                if (action->erase->is_temporary) {
                    g_object_unref(displayed);
                    GdkPixbuf *temp = pix_get_current();
                    displayed = perform_action_erase(action->erase, temp);
                    g_object_unref(temp);
                } else {
                    GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                    g_object_unref(draw_layer);
                    draw_layer = perform_action_erase(action->erase, temp);
                    g_object_unref(temp);
                    copy_pix_to_displayed();
                }
            } break;
        case TEXT_ACTION: {
                if (action->text->is_temporary) {
                    g_object_unref(displayed);
                    GdkPixbuf *temp = pix_get_current();
                    displayed = perform_action_text(action->text, temp);
                    g_object_unref(temp);
                } else {
                    GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                    g_object_unref(draw_layer);
                    draw_layer = perform_action_text(action->text, temp);
                    g_object_unref(temp);
                    copy_pix_to_displayed();
                }
            } break;
        case FLIP_HORIZONTALLY: {
                GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                temp = gdk_pixbuf_copy(draw_layer);
                g_object_unref(draw_layer);
                draw_layer = gdk_pixbuf_flip(temp, TRUE);
                g_object_unref(temp);
                temp = gdk_pixbuf_copy(original);
                g_object_unref(original);
                original = gdk_pixbuf_flip(original, TRUE);
                g_object_unref(temp);
                copy_pix_to_displayed();
            } break;
        case FLIP_VERTICALLY: { 
                GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                temp = gdk_pixbuf_copy(draw_layer);
                g_object_unref(draw_layer);
                draw_layer = gdk_pixbuf_flip(temp, FALSE);
                g_object_unref(temp);
                temp = gdk_pixbuf_copy(original);
                g_object_unref(original);
                original = gdk_pixbuf_flip(original, FALSE);
                g_object_unref(temp);
                copy_pix_to_displayed();
            } break;
        case ROTATE_CLOCKWISE: {
                GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                temp = gdk_pixbuf_copy(draw_layer);
                draw_layer = gdk_pixbuf_rotate_simple(temp, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
                g_object_unref(temp);
                temp = gdk_pixbuf_copy(original);
                original = gdk_pixbuf_rotate_simple(temp, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
                g_object_unref(temp);
                copy_pix_to_displayed();
                update_geo();
            } break;
        case ROTATE_COUNTERCLOCKWISE: {
                GdkPixbuf *temp = gdk_pixbuf_copy(draw_layer);
                temp = gdk_pixbuf_copy(draw_layer);
                draw_layer = gdk_pixbuf_rotate_simple(temp, GDK_PIXBUF_ROTATE_CLOCKWISE);
                g_object_unref(temp);
                temp = gdk_pixbuf_copy(original);
                original = gdk_pixbuf_rotate_simple(temp, GDK_PIXBUF_ROTATE_CLOCKWISE);
                g_object_unref(temp);
                copy_pix_to_displayed();
                update_geo();
            } break;
        case UNDO: {
                g_object_unref(draw_layer);
                draw_layer = history_undo_one();
                copy_pix_to_displayed();
                update_geo();
            } break;
        case REDO: {
                g_object_unref(draw_layer);
                draw_layer = history_redo_one();
                copy_pix_to_displayed();
                update_geo();
            } break;
        case UNDO_ALL: {
                g_object_unref(draw_layer);
                draw_layer = gdk_pixbuf_copy(original);
                copy_pix_to_displayed();
                update_geo();
            } break;
        default:
            return;
    }
    if (should_save_in_history(action)) {
        GdkPixbuf *temp = pix_get_current();
        history_add_one(temp);
        g_object_unref(temp);
        is_saved = FALSE;
    }
}

