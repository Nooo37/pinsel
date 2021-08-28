#ifndef CONFIG_H
#define CONFIG_H

#include "ui_state.h"

typedef struct {
    gboolean shift;
    gboolean control;
    gboolean alt;
    gboolean button1;
    gboolean button2;
    gboolean button3;
} Modifiers;

extern int config_init();

extern void config_perform_action(Action *action);

extern void config_perform_self_contained_action(ActionType type);

extern void config_perform_key_binding(char *key, Modifiers event);

extern void config_perform_mouse_binding(int x, int y, Modifiers event);

extern void config_perform_click_binding(int button, int x, int y, Modifiers event);

extern char* config_get_shortcut_ui();

#endif
