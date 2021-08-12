#include <gtk/gtk.h>

/*
 * The functions in the "draw" file are all pure and handle
 * the dirty work of drawing on a pixbuf
 */


/*
 * Types
 */ 

typedef struct
{
    int x;
    int y;
} coord_t;

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
                     char *font,
                     int font_size,
                     int x, 
                     int y);