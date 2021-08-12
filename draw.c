#include <gtk/gtk.h>
#include "draw.h"

GdkPixbuf* draw_line(GdkPixbuf *to_be_drawn_on, 
                     GList *positions, 
                     GdkRGBA *color, 
                     int width)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    coord_t *value;
    GList *listrunner;
    int img_height, img_width;

    img_width = gdk_pixbuf_get_width(to_be_drawn_on);
    img_height = gdk_pixbuf_get_height(to_be_drawn_on);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                    gdk_pixbuf_get_width(to_be_drawn_on), 
                    gdk_pixbuf_get_height(to_be_drawn_on));
    cr = cairo_create(surface);

    gdk_cairo_set_source_pixbuf(cr, to_be_drawn_on, 0, 0);
    cairo_paint(cr);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_set_line_width(cr, width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    listrunner = g_list_first(positions);
    if (listrunner == NULL) {
        return to_be_drawn_on;
    }

    // start at the first entry
    value = listrunner->data;
    cairo_move_to(cr, value->x, value->y);

    // make lines from each entry to the next entry
    while (listrunner != NULL) {
        value = listrunner->data;
        cairo_line_to(cr, value->x, value->y);
        listrunner = g_list_next(listrunner);
    }

    cairo_stroke(cr);

    return gdk_pixbuf_get_from_surface(surface, 0, 0, img_width, img_height);
}

GdkPixbuf* draw_text(GdkPixbuf *to_be_drawn_on, 
                     char *text, 
                     GdkRGBA *color, 
                     char *font,
                     int font_size,
                     int x, 
                     int y)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    int img_width, img_height;

    img_width = gdk_pixbuf_get_width(to_be_drawn_on), 
    img_height = gdk_pixbuf_get_height(to_be_drawn_on);

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                    img_width, img_height);
    cr = cairo_create(surface);

    gdk_cairo_set_source_pixbuf(cr, to_be_drawn_on, 0, 0);
    cairo_paint(cr);
    cairo_select_font_face(cr, font, 
                    CAIRO_FONT_SLANT_NORMAL, 
                    CAIRO_FONT_WEIGHT_BOLD);

    cairo_set_font_size(cr, font_size);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);  

    return gdk_pixbuf_get_from_surface(surface, 0, 0, img_width, img_height);
}

