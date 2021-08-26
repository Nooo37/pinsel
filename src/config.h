#ifndef CONFIG_H
#define CONFIG_H

#include "ui_state.h"

extern int config_init();

extern void config_perform_action(Action *action);

extern void config_perform_self_contained_action(ActionType type);

extern void config_perform_event(gpointer event);

extern char* config_get_shortcut_ui();

#endif
