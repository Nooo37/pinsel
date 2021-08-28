#ifndef CONFIG_H
#define CONFIG_H

#include "ui_state.h"

typedef struct {
    gboolean button1;
    gboolean button2;
    gboolean button3;
    gboolean shift;
    gboolean control;
    gboolean alt;
} Modifiers;

extern int config_init();

extern void config_perform_action(Action *action);

extern void config_perform_self_contained_action(ActionType type);

extern void config_perform_key_event(char *key, Modifiers mod);

extern void config_perform_click_event(int button, int x, int y, Modifiers mod);

extern void config_perform_motion_event(int x, int y, Modifiers mod);

extern char* config_get_shortcut_ui();

extern void config_notify_text(char* new_text);

extern void config_notify_text_close(gboolean accepted_changes);

/* extern void config_notify_color1(float r, float g, float b, float a); */
/*  */
/* extern void config_notify_color2(float r, float g, float b, float a); */
/*  */
/* extern void config_notify_mode(Mode new_mode); */

extern int config_get_history_limit();

#endif
