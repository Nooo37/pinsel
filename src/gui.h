#ifndef GUI_H
#define GUI_H 

#include <gtk/gtk.h>

extern int gui_init(gboolean is_on_top, 
                    gboolean is_maximized);

extern void update_drawing_area();

extern void gui_save_as();

extern void gui_save();

extern void gui_open_new_image();

extern void gui_open_text_dialog();

#endif
