#ifndef DRAW_H
#define DRAW_H 

#include <gtk/gtk.h>

/*
 * The functions in the "draw" file are all pure and handle
 * the dirty work of drawing on a pixbuf
 */

/* types */

typedef struct {
    GdkPixbuf* content;
    gboolean is_visible;
} Layer;

/*
 * Merges two pixbufs 
 *
 * top_one: The pixbufs that gets drawn on the top
 * bottom_one: The pixbuf that gets drawn on the bottom
 */
extern GdkPixbuf* merge_pixbufs(GdkPixbuf *top_one,
                                GdkPixbuf *bottom_one);

/*
 * Draws a line on the given pixbuf and returns the result
 *
 * to_be_drawn_on: The pixbuf on which the line will be drawn
 * positions: The line chrarcterized by a list of coordinates (coord_t)
 * color: The color of the line
 * width: The width of the line
 *
 */
extern GdkPixbuf* draw_line(GdkPixbuf *to_be_drawn_on, 
                            GList *positions, 
                            GdkRGBA *color, 
                            int width);

/*
 * Draws text on the given pixbuf and returns the result
 *
 * to_be_drawn_on: The pixbuf on which the line will be drawn
 * text: The texet to draw
 * color: The color of the text
 * font: The font of the text
 * font_size: The font size of the text
 * x: The x coordinate of the text on the given pixbuf
 * y: The y coordinate of the text on the given pixbuf
 *
 */
extern GdkPixbuf* draw_text(GdkPixbuf *to_be_drawn_on, 
                            char *text, 
                            GdkRGBA *color, 
                            PangoFontDescription *font_desc,
                            int x, 
                            int y);

/*
 * Erases everything from "current" that is in "mask" by replacing
 * it with "original"
 *
 * original: The pixbuf below the one that is currently erased on
 * current: The pixbuf on which this function erases
 * mask: The area on which this function erases
 */
extern GdkPixbuf* erase_area(GdkPixbuf *original,
                             cairo_surface_t *mask);

/*
 * Erases everything from "current" that is under a line by replacing
 * it with "original"
 *
 * original: The pixbuf below the one that is currently erased on
 * current: The pixbuf on which this function erases
 * positions: The line chrarcterized by a list of coordinates (coord_t)
 * width: The width of the line
 * alpha: The alpha value of the line
 */
extern GdkPixbuf* erase_under_line(GdkPixbuf *original,
                                   GList *positions,
                                   int width,
                                   float alpha);

/*
 * Crops the given image to the given width and height starting at the 
 * given coordinates
 *
 * original: THe pixbuf that should be cropped
 * x: y: The coordinates of the upper left crop-rectangle
 * width: height: The width and the height of the crop-rectangle
 */
extern GdkPixbuf* draw_crop(GdkPixbuf *original, 
                            int x,
                            int y,
                            int width,
                            int height);

#endif
