#include <gtk/gtk.h>


/* 
 * Utils are functions that only depend on constants and thus can be placed 
 * outside the big UI file
 */

typedef struct
{
    int x;
    int y;
} coord_t;

extern float get_sane_scale(int img_width, int img_height, int area_width, int area_height);

extern gboolean is_no_mod(GdkEventKey* key);

extern gboolean is_only_control(GdkEventKey* key);

extern gboolean is_only_alt(GdkEventKey* key);

extern gboolean is_only_alt_control(GdkEventKey* key);

extern gboolean is_valid_output_format(char *format);

extern void print_help();
