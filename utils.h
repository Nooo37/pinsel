#include <gtk/gtk.h>


/* 
 * Utils are functions that only depend on constants and thus can be placed 
 * outside the big UI file
 */

/* Gets a sane scale factor based on the provided arguments */
extern float get_sane_scale(int img_width, int img_height, int area_width, int area_height);

/* Writes the given Pixbuf/Image to stdout (through TEMP_OUT_FILE) */
extern gboolean write_pixbuf_to_stdout(GdkPixbuf* pixbuf);
