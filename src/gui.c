#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <libgen.h>
#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#include "cairo.h"
#include "gui.h"
#include "config.h"
#include "ui_state.h"
#include "utils.h"

static GdkDisplay *display;
static GdkSeat *seat;
static GdkDevice *device;

static GtkWidget *window;
static GtkWidget *canvas;
static GtkDialog *text_dialog;
static GtkButton *undo_button, *redo_button;
static GtkToggleButton *brush_toggle;
static GtkToggleButton *eraser_toggle;
static GtkToggleButton *text_toggle;
static GtkColorChooser *color_picker_primary;
static GtkColorChooser *color_picker_secondary;
static GtkAdjustment *radius_scale;

// updates the drawing area based on global state
static void update_drawing_area() 
{
    UIGeometry *geo = ui_get_geo();
    geo->area_height = gtk_widget_get_allocated_height(canvas);
    geo->area_width = gtk_widget_get_allocated_width(canvas);
    geo->img_height = pix_get_img_height();
    geo->img_width = pix_get_img_width();
    geo->mid_x = (geo->area_width - geo->img_width * geo->scale) / 2;
    geo->mid_y = (geo->area_height - geo->img_height * geo->scale) / 2;

    // initalize stuff
    cairo_region_t* cairo_region = cairo_region_create();
    GdkWindow* window = gtk_widget_get_window(canvas);
    GdkDrawingContext* drawing_context = gdk_window_begin_draw_frame(window,cairo_region);
    cairo_t* cr = gdk_drawing_context_get_cairo_context(drawing_context);

    // make background black -> overpaint the last area
    if (getenv("WAYLAND_DISPLAY")) {
        gtk_widget_queue_draw(canvas);
    } else {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_fill(cr);
        cairo_paint(cr);
    }

    // draw
    cairo_translate(cr, geo->mid_x + geo->offset_x, geo->mid_y + geo->offset_y);
    cairo_scale(cr, geo->scale, geo->scale);
    GdkPixbuf* temp = pix_get_displayed();
    gdk_cairo_set_source_pixbuf(cr, temp, 0, 0);
    cairo_paint(cr);

    // cleanup
    gdk_window_end_draw_frame(window, drawing_context);
    cairo_region_destroy(cairo_region);

    gtk_widget_set_sensitive(GTK_WIDGET(undo_button), pix_has_undo());
    gtk_widget_set_sensitive(GTK_WIDGET(redo_button), pix_has_redo());
}

static void set_title_saved(gboolean is_saved)
{
    if (pix_get_dest() == NULL)
        gtk_window_set_title((GtkWindow*) window, "*untitled");
    else if (is_saved)
        gtk_window_set_title((GtkWindow*) window, basename(pix_get_dest()));
    else {
        char *temp = g_strdup_printf("*%s", basename(pix_get_dest()));
        gtk_window_set_title((GtkWindow*) window, temp);
        g_free(temp);
    }
}

static Modifiers convert_gdk_to_modifiers(GdkModifierType event)
{
    Modifiers mods;
    mods.shift = event & GDK_SHIFT_MASK;
    mods.control = event & GDK_CONTROL_MASK;
    mods.alt = event & GDK_MOD1_MASK;
    mods.button1 = event & GDK_BUTTON1_MASK;
    mods.button2 = event & GDK_BUTTON2_MASK;
    mods.button3 = event & GDK_BUTTON3_MASK;
    return mods;
}

static gboolean on_draw(GtkWidget *da, 
                        cairo_t *cr, 
                        gpointer data)
{
    update_drawing_area();
    return FALSE;
}

static void redraw_popup(GtkWidget *temp, gpointer popup)
{
    gtk_widget_queue_draw(GTK_WIDGET(popup));
}

static void fullscreen(GtkWidget *temp, gpointer window)
{
    gtk_window_fullscreen(GTK_WINDOW(window));
}

static void fit_zoom()
{
    config_perform_self_contained_action(FIT_POSITION);
    update_drawing_area();
}

