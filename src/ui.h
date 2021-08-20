#include <gtk/gtk.h>

extern int build_ui(GdkPixbuf *init_pix,
                    char *init_dest,
                    gboolean is_on_top, 
                    gboolean is_maximized);

extern GdkPixbuf* get_pixbuf();
