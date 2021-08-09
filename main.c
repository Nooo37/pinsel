#include <stdbool.h>
#include <gtk/gtk.h>

static int DELTA_MOVE = 20;
static float DELTA_ZOOM = 0.04;

// global state. I hope that is how one does C
char dest[] = "";

// Gtk Elements
GtkWidget *canvas;

// pixbufs
GdkPixbuf *pix;
GdkPixbuf *old;
int img_width = 0;
int img_height = 0;

// geometry
float scale = 0.1;
int mid_x = 0;
int mid_y = 0;
int offset_x = 0;
int offset_y = 0;

// brush settings
GdkRGBA color;
int radius = 10;

// mouse dragging
bool is_dragging = false;
int dragstart_x = 0;
int dragstart_y = 0;
int offset_old_x = 0;
int offset_old_y = 0;

// adds a dot at position (x, y) using global brush settings
static void add_dot(int x, int y) 
{
    cairo_surface_t *surface;
    cairo_t *cr;
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                    img_width, img_height);
    cr = cairo_create(surface);

    cairo_move_to(cr, 0, 0);
    gdk_cairo_set_source_pixbuf(cr, pix, 0, 0);
    cairo_paint(cr);
    cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    cairo_arc(cr, x, y, radius, 0, 2 * G_PI);
    cairo_fill(cr);
    surface = cairo_get_target(cr);
    pix = gdk_pixbuf_get_from_surface(surface, 0, 0, 
                    cairo_image_surface_get_width(surface), 
                    cairo_image_surface_get_height(surface));
}

// updates the drawing area based on global state
static gboolean update_drawing_area() 
{
    /* cairo_t *cr; */
    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(canvas, alloc);
    int area_width = alloc->width;
    int area_height = alloc->height;

    // initalize stuff
    /* cr = gdk_cairo_create(gtk_widget_get_window(canvas)); */
    cairo_region_t * cairoRegion = cairo_region_create();
    GdkDrawingContext * drawingContext;
    GdkWindow* window = gtk_widget_get_window(canvas);
    drawingContext = gdk_window_begin_draw_frame (window,cairoRegion);
    cairo_t* cr = gdk_drawing_context_get_cairo_context (drawingContext);

    // "clear" background
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
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

// maybe I'll need that later for detecting a mouse button release for dragging
static gint button_press_event( GtkWidget      *widget,
                                GdkEventButton *event )
{
    return TRUE;
}

// connect to that to get scale on mouse scrolling
static gboolean mouse_scroll( GtkWidget *widget,
                              GdkEvent *event,
                              gpointer data) 
{
    double delta_x, delta_y;
    if (gdk_event_get_scroll_deltas(event, &delta_x, &delta_y)) {
        if (delta_y < 0) {
            decrease_scale();
        }
        if (delta_y > 0) {
            increase_scale();
        }
        return TRUE;
    }
    return FALSE;
}

// connect to that to get painting (and future dragging) abilities
static gint motion_notify_event( GtkWidget *widget,
                                 GdkEventMotion *event )
{
    int x, y, x_translated, y_translated;
    GdkModifierType state;

    // get coords
    if (event->is_hint) {
        gdk_window_get_pointer (event->window, &x, &y, &state);

    } else {
        x = event->x;
        y = event->y;
        state = event->state;
    }
      
    // strokes
    if (state & GDK_BUTTON1_MASK) {
        x_translated = (x - offset_x - mid_x) / scale;
        y_translated = (y - offset_y - mid_y) / scale;
        // TODO: interpolate between current point and previous one to get a 
        // straight line even with few signal triggerings
        add_dot(x_translated, y_translated);
        update_drawing_area();
    }
    // image dragging
    if (state & GDK_BUTTON2_MASK) {
        if (is_dragging) {
            offset_x = offset_old_x - (dragstart_x - x);
            offset_y = offset_old_y - (dragstart_y - y);
            update_drawing_area();
        } else {
            is_dragging = true;
            offset_old_x = offset_x;
            offset_old_y = offset_y;
            dragstart_x = x;
            dragstart_y = y;
        }
    } else {
        is_dragging = false;
    }

  return TRUE;
}

// undo all changes
static void undo_all_changes() 
{
    pix = old;
    offset_x = 0;
    offset_y = 0;
    update_drawing_area();
}

// connect to that to get the color button to do its job
static void change_color(GtkColorButton *color_button)
{
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), &color);
}

// connect to that to get the slider to do its job
static void change_radius(GtkRange *range)
{
    radius = gtk_range_get_value(range);
}