static void undo_all_changes() 
{
    config_perform_self_contained_action(UNDO_ALL);
    fit_zoom();
    update_drawing_area();
}

static void flip_horizontally()
{
    config_perform_self_contained_action(FLIP_HORIZONTALLY);
    update_drawing_area();
}

static void flip_vertically()
{
    config_perform_self_contained_action(FLIP_VERTICALLY);
    update_drawing_area();
}

static void rotate_left()
{
    config_perform_self_contained_action(ROTATE_COUNTERCLOCKWISE);
    update_drawing_area();
}

static void rotate_right()
{
    config_perform_self_contained_action(ROTATE_CLOCKWISE);
    update_drawing_area();
}

static void update_color_primary(GtkButton *button, gpointer user_data)
{
    GtkColorChooser *chooser = (GtkColorChooser*) user_data;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(chooser), ui_get_color1());
}

static void update_color_secondary(GtkButton *button, gpointer user_data)
{
    GtkColorChooser *chooser = (GtkColorChooser*) user_data;
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(chooser), ui_get_color2());
}

static void change_color1(GtkColorButton *color_button)
{
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), ui_get_color1());
}

static void change_color2(GtkColorButton *color_button)
{
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), ui_get_color1());
}

static void update_on_brush_toggle(GtkToggleButton *brush_toggle, gpointer user_data)
{
    gboolean button_state;
    button_state = gtk_toggle_button_get_active(brush_toggle);

    if (button_state == TRUE) {
        ui_set_mode(BRUSH);
        gtk_toggle_button_set_active(eraser_toggle, FALSE);
        gtk_toggle_button_set_active(text_toggle, FALSE);
    } else {
        if (ui_get_mode() == BRUSH)
            gtk_toggle_button_set_active(brush_toggle, TRUE);
    }
}

static void update_on_eraser_toggle(GtkToggleButton *eraser_toggle)
{
    gboolean button_state;
    button_state = gtk_toggle_button_get_active(eraser_toggle);

    if (button_state == TRUE) {
        ui_set_mode(ERASER);
        gtk_toggle_button_set_active(text_toggle, FALSE);
        gtk_toggle_button_set_active(brush_toggle, FALSE);
    } else {
        if (ui_get_mode() == ERASER)
            gtk_toggle_button_set_active(eraser_toggle, TRUE);
    }
}

static void update_on_text_toggle(GtkToggleButton *text_toggle)
{
    gboolean button_state;
    button_state = gtk_toggle_button_get_active(text_toggle);

    if (button_state == TRUE) {
        ui_set_mode(TEXT);
        gtk_toggle_button_set_active(eraser_toggle, FALSE);
        gtk_toggle_button_set_active(brush_toggle, FALSE);
    } else {
        if (ui_get_mode() == TEXT)
            gtk_toggle_button_set_active(text_toggle, TRUE);
    }
}


static void undo()
{
    config_perform_self_contained_action(UNDO);
    update_drawing_area();
}

static void redo()
{
    config_perform_self_contained_action(REDO);
    update_drawing_area();
}

extern void gui_save_as()
{
    GtkWidget *dialog;
     
    dialog = gtk_file_chooser_dialog_new ("Save File",
                                          GTK_WINDOW(window),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          "gtk-cancel", GTK_RESPONSE_CANCEL,
                                          "gtk-save", GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
    if (pix_get_dest() != NULL)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), pix_get_dest());
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        pix_set_dest(temp);
        pix_save();
    }
    
    gtk_widget_destroy (dialog);
}

extern void gui_save()
{
    if (pix_get_dest() == NULL)
        gui_save_as();
    else
        pix_save();
}

extern void gui_open_new_image()
{
    GtkWidget *dialog;
     
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          GTK_WINDOW(window),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          "gtk-cancel", GTK_RESPONSE_CANCEL,
                                          "gtk-save", GTK_RESPONSE_ACCEPT,
                                          NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        pix_load_new_image(filename);
        update_drawing_area();
        g_free(filename);
    }
    
    gtk_widget_destroy (dialog);
}

