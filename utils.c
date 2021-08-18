#include <gtk/gtk.h>
#include "config.h"

extern float get_sane_scale(int img_width, int img_height, int area_width, int area_height) 
{
    if ((img_width / img_height) > (area_width / area_height))
        return (area_width / (float) img_width) * (1 - SANE_SCALE_MARGIN);
    else
        return (area_height / (float) img_height) * (1 - SANE_SCALE_MARGIN);
}


extern gboolean write_pixbuf_to_stdout(GdkPixbuf* pixbuf)
{
    remove(TEMP_OUT_FILE);
    gdk_pixbuf_save(pixbuf, TEMP_OUT_FILE, "png", NULL, NULL);
    FILE *a = fopen(TEMP_OUT_FILE, "r");
    int n;
    char s[65536];
    while ((n = fread(s, 1, sizeof(s), a))) {
        fwrite(s, 1, n, stdout);
    }
    return TRUE;
}

extern gboolean write_stdin_to_file()
{
    void *content = malloc(BUF_SIZE);

    FILE *fp = fopen(TEMP_IN_FILE, "w");

    if (fp == 0)
        return FALSE;

    int read;
    while ((read = fread(content, 1, BUF_SIZE, stdin))) {
        fwrite(content, read, 1, fp);
    }
    if (ferror(stdin))
        return FALSE;

    fclose(fp);
    return TRUE;
}

