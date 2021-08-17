#include <stdlib.h>
#include <libgen.h>
#include <gtk/gtk.h>
#include "draw.h"
#include "history.h"

#define TEMP_OUT_FILE "/tmp/pinsel_output.png"
#define TEMP_IN_FILE "/tmp/pinsel_input.png"
/* #define UI_FILE "window.ui" */
#define UI_FILE "/usr/local/lib/pinsel/window.ui"
#define SANE_SCALE_MARGIN 0.03
#define DELTA_MOVE 20
#define DELTA_ZOOM 0.04
#define DELTA_RADIUS 2
#define BUF_SIZE 1024

typedef enum {
    BRUSH,
    ERASER,
    TEXT
} Mode;

typedef enum {
    BRUSHING,
    LINING,
    ERASING,
    TEXTING,
    DRAGGING,
    IDLE
} Activity;

// global state. I hope that is how one does C
Mode mode = BRUSH;
Activity activity = IDLE;
char *dest = NULL;

// Gtk
GdkDisplay *display;
GdkSeat *seat;
GdkDevice *device;

GtkWidget *window;
GtkWidget *canvas;
GtkDialog *text_dialog;
GtkToggleButton *brush_toggle;
GtkToggleButton *eraser_toggle;
GtkToggleButton *text_toggle;
GtkAdjustment *radius_scale;

// pixbufs
GdkPixbuf *pix;
GdkPixbuf *old;
GdkPixbuf *before_action;
int img_width = 0;
int img_height = 0;

// geometry
float scale = 1;
int mid_x = 0;
int mid_y = 0;
int offset_x = 0;
int offset_y = 0;
int area_width;
int area_height;

// brush settings
GdkRGBA color1; // primary color
GdkRGBA color2; // secondary color
int radius = 10;
GList *coords = NULL;
coord_t *start_line = NULL;

// mouse dragging
int dragstart_x = 0;
int dragstart_y = 0;
int offset_old_x = 0;
int offset_old_y = 0;

// text tool
gchar *text = "";
gchar *font = "Sans";
int text_x = 20;
int text_y = 20;
int font_size = 12;

// updates the drawing area based on global state
static gboolean update_drawing_area() 
{
    /* cairo_t *cr; */
    GtkAllocation *alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(canvas, alloc);
    area_width = alloc->width;
    area_height = alloc->height;

    // initalize stuff
    /* cr = gdk_cairo_create(gtk_widget_get_window(canvas)); */
    cairo_region_t* cairoRegion = cairo_region_create();
    GdkWindow* window = gtk_widget_get_window(canvas);
    GdkDrawingContext* drawingContext = gdk_window_begin_draw_frame (window,cairoRegion);
    cairo_t* cr = gdk_drawing_context_get_cairo_context (drawingContext);

    // "clear" background
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

    // update geometry
    mid_x = (area_width - img_width * scale) / 2;
    mid_y = (area_height - img_height * scale) / 2;

    // draw
    cairo_translate(cr, mid_x + offset_x, mid_y + offset_y);
    cairo_scale(cr, scale, scale);
    gdk_cairo_set_source_pixbuf(cr, pix, 0, 0);
    cairo_paint(cr);

    // cleanup
    gdk_window_end_draw_frame(window,drawingContext);
    cairo_region_destroy(cairoRegion);
    g_free(alloc);

    return TRUE;
}

static void set_title_saved(gboolean is_saved)
{
    if (dest == NULL)
        gtk_window_set_title((GtkWindow*) window, "*untitled");
    else if (is_saved)
        gtk_window_set_title((GtkWindow*) window, basename(dest));
    else {
        char *temp = (char*) malloc(100 * sizeof(char));
        sprintf(temp, "*%s", basename(dest));
        gtk_window_set_title((GtkWindow*) window, temp);
        free(temp);
    }
}

static void change()
{
    history_add_one(pix);
    set_title_saved(FALSE);
    update_drawing_area();
}