extern void gui_open_text_dialog()
{
    gtk_widget_show(GTK_WIDGET(text_dialog));
    gtk_window_set_keep_above(GTK_WINDOW(text_dialog), TRUE);
}


static void open_new_image(GtkWidget *temp, GtkPopover *popover)
{
    gui_open_new_image();
    redraw_popup(NULL, popover);
}

static void open_about_dialog(GtkWidget *temp, gpointer about_dialog)
{
    if (gtk_dialog_run(GTK_DIALOG(about_dialog)))
        gtk_widget_hide(GTK_WIDGET(about_dialog));
}

static void open_shortcuts_dialog(GtkWidget *temp, gpointer shortcuts_dialog)
{
    GtkBuilder *builder;
    GtkWidget *overlay;

    gchar* ui = config_get_shortcut_ui();
    builder = gtk_builder_new_from_string(ui, -1);
    overlay = GTK_WIDGET(gtk_builder_get_object(builder, "shortcuts_dialog"));
    gtk_window_set_transient_for(GTK_WINDOW(overlay), GTK_WINDOW(window));
    g_object_set(overlay, "view-name", NULL, NULL);
    gtk_widget_show(overlay);
    g_object_unref(builder);
}

static void update_color_buttons()
{
    gtk_color_chooser_set_rgba(color_picker_primary, ui_get_color1());
    gtk_color_chooser_set_rgba(color_picker_secondary, ui_get_color2());
}

static void update_toggle_buttons()
{
    Mode mode = ui_get_mode();
    gtk_toggle_button_set_active(text_toggle, mode == TEXT);
    gtk_toggle_button_set_active(eraser_toggle, mode == ERASER);
    gtk_toggle_button_set_active(brush_toggle, mode == BRUSH);
}

static void quit_text_tool_ok()
{
    gtk_widget_hide((GtkWidget*) text_dialog);
    config_notify_text_close(TRUE);
    update_drawing_area();
    update_toggle_buttons();
    update_color_buttons();
    set_title_saved(pix_is_saved());
}

static void quit_text_tool_cancel()
{
    gtk_widget_hide((GtkWidget*) text_dialog);
    config_notify_text_close(FALSE);
    update_drawing_area();
    update_toggle_buttons();
    update_color_buttons();
    set_title_saved(pix_is_saved());
}

extern void gui_update()
{
    update_drawing_area();
    update_toggle_buttons();
    update_color_buttons();
    set_title_saved(pix_is_saved());
}

static void update_on_text_buffer_change(GtkTextBuffer *textbuffer)
{
    GtkTextIter start, end;

    gtk_text_buffer_get_bounds(textbuffer, &start, &end);
    ui_set_text(gtk_text_buffer_get_text(textbuffer, &start, &end, TRUE));
    gui_update();
}

static void on_font_set(GtkFontButton* button, gpointer user_data)
{
    ui_set_font(gtk_font_chooser_get_font_desc((GtkFontChooser*) button));
    config_notify_text(ui_get_text()); // temporary hack to update the drawing area on font change
    gui_update();
}

static void change_radius(GtkAdjustment *adjust)
{
    ui_set_width(gtk_adjustment_get_value(adjust));
    gui_update();
}

static gboolean on_click(GtkWidget      *widget,
                         GdkEventButton *event)
{
    int x, y;

    x = event->x;
    y = event->y;

    Modifiers mod = convert_gdk_to_modifiers(event->state);
    config_perform_click_event(event->button, x, y, mod);
    gui_update();
    return TRUE;
}

static gboolean on_motion(GtkWidget *widget,
                          GdkEventMotion *event)
{
    int x, y;
    GdkModifierType state;

    // get coords
    if (event->is_hint) {
        gdk_window_get_device_position (event->window, device, &x, &y, &state);
    } else {
        x = event->x;
        y = event->y;
        state = event->state;
    }

    Modifiers mod = convert_gdk_to_modifiers(state);
    config_perform_motion_event(x, y, mod);

    update_drawing_area();
    return TRUE;
}

