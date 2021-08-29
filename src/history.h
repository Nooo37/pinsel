#include <gtk/gtk.h>

extern void history_init(GdkPixbuf* layer, int limit);

extern void history_add_one(GdkPixbuf* layer);

extern GdkPixbuf* history_redo_one();

extern GdkPixbuf* history_undo_one();

extern GdkPixbuf* history_undo_all();

extern void history_free();

extern gboolean history_has_undo();

extern gboolean history_has_redo();