// connect to that to get painting (and future dragging) abilities
static gint motion_notify_event( GtkWidget *widget,
                                 GdkEventMotion *event )
{
    int x, y, x_translated, y_translated;
    GdkModifierType state;

    // get coords
    if (event->is_hint) {
        gdk_window_get_device_position (event->window, device, &x, &y, &state);
    } else {
        x = event->x;
        y = event->y;
        state = event->state;
    }

    x_translated = (x - offset_x - mid_x) / scale;
    y_translated = (y - offset_y - mid_y) / scale;

    // do a line when holding shift and moving the mouse in brush mode
    if ((state & GDK_BUTTON1_MASK) && (state & GDK_SHIFT_MASK) && mode == BRUSH) {
        if (activity == LINING) {
            coord_t* temp = g_new(coord_t, 1);
            temp->x = x_translated;
            temp->y = y_translated;
            coords = g_list_append(coords, start_line);
            coords = g_list_append(coords, temp);
            pix = draw_line(before_action, coords, &color1, radius);
            update_drawing_area();
            g_free(temp);
            g_list_free(coords);
            coords = NULL;
        } else {
            activity = LINING;
            start_line = g_new(coord_t, 1);
            start_line->x = x_translated;
            start_line->y = y_translated;
            g_clear_object(&before_action);
            before_action = gdk_pixbuf_copy(pix);
        }
        return TRUE;
    } else {
        if (activity == LINING) {
            coord_t* temp = g_new(coord_t, 1);
            temp->x = x_translated;
            temp->y = y_translated;
            coords = g_list_append(coords, start_line);
            coords = g_list_append(coords, temp);
            pix = draw_line(before_action, coords, &color1, radius);
            g_free(temp);
            g_list_free(coords);
            g_free(start_line);
            coords = NULL;
            change();
            activity = IDLE;
            return TRUE;
        }
    }

    // strokes drawing
    if ((state & GDK_BUTTON1_MASK && mode == BRUSH) ||
                    ((state & GDK_BUTTON3_MASK && mode == ERASER))) {
        if (activity == BRUSHING) {
            coord_t* temp = g_new(coord_t, 1);
            temp->x = x_translated;
            temp->y = y_translated;
            coords = g_list_append(coords, temp);
            g_clear_object(&pix);
            pix = draw_line(before_action, coords, &color1, radius);
            update_drawing_area();
        } else {
            activity = BRUSHING;
            g_clear_object(&before_action);
            before_action = gdk_pixbuf_copy(pix);
            update_drawing_area();
        }
    } else {
        if (activity == BRUSHING) {
            pix = draw_line(before_action, coords, &color1, radius);
            g_list_free_full(coords, g_free);
            coords = NULL;
            change();
            activity = IDLE;
        }
    }

    // strokes erase
    if ((state & GDK_BUTTON3_MASK && mode == BRUSH) ||
                    ((state & GDK_BUTTON1_MASK && mode == ERASER))) {
        if (activity == ERASING) {
            coord_t* temp = g_new(coord_t, 1);
            temp->x = x_translated;
            temp->y = y_translated;
            coords = g_list_append(coords, temp);
            g_clear_object(&pix);
            pix = erase_under_line(old, before_action, coords, radius, 1.0);
            update_drawing_area();
        } else {
            activity = ERASING;
            g_clear_object(&before_action);
            before_action = gdk_pixbuf_copy(pix);
            update_drawing_area();
        }
    } else {
        if (activity == ERASING) {
            pix = erase_under_line(old, before_action, coords, radius, 1.0);
            g_list_free_full(coords, g_free);
            coords = NULL;
            change();
            activity = IDLE;
        }
    }

    // image dragging
    if (state & GDK_BUTTON2_MASK) {
        if (activity == DRAGGING) {
            offset_x = offset_old_x - (dragstart_x - x);
            offset_y = offset_old_y - (dragstart_y - y);
            update_drawing_area();
        } else {
            activity = DRAGGING;
            offset_old_x = offset_x;
            offset_old_y = offset_y;
            dragstart_x = x;
            dragstart_y = y;
        }
    } else {
        if (activity == DRAGGING)
            activity = IDLE;
    }

    return TRUE;
}

void temporary_text_display()
{
    g_clear_object(&pix);
    pix = draw_text(before_action, text, &color1, font, font_size, text_x, text_y);
    update_drawing_area();
}

