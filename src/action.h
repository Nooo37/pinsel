#ifndef ACTION_H
#define ACTION_H

#include <gtk/gtk.h>

typedef enum {
    BRUSH,
    ERASER,
    TEXT
} Mode;

typedef struct {
    GList *positions;
    GdkRGBA *color;
    int width;
} BrushAction;

typedef struct {
    GList *positions;
    float alpha;
    int width;
} EraseAction;

typedef struct {
    char *text;
    PangoFontDescription *font;
    GdkRGBA *color;
    int x;
    int y;
} TextAction;

typedef enum {
    // pixbuf related actions
    DISCARD,
    APPLY,
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
    SAVE,
    // ui related actions
    TEXT_INPUT,
    SWITCH_COLORS,
    SET_COLOR1,
    SET_COLOR2,
    SWITCH_MODE,
    QUIT_UNSAFE,
    ZOOM,
    MOVE_HORIZONTALLY,
    MOVE_VERTICALLY,
    FIT_POSITION,
    SAVE_AS,
    OPEN
} ActionType;

typedef struct {
    ActionType type;
    union {
        BrushAction *brush; /* BRUSH_ACTION */
        EraseAction *erase; /* ERASE_ACTION */
        TextAction *text; /* TEXT_ACTION */
        Mode mode; /* SET_MODE */
        float zoom; /* ZOOM */
        int move; /* MOVE_HORIZONTALLY, MOVE_VERTICALLY */
        GdkRGBA color; /* SET_COLOR1, SET_COLOR2 */
    };
} Action;

extern GdkPixbuf* perform_action_brush(BrushAction *action, GdkPixbuf *base);

extern GdkPixbuf* perform_action_text(TextAction *action, GdkPixbuf *base);

extern GdkPixbuf* perform_action_erase(EraseAction *action, GdkPixbuf *base);
#endif
