#include <gtk/gtk.h>

/*
 * Initalizes the history
 *
 * layer: The starting layer
 */ 
extern void history_init(GdkPixbuf* layer);

/*
 * Adds one entry to the history
 *
 * layer: The new pixbuf layer that gets added to the history
 */
extern void history_add_one(GdkPixbuf* layer);


/*
 * Performs one redo action
 *
 * return: The resulting "new" pixbuf
 */
extern GdkPixbuf* history_redo_one();

/*
 * Performs one undo action
 *
 * return: The resulting "new" pixbuf
 */
extern GdkPixbuf* history_undo_one();


/*
 * Performs all undo actions
 *
 * return: The resulting "new" pixbuf
 */
extern GdkPixbuf* history_undo_all();
