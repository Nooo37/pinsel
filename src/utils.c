#include <gtk/gtk.h>
#include <stdlib.h>

#include "pinsel.h"

extern float get_sane_scale(int img_width, int img_height, int area_width, int area_height) 
{
    if ((img_width / img_height) > (area_width / area_height))
        return (area_width / (float) img_width) * (1 - SANE_SCALE_MARGIN);
    else
        return (area_height / (float) img_height) * (1 - SANE_SCALE_MARGIN);
}

extern gboolean is_no_mod(GdkEventKey* key)
{
    return (key->state | GDK_SHIFT_MASK) == (GDK_SHIFT_MASK);
}

extern gboolean is_valid_output_format(const char *format)
{
    GSList *listrunner = gdk_pixbuf_get_formats();
    while (listrunner != NULL) {
        GdkPixbufFormat *pix_format = (GdkPixbufFormat*) (listrunner->data);
        gchar *name = gdk_pixbuf_format_get_name(pix_format);
        if (strcmp(name, format) == 0)
            return TRUE;
        listrunner = g_slist_next(listrunner);
    }
    return FALSE;
}

extern void print_help()
{
    GInputStream* help_stream = g_resources_open_stream("/data/help.txt", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    int buffer_size = 10000;
    char buffer[buffer_size];
    gsize count;
    g_input_stream_read_all (help_stream, buffer, buffer_size, &count, NULL,  NULL);
    printf("%s", buffer);
}
