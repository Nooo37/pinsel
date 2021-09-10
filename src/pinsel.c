#include <gtk/gtk.h>
#include <gio/gunixoutputstream.h>
#include <gio/gunixinputstream.h>
#include <stdlib.h>
#include <unistd.h>
#include <argp.h>

#include "pinsel.h"
#include "config.h"
#include "pixbuf.h"
#include "utils.h"
#include "gui.h"

struct arguments {
    gboolean is_on_top;
    gboolean is_maximized;
    const char *output_format;
    const char *init_dest;
    const char *pixbuf_file;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);

const char *argp_program_version = "pinsel 0.0.0";
const char *argp_program_bug_address = "https://github.com/Nooo37/pinsel/issues/new";
static char doc[] = "Minimal screenshot annotation tool with lua config";
static char args_doc[] = "FILE";

static struct argp_option options[] = {
//    long name     short   arg flags doc
    { "output",     'o',    "FILE", 0,   "Save result to FILE"              },
    { "format",     'f',    0,      0,   "Define the image format"          },
    { "ontop",      't',    0,      0,   "Puts the window above all others" },
    { "maximize",   'm',    0,      0,   "Maximizes the window on startup"  },
    { 0 }
};

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[])
{
    struct arguments args = {
        .is_on_top = FALSE,
        .is_maximized = FALSE,
        .output_format = "png",
        .init_dest = NULL,
        .pixbuf_file = NULL
    };

    // handle command line arguments
    argp_parse(&argp, argc, argv, 0, 0, &args);

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
    // read image from file parameter
    } else if (args.pixbuf_file != NULL) {
        GError *err = NULL;
        init_pix = gdk_pixbuf_new_from_file(args.pixbuf_file, &err);
        if (args.init_dest == NULL)
            args.init_dest = args.pixbuf_file;
        if (err) {
            fprintf(stderr, "Failed to load image from argument\n%s\n",
                    err->message);
            return 1;
        }
    } else {
        argp_help(&argp, stdout,
                  ARGP_HELP_USAGE | ARGP_HELP_DOC | ARGP_HELP_LONG,
                  "pinsel");
    }

    char* config_file_location = "init.lua";
    gboolean use_default_config = FALSE;
    if (getenv("XDG_CONFIG_HOME"))
        config_file_location = getenv("XDG_CONFIG_HOME");
    else if (getenv("HOME")) {
        char *home_dir = getenv("HOME");
        config_file_location = g_strdup_printf("%s/%s", home_dir, ".config");
    }
    config_file_location = g_strdup_printf("%s/pinsel/init.lua", config_file_location);

    if (access(config_file_location, R_OK) != 0)
        use_default_config = TRUE;

    if (!config_init(config_file_location, use_default_config)) {
        fprintf(stderr, "Failed to initalize configuration\n");
        return 1;
    }

    if (!GDK_IS_PIXBUF(init_pix)) {
        fprintf(stderr, "Please provide an image as an argument or through stdin\n");
        return 1;
    }

    int limit = config_get_history_limit();
    pix_init(init_pix, limit);
    pix_set_dest(args.init_dest);

    gtk_init(&argc, &argv);

    if (!gui_init(args.is_on_top, args.is_maximized))
        return 1;

    gtk_main();

    // write the image to stdout if the apps output gets piped into another program
    if (isatty(1) != 1) {
        GError *err = NULL;
        GOutputStream *stdout_stream = g_unix_output_stream_new(1, FALSE);

        gdk_pixbuf_save_to_stream(pix_get_current(), stdout_stream,
                                  args.output_format, NULL, &err, NULL);
        if (err != NULL) {
            fprintf(stderr, "%s\n", err->message);
            return 1;
        }

        g_output_stream_flush(stdout_stream, NULL, &err);
        if (err != NULL) {
            fprintf(stderr, "%s\n", err->message);
            return 1;
        }
    }

    return 0;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        struct arguments *arguments = state->input;

        switch (key) {
                case 'o': /* output */
                        arguments->init_dest = arg;
                        break;
                case 'f': /* format */
                        arguments->output_format = arg;
                        break;
                case 't': /* on top */
                        arguments->is_on_top = TRUE;
                        break;
                case 'm': /* maximize */
                        arguments->is_maximized = TRUE;
                        break;
                case ARGP_KEY_ARG:
                        if (state->arg_num > 1) {
                                argp_usage(state);
                        }

                        arguments->pixbuf_file = arg;
                        break;

                default:
                        return ARGP_ERR_UNKNOWN;
        }

        return 0;
}
