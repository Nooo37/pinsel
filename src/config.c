#include "ui_state.h"
#include "pixbuf.h"

extern void config_perform_action(Action *action)
{
    switch (action->type) {
        case ZOOM: case MOVE_HORIZONTALLY: case MOVE_VERTICALLY:
        case FIT_POSITION:
            ui_perform_action(action);
        default:
            pix_perform_action(action);
    }
}

extern void config_perform_self_contained_action(ActionType type)
{
    Action temp;
    temp.type = type;
    config_perform_action(&temp);
}