static gboolean on_scroll(GtkWidget *widget,
                          GdkEventScroll *event,
                          gpointer data)
{
    Modifiers mod = convert_gdk_to_modifiers(event->state);
    if (event->direction == GDK_SCROLL_UP)
        config_perform_key_event("scroll_up", mod);
    else if (event->direction == GDK_SCROLL_DOWN)
        config_perform_key_event("scroll_down", mod);
    gui_update();
    return TRUE;
}

static gboolean on_key(GtkWidget *widget,
                       GdkEventKey *event,
                       gpointer user_data)
{
    char key[0];
    key[0] = tolower((char) event->keyval);
    Modifiers mod = convert_gdk_to_modifiers(event->state);
    config_perform_key_event(key, mod);
    gui_update();
    return TRUE;
}

// build the gtk ui and connects all signals
extern int gui_init(gboolean is_on_top, 
                    gboolean is_maximized)
{
    GtkBuilder *builder; 
    GtkButton *fullscreen_button,
              *rotate_left_button, *rotate_right_button, 
              *flip_horizontally_button, *flip_vertically_button,
              *undo_all_button, *fit_zoom_button, *about_button,
              *save_button, *save_as_button, *open_button, *shortcuts_button;
    GtkButton *color_switch_button, *text_dialog_ok, *text_dialog_cancel;
    GtkTextBuffer *textbuffer;
    GtkFontButton *font_button;
    GtkWidget *popover, *about_dialog, *shortcuts_dialog;
    gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);

    // init devices (for mouse position and clipboard)
    display = gdk_display_get_default();
    seat = gdk_display_get_default_seat(display);
    device = gdk_seat_get_pointer(seat);

    // building and getting all the widgets, connecting signals
    builder = gtk_builder_new_from_resource("/data/window.ui");

    // main window and its callbacks
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(G_OBJECT(window), "destroy", 
                    G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", 
                    G_CALLBACK(on_key), NULL);

    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK
                    | GDK_KEY_RELEASE_MASK);

    // drawing area and its callbacks
    canvas = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));
    g_signal_connect(G_OBJECT(canvas), "draw", 
                    G_CALLBACK(on_draw), NULL);
    g_signal_connect (G_OBJECT(canvas), "motion_notify_event", 
                    G_CALLBACK(on_motion), NULL);
    g_signal_connect (G_OBJECT(canvas), "button_press_event", 
                    G_CALLBACK(on_click), NULL);
    g_signal_connect (G_OBJECT(canvas), "scroll-event", 
                    G_CALLBACK(on_scroll), NULL);

    gtk_widget_set_events (canvas, GDK_EXPOSURE_MASK
                       | GDK_LEAVE_NOTIFY_MASK
                       | GDK_BUTTON_PRESS_MASK
                       | GDK_KEY_PRESS_MASK
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
                    G_CALLBACK(ui_switch_colors), NULL);
    g_signal_connect(G_OBJECT(color_switch_button), "pressed", 
                    G_CALLBACK(update_color_primary), (gpointer) color_picker_primary);
    g_signal_connect(G_OBJECT(color_switch_button), "pressed", 
                    G_CALLBACK(update_color_secondary), (gpointer) color_picker_secondary);


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

    // text dialog
    text_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "text_dialog"));
    g_signal_connect(G_OBJECT(text_dialog), "close", 
                    G_CALLBACK(quit_text_tool_cancel), NULL);
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
    popover = GTK_WIDGET(gtk_builder_get_object(builder, "popover1"));
    rotate_left_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_left_button"));
    g_signal_connect(G_OBJECT(rotate_left_button), "pressed", 
                    G_CALLBACK(rotate_left), NULL);
    g_signal_connect(G_OBJECT(rotate_left_button), "pressed", 
                    G_CALLBACK(redraw_popup), (gpointer) popover);
    rotate_right_button = GTK_BUTTON(gtk_builder_get_object(builder, "rotate_right_button"));
    g_signal_connect(G_OBJECT(rotate_right_button), "pressed", 
                    G_CALLBACK(rotate_right), NULL);
    g_signal_connect(G_OBJECT(rotate_right_button), "pressed", 
                    G_CALLBACK(redraw_popup), (gpointer) popover);
    flip_horizontally_button = GTK_BUTTON(gtk_builder_get_object(builder, "flip_horizontally_button"));
    g_signal_connect(G_OBJECT(flip_horizontally_button), "pressed", 
                    G_CALLBACK(flip_horizontally), NULL);
    g_signal_connect(G_OBJECT(flip_horizontally_button), "pressed", 
                    G_CALLBACK(redraw_popup), (gpointer) popover);
    flip_vertically_button = GTK_BUTTON(gtk_builder_get_object(builder, "flip_vertically_button"));
    g_signal_connect(G_OBJECT(flip_vertically_button), "pressed", 
                    G_CALLBACK(flip_vertically), NULL);
    g_signal_connect(G_OBJECT(flip_vertically_button), "pressed", 
                    G_CALLBACK(redraw_popup), (gpointer) popover);
    undo_all_button = GTK_BUTTON(gtk_builder_get_object(builder, "undo_all_button"));
    g_signal_connect(G_OBJECT(undo_all_button), "pressed", 
                    G_CALLBACK(undo_all_changes), NULL);
    g_signal_connect(G_OBJECT(undo_all_button), "pressed", 
                    G_CALLBACK(redraw_popup), (gpointer) popover);
    fit_zoom_button = GTK_BUTTON(gtk_builder_get_object(builder, "fit_zoom_button"));
    g_signal_connect(G_OBJECT(fit_zoom_button), "pressed", 
                    G_CALLBACK(fit_zoom), NULL);
    g_signal_connect(G_OBJECT(fit_zoom_button), "pressed", 
                    G_CALLBACK(redraw_popup), (gpointer) popover);
    save_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_button"));
    g_signal_connect(G_OBJECT(save_button), "pressed", 
                    G_CALLBACK(gui_save), NULL);
    save_as_button = GTK_BUTTON(gtk_builder_get_object(builder, "save_as_button"));
    g_signal_connect(G_OBJECT(save_as_button), "pressed", 
                    G_CALLBACK(gui_save_as), NULL);
    fullscreen_button = GTK_BUTTON(gtk_builder_get_object(builder, "fullscreen_button"));
    g_signal_connect(G_OBJECT(fullscreen_button), "pressed", 
                    G_CALLBACK(fullscreen), (gpointer) window);
    open_button = GTK_BUTTON(gtk_builder_get_object(builder, "open_button"));
    g_signal_connect(G_OBJECT(open_button), "pressed", 
                    G_CALLBACK(open_new_image), (gpointer) popover);
    shortcuts_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "shortcuts_dialog"));
    shortcuts_button = GTK_BUTTON(gtk_builder_get_object(builder, "shortcuts_button"));
    g_signal_connect(G_OBJECT(shortcuts_button), "pressed", 
                    G_CALLBACK(open_shortcuts_dialog), (gpointer) shortcuts_dialog);
    about_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "about_dialog"));
    about_button = GTK_BUTTON(gtk_builder_get_object(builder, "about_button"));
    g_signal_connect(G_OBJECT(about_button), "pressed", 
                    G_CALLBACK(open_about_dialog), (gpointer) about_dialog);

    // start
    gtk_window_set_keep_above(GTK_WINDOW(window), is_on_top);
    if (is_maximized)
        gtk_window_maximize(GTK_WINDOW(window));
    gtk_widget_show(window);                
    set_title_saved(FALSE);
    ui_state_init();
    update_drawing_area();
    update_color_buttons();
    update_toggle_buttons();
    fit_zoom();
    ui_set_font(gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(font_button)));

    return 1;
}
