#include <gtk/gtk.h>
#include <gio/gunixoutputstream.h>
#include <gio/gunixinputstream.h>
#include <stdlib.h>

#include "gio/gmenumodel.h"
#include "pinsel.h"
#include "config.h"
#include "pixbuf.h"
#include "utils.h"
#include "gui.h"

int main(int argc, char *argv[])
{
    gboolean is_on_top = FALSE, is_maximized = FALSE;
    char *output_format = "png";
    gchar *init_dest = NULL;
    GdkPixbuf *init_pix = NULL;

    // read image from stdin if something is being piped into the app
    if (isatty(0) != 1) {
        GError *err = NULL;
        GInputStream *stdin_stream = g_unix_input_stream_new(0, FALSE);

        init_pix = gdk_pixbuf_new_from_stream(stdin_stream, NULL, &err);
        if (err || !GDK_IS_PIXBUF(init_pix)) {
            fprintf(stderr, "Failed to load image from stdin\n%s\n", err->message);
            return 1;
        }
    }

    char* config_file_location = "init.lua";
    /* if (getenv("XDG_CONFIG_HOME")) */
    /*     config_file_location = getenv("XDG_CONFIG_HOME"); */
    /* else if (getenv("HOME")) { */
    /*     char *home_dir = getenv("HOME"); */
    /*     config_file_location = g_strdup_printf("%s%s", home_dir, ".config"); */
    /* } */
    /* config_file_location = g_strdup_printf("%s/pinsel/init.lua", config_file_location); */
        
    if (!config_init(config_file_location))
        return 1;


    // handle all the other command line arguments
    for (int i = 1; i < argc; i++) {
        if (FALSE) {
            //
        } MATCH(argv[i], "--output", "-o") {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Missing argument for '%s'\n", argv[i - 1]);
                return 1;
            } else {
                init_dest = argv[i];
            }
        } MATCH(argv[i], "--format", "-f") {
            i++;
            if (i >= argc) {
                fprintf(stderr, "Missing argument for '%s'\n", argv[i - 1]);
                return 1;
            } else {
                if (strcmp(argv[i], "jpg") == 0)
                    argv[i] = "jpeg";
                if (!is_valid_output_format(argv[i])) {
                    fprintf(stderr, "Not a valid output format: '%s'\n", argv[i]);
                    return 1;
                }
                output_format = argv[i];
            }
        } MATCH(argv[i], "--help", "-h") {
            print_help();
            return 0;
        } MATCH(argv[i], "--version", "-v") {
            printf("%s\n", VERSION);
            return 0;
        } MATCH(argv[i], "--ontop") {
            is_on_top = TRUE;
        } MATCH(argv[i], "--maximize") {
            is_maximized = TRUE;
        } else if (!GDK_IS_PIXBUF(init_pix)) { 
            GError *err = NULL;
            init_pix = gdk_pixbuf_new_from_file(argv[i], &err);
            if (init_dest == NULL)
                init_dest = argv[i];
            if (err) {
                fprintf(stderr, "Failed to load image from argument\n%s\n", err->message);
                return 1;
            }
        }
    }

    if (!GDK_IS_PIXBUF(init_pix)) {
        fprintf(stderr, "Please provide an image as an argument or through stdin\n");
        return 1;
    }

    pix_init(init_pix);
    pix_set_dest(init_dest);

    gtk_init(&argc, &argv);

    if (!gui_init(is_on_top, is_maximized))
        return 1;

    gtk_main();

    // write the image to stdout if the apps output gets piped into another program
    if (isatty(1) != 1) {
        GError *err = NULL;
        GOutputStream *stdout_stream = g_unix_output_stream_new(1, FALSE);

        gdk_pixbuf_save_to_stream(pix_get_current(), stdout_stream, output_format, NULL, &err, NULL);
        if (err != NULL) {
            printf("%s\n", err->message);
            return 1;
        }

        g_output_stream_flush(stdout_stream, NULL, &err);
        if (err != NULL) {
            printf("%s\n", err->message);
            return 1;
        }
    }

    return 0;
}
