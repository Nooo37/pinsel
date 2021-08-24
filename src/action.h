#ifndef ACTION_H
#define ACTION_H
#include <gtk/gtk.h>

typedef struct {
    GList *positions;
    GdkRGBA *color;
    int width;
    gboolean is_temporary;
} BrushAction;

typedef struct {
    GList *positions;
    float alpha;
    int width;
    gboolean is_temporary;
} EraseAction;

typedef struct {
    char *text;
    PangoFontDescription *font;
    GdkRGBA *color;
    int x;
    int y;
    gboolean is_temporary;
} TextAction;

typedef enum {
    // pixbuf related actions
    BRUSH_ACTION,
    ERASE_ACTION,
    TEXT_ACTION,
    FLIP_HORIZONTALLY,
    FLIP_VERTICALLY,
    ROTATE_CLOCKWISE,
    ROTATE_COUNTERCLOCKWISE,
    UNDO,
    REDO,
    UNDO_ALL,
    // ui related actions
    QUIT_UNSAFE,
    ZOOM,
    MOVE_HORIZONTALLY,
    MOVE_VERTICALLY,
    FIT_POSITION,
} ActionType;

typedef struct {
    ActionType type;
    union {
        BrushAction *brush; /* BRUSH_ACTION */
        EraseAction *erase; /* ERASE_ACTION */
        TextAction *text; /* TEXT_ACTION */
        gpointer *nothing; // NULL pointer in case the type is self containing
        float zoom; /* ZOOM */
        int move; /* MOVE_HORIZONTALLY, MOVE_VERTICALLY */

    };
} Action;

extern GdkPixbuf* perform_action_brush(BrushAction *action, GdkPixbuf *base);

extern GdkPixbuf* perform_action_text(TextAction *action, GdkPixbuf *base);

extern GdkPixbuf* perform_action_erase(EraseAction *action, GdkPixbuf *base);
#endif