// the function to connect to, to update the drawing area on resize
static gboolean on_draw (GtkWidget *da, 
                         cairo_t *cr, 
                         gpointer data)
{
    update_drawing_area();
    return FALSE;
}

// increases the scale of the image
static gboolean increase_scale()
{
    scale += DELTA_ZOOM;
    update_drawing_area();
    return FALSE;
}

// decreases the scale of the image
static gboolean decrease_scale()
{
    scale -= DELTA_ZOOM;
    update_drawing_area();
    return FALSE;
}

// increases the x offset
static gboolean increase_offset_x()
{
    offset_x += DELTA_MOVE;
    update_drawing_area();
    return FALSE;
}

// decreases the x offset
static gboolean decrease_offset_x()
{
    offset_x -= DELTA_MOVE;
    update_drawing_area();
    return FALSE;
}

// increases the y offset
static gboolean increase_offset_y()
{
    offset_y += DELTA_MOVE;
    update_drawing_area();
    return FALSE;
}

// decreases the y offset
static gboolean decrease_offset_y()
{
    offset_y -= DELTA_MOVE;
    update_drawing_area();
    return FALSE;
}

static void increase_radius()
{
    gtk_adjustment_set_value(radius_scale, radius + DELTA_RADIUS);
}

static void decrease_radius()
{
    gtk_adjustment_set_value(radius_scale, radius - DELTA_RADIUS);
}


static gint button_press_event( GtkWidget      *widget,
                                GdkEventButton *event )
{
    int x, y, x_translated, y_translated;

    // get coords
    x = event->x;
    y = event->y;
    x_translated = (x - offset_x - mid_x) / scale;
    y_translated = (y - offset_y - mid_y) / scale;

    if (mode == TEXT) {
        text_x = x_translated;
        text_y = y_translated;

        if (activity == TEXTING) {
            temporary_text_display();
        } else {
            activity = TEXTING;
            g_clear_object(&before_action);
            before_action = gdk_pixbuf_copy(pix);
            gtk_widget_show((GtkWidget*) text_dialog);
        }
    } // else if (mode == BRUSHING) {
      // TODO: draw dot on click in brush mode
    /*     coord_t* temp = g_new(coord_t, 1); */
    /*     temp->x = x_translated; */
    /*     temp->y = y_translated; */
    /*     coords = g_list_append(coords, temp); */
    /*     g_clear_object(&before_action); */
    /*     before_action = gdk_pixbuf_copy(pix); */
    /*     g_clear_object(&pix); */
    /*     pix = draw_line(before_action, coords, &color1, radius); */
    /*     change(); */
    /*     g_list_free_full(coords, g_free); */
    /*     coords = NULL; */
    /* } */

    return TRUE;
}

// connect to that to get scale on mouse scrolling
static gboolean mouse_scroll( GtkWidget *widget,
                              GdkEventScroll *event,
                              gpointer data) 
{
    if (event->direction == GDK_SCROLL_UP && (event->state & GDK_CONTROL_MASK) && (event->state & GDK_MOD1_MASK))
        increase_radius();
    else if (event->direction == GDK_SCROLL_DOWN && (event->state & GDK_CONTROL_MASK) && (event->state & GDK_MOD1_MASK))
        decrease_radius();
    else if (event->direction == GDK_SCROLL_UP)
        increase_scale();
    else if (event->direction == GDK_SCROLL_DOWN)
        decrease_scale();
    return TRUE;
}

// undo all changes
static void undo_all_changes() 
{
    pix = history_undo_all();
    set_title_saved(FALSE);
    offset_x = 0;
    offset_y = 0;
    update_drawing_area();
}

static void fit_zoom()
{
    offset_x = 0;
    offset_y = 0;
    // TOOD: set scale to get_sane_scale
    update_drawing_area();
}

static void flip_horizontally()
{
    pix = gdk_pixbuf_flip(pix, TRUE);
    change();
}

static void flip_vertically()
{
    pix = gdk_pixbuf_flip(pix, FALSE);
    change();
}

