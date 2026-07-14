#ifndef TINYUI_FOCUS_H
#define TINYUI_FOCUS_H

#include "core/obj.h"

typedef enum tinyui_focus_direction {
    TINYUI_FOCUS_NEXT = 0,
    TINYUI_FOCUS_PREVIOUS,
    TINYUI_FOCUS_LEFT,
    TINYUI_FOCUS_RIGHT,
    TINYUI_FOCUS_UP,
    TINYUI_FOCUS_DOWN,
} tinyui_focus_direction_t;

tinyui_result_t tinyui_focus_set(tinyui_obj_t *obj);
tinyui_result_t tinyui_focus_clear(void);
tinyui_result_t tinyui_focus_move(tinyui_focus_direction_t direction);
tinyui_obj_t *tinyui_focus_current(void);

#endif
