#include <gtk/gtk.h>

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
    BRUSH_ACTION,
    ERASE_ACTION,
    TEXT_ACTION,
    FLIP_HORIZONTALLY,
    FLIP_VERTICALLY,
    ROTATE_CLOCKWISE,
    ROTATE_COUNTERCLOCKWISE,
} ActionTypes;

typedef struct {
    ActionTypes type;
    union {
        BrushAction *brush;
        EraseAction *erase;
        TextAction *text;
        gpointer *nothing; // NULL pointer in case the type is self containing
    };
} Action;

extern GdkPixbuf* perform_action(Action *action, GdkPixbuf *base);

