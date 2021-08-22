#include <gtk/gtk.h>

#include "pixbuf.h" // shouldn't be here 
#include "utils.h"
#include "draw.h"

cairo_surface_t* strokes_to_surface(GList *positions,
                                    GdkRGBA *color,
                                    int width,
                                    int img_width,
                                    int img_height)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    coord_t *value;
    GList *listrunner;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img_width, img_height);
    cr = cairo_create(surface);

    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_set_line_width(cr, width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    listrunner = g_list_first(positions);
    if (listrunner == NULL) {
        return surface;
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

    g_list_free(listrunner);
    cairo_destroy(cr);

    return surface;
}

GdkPixbuf* draw_line(GdkPixbuf *to_be_drawn_on, 
                     GList *positions, 
                     GdkRGBA *color, 
                     int width)
{
    GdkPixbuf *result;
    cairo_surface_t *surface, *strokes;
    cairo_t *cr;
    int img_height, img_width;

    img_width = gdk_pixbuf_get_width(to_be_drawn_on);
    img_height = gdk_pixbuf_get_height(to_be_drawn_on);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img_width, img_height);
    cr = cairo_create(surface);
    strokes = strokes_to_surface(positions, color, width, img_width, img_height);

    gdk_cairo_set_source_pixbuf(cr, to_be_drawn_on, 0, 0);
    cairo_paint(cr);
    cairo_set_source_surface(cr, strokes, 0, 0);
    cairo_paint(cr);

    result = gdk_pixbuf_get_from_surface(surface, 0, 0, img_width, img_height);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    cairo_surface_destroy(strokes);

    return result;
}

GdkPixbuf* draw_text(GdkPixbuf *to_be_drawn_on, 
                     char *text, 
                     GdkRGBA *color, 
                     PangoFontDescription *font_desc,
                     int x, 
                     int y)
{
    GdkPixbuf *result;
    cairo_surface_t *surface;
    cairo_t *cr;
    int img_width, img_height;
    PangoContext* context = NULL;
    PangoLayout* layout = NULL;
    PangoFontMap* font_map = NULL;


    img_width = gdk_pixbuf_get_width(to_be_drawn_on), 
    img_height = gdk_pixbuf_get_height(to_be_drawn_on);

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                    img_width, img_height);
    cr = cairo_create(surface);

    gdk_cairo_set_source_pixbuf(cr, to_be_drawn_on, 0, 0);
    cairo_paint(cr);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_move_to(cr, x, y);

    font_map = pango_cairo_font_map_new();

    context = pango_font_map_create_context(font_map);

    layout = pango_layout_new(context);

    pango_layout_set_font_description(layout, font_desc);
    pango_font_map_load_font(font_map, context, font_desc);

    pango_layout_set_markup(layout, text, -1);

    pango_cairo_show_layout(cr, layout);

    result = gdk_pixbuf_get_from_surface(surface, 0, 0, img_width, img_height);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return result;
}

extern GdkPixbuf* merge_pixbufs(GdkPixbuf *top_one,
                                GdkPixbuf *bottom_one)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    int img_height, img_width;

    img_width = gdk_pixbuf_get_width(top_one);
    img_height = gdk_pixbuf_get_height(bottom_one);

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                    img_width, img_height);
    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, bottom_one, 0, 0);
    cairo_paint(cr);
    gdk_cairo_set_source_pixbuf(cr, top_one, 0, 0);
    cairo_paint(cr);

    return gdk_pixbuf_get_from_surface(surface, 0, 0, img_width, img_height);
}

extern GdkPixbuf* erase_area(GdkPixbuf *original,
                             cairo_surface_t *mask)
{
    cairo_t *cr;
    cairo_surface_t *surface;
    int img_height, img_width;
    GdkPixbuf *result;

    // assertion: original and current have the same height and width
    img_width = gdk_pixbuf_get_width(original);
    img_height = gdk_pixbuf_get_height(original);

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                    img_width, img_height);
    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, original, 0, 0);
    cairo_paint(cr);
    // TODO: fix that dependency by actually erasing from `original``
    GdkPixbuf *temp = pix_get_original();
    gdk_cairo_set_source_pixbuf(cr, temp, 0, 0);
    cairo_mask_surface(cr, mask, 0, 0);
    cairo_fill(cr);

    result = gdk_pixbuf_get_from_surface(surface, 0, 0, img_width, img_height);

    /* g_object_unref(temp); */
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return result;
}

extern GdkPixbuf* erase_under_line(GdkPixbuf *original,
                                   GList *positions,
                                   int width,
                                   float alpha)
{
    cairo_surface_t *strokes;
    GdkRGBA color;
    int img_height, img_width;
    GdkPixbuf *result;

    color.red = color.green = color.blue = 0;
    color.alpha = alpha;
    img_width = gdk_pixbuf_get_width(original);
    img_height = gdk_pixbuf_get_height(original);
    strokes = strokes_to_surface(positions, &color, width, img_width, img_height);
    result = erase_area(original, strokes);

    cairo_surface_destroy(strokes);

    return result;
}
