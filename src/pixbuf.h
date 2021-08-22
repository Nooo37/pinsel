#include <gtk/gtk.h>

#include "action.h"

extern void pix_init(GdkPixbuf *temp);

extern GdkPixbuf* pix_get_original();

extern GdkPixbuf* pix_get_current();

extern GdkPixbuf* pix_get_displayed();

extern GdkPixbuf* pix_undo();

extern GdkPixbuf* pix_redo();

extern GdkPixbuf* pix_undo_all();

extern void pix_add_action_temporarily(Action *action);

extern void pix_add_action_permanently(Action *action);

extern void pix_undo_temrporarily_action();

extern char* pix_get_dest();

extern void pix_set_dest(char* new_dest);

extern gboolean pix_save();

extern void pix_load_new_image(char* filename);