static void rotate_left()
{
    pix = gdk_pixbuf_rotate_simple(pix, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
    change();
}

static void rotate_right()
{
    pix = gdk_pixbuf_rotate_simple(pix, GDK_PIXBUF_ROTATE_CLOCKWISE);
    change();
}

// switch primary and secondary color
static void switch_colors()
{
    GdkRGBA temp;
    temp = color1;
    color1 = color2;
    color2 = temp;
}

static void quit_text()
{
    /* mode = BRUSH; */
    activity = IDLE;
    gtk_toggle_button_set_active(brush_toggle, TRUE);
    gtk_toggle_button_set_active(eraser_toggle, FALSE);
    gtk_toggle_button_set_active(text_toggle, FALSE);
    gtk_widget_hide((GtkWidget*) text_dialog);
}

static void quit_text_tool_ok()
{
    quit_text();
    before_action = pix;
    pix = draw_text(before_action, text, &color1, font, font_size, text_x, text_y);
    change();
}

static void quit_text_tool_cancel()
{
    quit_text();
    pix = before_action;
    update_drawing_area();
}

// update the primary colors button
static void update_color_primary(GtkButton *button, gpointer user_data)
{
    GtkColorChooser *chooser = (GtkColorChooser*) user_data;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(chooser), &color1);
}

// update the secondary colors button
static void update_color_secondary(GtkButton *button, gpointer user_data)
{
    GtkColorChooser *chooser = (GtkColorChooser*) user_data;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(chooser), &color2);
}

// connect to that to get the color button to do its job
static void change_color1(GtkColorButton *color_button)
{
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), &color1);
}

// connect to that to get the color button to do its job
static void change_color2(GtkColorButton *color_button)
{
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), &color2);
}

static void update_on_brush_toggle(GtkToggleButton *brush_toggle, gpointer user_data)
{
    gboolean button_state;
    button_state = gtk_toggle_button_get_active(brush_toggle);

    if (button_state == TRUE) {
        mode = BRUSH;
        gtk_toggle_button_set_active(eraser_toggle, FALSE);
        gtk_toggle_button_set_active(text_toggle, FALSE);
    } else {
        if (mode == BRUSH)
            gtk_toggle_button_set_active(brush_toggle, TRUE);
    }
}

static void update_on_eraser_toggle(GtkToggleButton *eraser_toggle)
{
    gboolean button_state;
    button_state = gtk_toggle_button_get_active(eraser_toggle);

    if (button_state == TRUE) {
        mode = ERASER;
        gtk_toggle_button_set_active(text_toggle, FALSE);
        gtk_toggle_button_set_active(brush_toggle, FALSE);
    } else {
        if (mode == ERASER)
            gtk_toggle_button_set_active(eraser_toggle, TRUE);
    }
}

static void update_on_text_toggle(GtkToggleButton *text_toggle)
{
    gboolean button_state;
    button_state = gtk_toggle_button_get_active(text_toggle);

    if (button_state == TRUE) {
        mode = TEXT;
        gtk_toggle_button_set_active(eraser_toggle, FALSE);
        gtk_toggle_button_set_active(brush_toggle, FALSE);
    } else {
        if (mode == TEXT)
            gtk_toggle_button_set_active(text_toggle, TRUE);
    }
}

static void update_on_text_buffer_change(GtkTextBuffer *textbuffer)
{
    GtkTextIter start, end;

    gtk_text_buffer_get_bounds(textbuffer, &start, &end);
    text = gtk_text_buffer_get_text(textbuffer, &start, &end, TRUE);
    temporary_text_display();
}

static void on_font_set(GtkFontButton* button, gpointer user_data)
{
    const char* font_name = pango_font_family_get_name(gtk_font_chooser_get_font_family((GtkFontChooser*) button));
    /* char* font_face = pango_font_face_get_face_name(gtk_font_chooser_get_font_face((GtkFontChooser*) button)); */
    // TODO: support for font faces
    font_size = gtk_font_chooser_get_font_size((GtkFontChooser*) button) / 1000;
    font = font_name;
    temporary_text_display();
}


// connect to that to get the slider to do its job
static void change_radius(GtkAdjustment *adjust)
{
    radius = gtk_adjustment_get_value(adjust);
}


