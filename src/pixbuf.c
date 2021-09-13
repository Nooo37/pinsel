#include <gtk/gtk.h>

#include "action.h"
#include "history.h"

static GdkPixbuf *draw_layer;
static GdkPixbuf *displayed;
static GdkPixbuf *original;
static char* dest = NULL;
static gboolean is_saved = FALSE;
static int width, height, history_limit;

static void copy_pix_to_displayed()
{
    g_object_unref(displayed);
    displayed = gdk_pixbuf_copy(draw_layer);
}

static void copy_displayed_to_pix()
{
    g_object_unref(draw_layer);
    draw_layer = gdk_pixbuf_copy(displayed);
}

static void update_geo()
{
    width = gdk_pixbuf_get_width(draw_layer);
    height = gdk_pixbuf_get_height(draw_layer);
}

extern void pix_init(GdkPixbuf *temp, int limit)
{
    displayed = gdk_pixbuf_copy(temp);
    original = gdk_pixbuf_copy(temp);
    draw_layer = gdk_pixbuf_copy(temp);
    history_limit = limit;
    update_geo();
    history_init(draw_layer, limit);
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
    return displayed;
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
    // TODO: , get format by the destintation file name
    // char* format = get_format_by_filename(dest);
    if (dest != NULL) {
        GError *err = NULL;
        gdk_pixbuf_save(pix_get_current(), dest, "png", &err, NULL);
        if (err) {
            fprintf(stderr, "Failed to save image:\n%s\n", err->message);
            g_error_free(err);
            return FALSE;
        } else {
            return TRUE;
        }
    }
    return FALSE;
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
    pix_init(temp, history_limit);
    pix_set_dest(filename);
    g_object_unref(temp);
}

extern void pix_perform_action(Action *action)
{
    /* early return means: do not put the current state into history */
    switch (action->type) {
        case APPLY: {
                copy_displayed_to_pix();
                update_geo();
            } break;
        case DISCARD: {
                copy_pix_to_displayed();
                return;
            } break;
        case BRUSH_ACTION: {
                GdkPixbuf *temp = gdk_pixbuf_copy(displayed);
                g_object_unref(displayed);
                displayed = perform_action_brush(action->brush, temp);
                g_object_unref(temp);
                return;
            } break;
        case ERASE_ACTION: { 
                GdkPixbuf *temp = gdk_pixbuf_copy(displayed);
                g_object_unref(displayed);
                displayed = perform_action_erase(action->erase, temp);
                g_object_unref(temp);
                return;
            } break;
        case TEXT_ACTION: {
                GdkPixbuf *temp = gdk_pixbuf_copy(displayed);
                g_object_unref(displayed);
                displayed = perform_action_text(action->text, temp);
                g_object_unref(temp);
                return;
            } break;
        case CROP_ACTION: {
                CropAction *old = action->crop;
                CropAction crop;
                // sanitize the incomming crop geometry a bit
                // (shouldn't error out with negative/out-of-bounds sizes)
                crop.x = old->x;
                crop.y = old->y;
                crop.width = old->width;
                crop.height = old->height;
                if (crop.width < 0) {
                    crop.x += crop.width;
                    crop.width *= -1;
                }
                if (crop.height < 0) {
                    crop.y += crop.height;
                    crop.height *= -1;
                }
                if (crop.x < 0) {
                    crop.width += crop.x;
                    crop.x = 0;
                }
                if (crop.y < 0) {
                    crop.height += crop.y;
                    crop.y = 0;
                }
                if (crop.width + crop.x > width) {
                    crop.width = width - crop.x;
                }
                if (crop.height + crop.y > height) {
                    crop.height = height - crop.y;
                }
                GdkPixbuf *temp = gdk_pixbuf_copy(displayed);
                g_object_unref(displayed);
                displayed = perform_action_crop(&crop, temp);
                g_object_unref(temp);
                return;
            } break;
        case FLIP_HORIZONTALLY: {
                GdkPixbuf *temp;
                temp = gdk_pixbuf_copy(draw_layer);
                g_object_unref(draw_layer);
                draw_layer = gdk_pixbuf_flip(temp, TRUE);
                g_object_unref(temp);
                temp = gdk_pixbuf_copy(original);
                g_object_unref(original);
                original = gdk_pixbuf_flip(temp, TRUE);
                g_object_unref(temp);
                copy_pix_to_displayed();
            } break;
        case FLIP_VERTICALLY: { 
                GdkPixbuf *temp;
                temp = gdk_pixbuf_copy(draw_layer);
                g_object_unref(draw_layer);
                draw_layer = gdk_pixbuf_flip(temp, FALSE);
                g_object_unref(temp);
                temp = gdk_pixbuf_copy(original);
                g_object_unref(original);
                original = gdk_pixbuf_flip(temp, FALSE);
                g_object_unref(temp);
                copy_pix_to_displayed();
            } break;
        case ROTATE_CLOCKWISE: {
                GdkPixbuf *temp;
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
                GdkPixbuf *temp;
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
                return;
            } break;
        case REDO: {
                g_object_unref(draw_layer);
                draw_layer = history_redo_one();
                copy_pix_to_displayed();
                update_geo();
                return;
            } break;
        case UNDO_ALL: {
                g_object_unref(draw_layer);
                draw_layer = gdk_pixbuf_copy(original);
                copy_pix_to_displayed();
                update_geo();
            } break;
        case SAVE: {
                is_saved = pix_save();
                return;
            } break;
        default:
            return;
    }
    history_add_one(draw_layer);
    is_saved = FALSE;
}