// get sane scaling default based on the image size at app launch
float get_sane_scale() 
{
    GtkAllocation* alloc = g_new(GtkAllocation, 1);
    gtk_widget_get_allocation(canvas, alloc);
    int area_width = alloc->width;
    int area_height = alloc->height;
    g_free(alloc);
    if ((area_width / area_height) > (img_width / img_height)) {
        // the area has a wider aspect ratio than the image
        return (area_height / img_height) ;
    } else {
        // the image has a wider aspect raio than the area
        return (area_width / img_width) ;
    }
}

// global keybinds
gboolean my_key_press(GtkWidget *widget,
                      GdkEventKey *event,
                      gpointer user_data) 
{
    if (event->state & GDK_CONTROL_MASK && event->keyval == 'w')
        gtk_main_quit();
    if (event->keyval == 'u')
        increase_scale();
    if (event->keyval == 'i')
        decrease_scale();
    if (event->keyval == 'h')
        increase_offset_x();
    if (event->keyval == 'l')
        decrease_offset_x();
    if (event->keyval == 'j')
        decrease_offset_y();
    if (event->keyval == 'k')
        increase_offset_y();
    if (event->keyval == 'x')
        undo_all_changes();
    if (event->keyval == 'q') {
        // TODO: the save destination shoule have been the second argument
        gdk_pixbuf_save(pix, "file.png", "png", NULL, NULL);
        gtk_main_quit();
    }
    if (event->keyval == 'c') {
        GdkDisplay *display = gdk_display_get_default();
        GtkClipboard *clipboard = gtk_clipboard_get_for_display(display,
                        GDK_SELECTION_CLIPBOARD);
        gtk_clipboard_set_image(clipboard, pix);
        // the application has to remain open long enough to
        // copy the image, closing it right after here won't work
    }
    return FALSE;
}

// main
int main(int argc, char *argv[])
{
    GError *err = NULL;
    GtkBuilder *builder; 
    GtkWidget *window;
    GtkButton *undo_button;
    GtkColorButton *color_picker;
    GtkScale *radius_scale;

    // init image related

    pix = gdk_pixbuf_new_from_file(argv[1], &err);
    if(err) {
        printf("Error : %s\n", err->message);
        g_error_free(err);
        return FALSE;
    }
    old = gdk_pixbuf_copy(pix);
    img_width = gdk_pixbuf_get_width(pix);
    img_height = gdk_pixbuf_get_height(pix);

    // EPIC WIDGETS

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "window.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(G_OBJECT(window), "destroy", 
                    G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", 
                    G_CALLBACK(my_key_press), NULL);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);

    canvas = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));
    g_signal_connect(G_OBJECT(canvas), "draw", G_CALLBACK(on_draw), NULL);

    g_signal_connect (G_OBJECT(canvas), "motion_notify_event", 
                    G_CALLBACK(motion_notify_event), NULL);
    g_signal_connect (G_OBJECT(canvas), "button_press_event", 
                    G_CALLBACK(button_press_event), NULL);
    g_signal_connect(G_OBJECT(window), "scroll-event", 
                    G_CALLBACK(mouse_scroll), NULL);

    gtk_widget_set_events (canvas, GDK_EXPOSURE_MASK
                       | GDK_LEAVE_NOTIFY_MASK
                       | GDK_BUTTON_PRESS_MASK
                       | GDK_POINTER_MOTION_MASK
                       | GDK_POINTER_MOTION_HINT_MASK
                       | GDK_SCROLL_MASK);

    undo_button = GTK_BUTTON(gtk_builder_get_object(builder, "undo_button"));
    g_signal_connect(G_OBJECT(undo_button), "pressed", 
                    G_CALLBACK(undo_all_changes), NULL);

    // color picker
    color_picker = GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "color_picker"));
    g_signal_connect(G_OBJECT(color_picker), "color-set", 
                    G_CALLBACK(change_color), NULL);

    color.alpha = 1; // rest is already at 0 for black
    gtk_color_chooser_set_rgba(color_picker, &color);

    // slider
    radius_scale = GTK_WIDGET(gtk_builder_get_object(builder, "radius_scale"));
    g_signal_connect(G_OBJECT(radius_scale), "value-changed", 
                    G_CALLBACK(change_radius), NULL);
    gtk_range_set_value(radius_scale, radius);

    // start
    gtk_widget_show(window);                
    scale = get_sane_scale();
    gtk_main();

    return 0;
}