// get sane scaling default based on the image size at app launch
float get_sane_scale() 
{
    if ((img_width / img_height) > (area_width / area_height))
        return (area_width / (float) img_width) * (1 - SANE_SCALE_MARGIN);
    else
        return (area_height / (float) img_height) * (1 - SANE_SCALE_MARGIN);
}

// writes the stdin stream into a png file to process it further
void write_stdin_to_file()
{
    void *content = malloc(BUF_SIZE);

    FILE *fp = fopen(TEMP_IN_FILE, "w");

    if (fp == 0)
        printf("Error: Couldn't open the temp file\n");

    int read;
    while ((read = fread(content, 1, BUF_SIZE, stdin))) {
        fwrite(content, read, 1, fp);
    }
    if (ferror(stdin))
        printf("Error: Couldn't read from stdin\n");

    fclose(fp);
}

void undo()
{
    pix = history_undo_one();
    set_title_saved(FALSE);
    update_drawing_area();
}

void redo()
{
    pix = history_redo_one();
    set_title_saved(FALSE);
    update_drawing_area();
}

// everything that needs to happen for cleanly quitting the app
void quit()
{
    // the current image to stdout
    if (isatty(1) != 1) { // the first 1 refers to stdout
        remove(TEMP_OUT_FILE);
        gdk_pixbuf_save(pix, TEMP_OUT_FILE, "png", NULL, NULL);
        FILE *a = fopen(TEMP_OUT_FILE, "r");
        int n;
        char s[65536];
        while ((n = fread(s, 1, sizeof(s), a))) {
            fwrite(s, 1, n, stdout);
        }
    }
    // quit gtk
    gtk_main_quit();
}

void save_as()
{
    // TODO: do
}

// saves the image
void save()
{
    if (dest == NULL) {
        save_as();
    } else {
        gdk_pixbuf_save(pix, dest, "png", NULL, NULL);
        set_title_saved(TRUE);
    }
}

// loads the image into the application
int load(char* filename)
{
    GError *err = NULL;

    if (filename == NULL) 
        return 1;
    pix = gdk_pixbuf_new_from_file(filename, &err);
    if (err) {
        g_error_free(err);
        return 1;
    }
    old = gdk_pixbuf_copy(pix);
    img_width = gdk_pixbuf_get_width(pix);
    img_height = gdk_pixbuf_get_height(pix);
    history_init(old);
    return 0;
}

// global keybinds
gboolean my_key_press(GtkWidget *widget,
                      GdkEventKey *event,
                      gpointer user_data) 
{
    if (event->keyval == 'w')
        quit();
    if (event->keyval == 's')
        save();
    if (event->keyval == 'q') {
        save();
        quit();
    }
    if (event->keyval == 'x')
        undo_all_changes();
    if (event->keyval == 'u' || 
                    (event->state & GDK_CONTROL_MASK && event->keyval == 'z'))
        undo();
    if (event->keyval == 'r' || 
                    (event->state & GDK_CONTROL_MASK && event->keyval == 'Z') ||
                    (event->state & GDK_CONTROL_MASK && event->keyval == 'y'))
        redo();
    // movement, zoom
    if (event->keyval == '+')
        increase_scale();
    if (event->keyval == '-')
        decrease_scale();
    if (event->keyval == 'h')
        increase_offset_x();
    if (event->keyval == 'l')
        decrease_offset_x();
    if (event->keyval == 'j')
        decrease_offset_y();
    if (event->keyval == 'k')
        increase_offset_y();
    return FALSE;
}

