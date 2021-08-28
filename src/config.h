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

#endif