// build the gtk ui and connects all signals
int build_ui()
{
    GtkBuilder *builder; 
    GtkButton *undo_button, *redo_button, 
              *rotate_left_button, *rotate_right_button, 
              *flip_horizontally_button, *flip_vertically_button,
              *undo_all_button, *fit_zoom_button, *about_button,
              *save_button, *save_as_button, *open_button, *shortcuts_button;
    GtkButton *color_switch_button, *text_dialog_ok, *text_dialog_cancel;
    GtkColorChooser *color_picker_primary;
    GtkColorChooser *color_picker_secondary;
    GtkTextBuffer *textbuffer;
    GtkFontButton *font_button;

    // init devices (for mouse position and clipboard)
    display = gdk_display_get_default();
    seat = gdk_display_get_default_seat(display);
    device = gdk_seat_get_pointer(seat);

    // building and getting all the widgets, connecting signals
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, UI_FILE, NULL);

    // main window and its callbacks
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(G_OBJECT(window), "destroy", 
                    G_CALLBACK(quit), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", 
                    G_CALLBACK(my_key_press), NULL);

    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK
                    | GDK_KEY_RELEASE_MASK);

    // drawing area and its callbacks
    canvas = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));
    g_signal_connect(G_OBJECT(canvas), "draw", 
                    G_CALLBACK(on_draw), NULL);
    g_signal_connect (G_OBJECT(canvas), "motion_notify_event", 
                    G_CALLBACK(motion_notify_event), NULL);
    g_signal_connect (G_OBJECT(canvas), "button_press_event", 
                    G_CALLBACK(button_press_event), NULL);
    g_signal_connect (G_OBJECT(canvas), "scroll-event", 
                    G_CALLBACK(mouse_scroll), NULL);

    gtk_widget_set_events (canvas, GDK_EXPOSURE_MASK
                       | GDK_LEAVE_NOTIFY_MASK
                       | GDK_BUTTON_PRESS_MASK
                       | GDK_POINTER_MOTION_MASK
                       | GDK_POINTER_MOTION_HINT_MASK
                       | GDK_SCROLL_MASK);

    undo_button = GTK_BUTTON(gtk_builder_get_object(builder, "undo_button"));
    g_signal_connect(G_OBJECT(undo_button), "pressed", 
                    G_CALLBACK(undo), NULL);
    redo_button = GTK_BUTTON(gtk_builder_get_object(builder, "redo_button"));
    g_signal_connect(G_OBJECT(redo_button), "pressed", 
                    G_CALLBACK(redo), NULL);

    // color picker
    color_picker_primary = GTK_COLOR_CHOOSER(gtk_builder_get_object(builder, "color_picker_primary"));
    g_signal_connect(G_OBJECT(color_picker_primary), "color-set", 
                    G_CALLBACK(change_color1), NULL);
    color_picker_secondary = GTK_COLOR_CHOOSER(gtk_builder_get_object(builder, "color_picker_secondary"));
    g_signal_connect(G_OBJECT(color_picker_secondary), "color-set", 
                    G_CALLBACK(change_color2), NULL);
    color_switch_button = GTK_BUTTON(gtk_builder_get_object(builder, "color_switch"));
    g_signal_connect(G_OBJECT(color_switch_button), "pressed", 
                    G_CALLBACK(switch_colors), NULL);
    g_signal_connect(G_OBJECT(color_switch_button), "pressed", 
                    G_CALLBACK(update_color_primary), (gpointer) color_picker_primary);
    g_signal_connect(G_OBJECT(color_switch_button), "pressed", 
                    G_CALLBACK(update_color_secondary), (gpointer) color_picker_secondary);

    gtk_color_chooser_get_rgba(color_picker_primary, &color1);
    gtk_color_chooser_get_rgba(color_picker_secondary, &color2);

    // modi toggle buttons

    brush_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "toggle_brush"));
    eraser_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "toggle_eraser"));
    text_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "toggle_text"));

    g_signal_connect(G_OBJECT(brush_toggle), "toggled", 
                    G_CALLBACK(update_on_brush_toggle), NULL);
    g_signal_connect(G_OBJECT(eraser_toggle), "toggled", 
                    G_CALLBACK(update_on_eraser_toggle), NULL);
    g_signal_connect(G_OBJECT(text_toggle), "toggled", 
                    G_CALLBACK(update_on_text_toggle), NULL);

    gtk_toggle_button_set_active(brush_toggle, TRUE);
    gtk_toggle_button_set_active(eraser_toggle, FALSE);
    gtk_toggle_button_set_active(text_toggle, FALSE);

    // text dialog
    text_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "text_dialog"));
    gtk_window_set_keep_above((GtkWindow*) text_dialog, TRUE);
    text_dialog_ok = GTK_BUTTON(gtk_builder_get_object(builder, "text_dialog_ok"));
    g_signal_connect(G_OBJECT(text_dialog_ok), "pressed", 
                    G_CALLBACK(quit_text_tool_ok), NULL);
    text_dialog_cancel = GTK_BUTTON(gtk_builder_get_object(builder, "text_dialog_cancel"));
    g_signal_connect(G_OBJECT(text_dialog_cancel), "pressed", 
                    G_CALLBACK(quit_text_tool_cancel), NULL);
    textbuffer = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "textbuffer"));
    g_signal_connect(G_OBJECT(textbuffer), "changed", 
                    G_CALLBACK(update_on_text_buffer_change), NULL);
    font_button = GTK_FONT_BUTTON(gtk_builder_get_object(builder, "font_picker"));
    g_signal_connect(G_OBJECT(font_button), "font_set", 
                    G_CALLBACK(on_font_set), NULL);

    // scale
    radius_scale = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "radius_scale"));
    g_signal_connect(G_OBJECT(radius_scale), "value-changed", 
                    G_CALLBACK(change_radius), NULL);

    // everything in the popover
    rotate_left_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button"));
    g_signal_connect(G_OBJECT(rotate_left_button), "pressed", 
                    G_CALLBACK(rotate_left), NULL);
    rotate_right_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button"));
    g_signal_connect(G_OBJECT(rotate_right_button), "pressed", 
                    G_CALLBACK(rotate_right), NULL);
    flip_horizontally_button = GTK_BUTTON(gtk_builder_get_object(builder, "flip_horizontally_button"));
    g_signal_connect(G_OBJECT(flip_horizontally_button), "pressed", 
                    G_CALLBACK(flip_horizontally), NULL);
    flip_vertically_button = GTK_BUTTON(gtk_builder_get_object(builder, "flip_vertically_button"));
    g_signal_connect(G_OBJECT(flip_vertically_button), "pressed", 
                    G_CALLBACK(flip_vertically), NULL);
    undo_all_button = GTK_BUTTON(gtk_builder_get_object(builder, "undo_all_button"));
    g_signal_connect(G_OBJECT(undo_all_button), "pressed", 
                    G_CALLBACK(undo_all_changes), NULL);
    fit_zoom_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_zoom_button"));
    g_signal_connect(G_OBJECT(fit_zoom_button), "pressed", 
                    G_CALLBACK(fit_zoom), NULL);
    save_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
    g_signal_connect(G_OBJECT(save_button), "pressed", 
                    G_CALLBACK(save), NULL);
    save_as_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_as_button"));
    g_signal_connect(G_OBJECT(save_as_button), "pressed", 
                    G_CALLBACK(save_as), NULL);
    // TODO: add action to the buttons
    open_button = GTK_BUTTON(gtk_builder_get_object(builder, "open_button"));
    shortcuts_button = GTK_BUTTON(gtk_builder_get_object(builder, "shortcuts_button"));
    about_button = GTK_BUTTON(gtk_builder_get_object(builder, "about_button"));
    // start
    gtk_widget_show(window);                
    set_title_saved(FALSE);

    update_drawing_area();
    scale = get_sane_scale();
    return 0;
}

// main
int main(int argc, char *argv[])
{
    char *image_to_edit = NULL; // path to the image

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            i++;
            if (i >= argc) {
                printf("Missing argument for -o\n");
                return 1;
            } else {
                dest = argv[i];
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            // TODO: help text
            printf("PR a help text please\n");
            return 1;
        }
    }

    // initalize the image

    if (argc > 1) { // is the image a command line argument?
        image_to_edit = argv[1];
    }

    if (isatty(0) != 1) { // 0 refers to stdin, is the image coming in a pipe?
        write_stdin_to_file();
        image_to_edit = TEMP_IN_FILE;
    } 

    // TODO: show something when it's started without a file as arg or stdin
    load(image_to_edit);


    gtk_init(&argc, &argv);

    int t = build_ui();

    gtk_main();

    return 0 | t;
}

