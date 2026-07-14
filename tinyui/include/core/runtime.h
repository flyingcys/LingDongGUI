#ifndef TINYUI_RUNTIME_H
#define TINYUI_RUNTIME_H

#include "core/obj.h"

#include <stdint.h>

typedef struct tinyui_window_props tinyui_window_props_t;
typedef struct tinyui_background_props tinyui_background_props_t;

typedef enum tinyui_screen_transition {
    TINYUI_SCREEN_TRANSITION_NONE = 0,
    TINYUI_SCREEN_TRANSITION_FADE_WHITE,
    TINYUI_SCREEN_TRANSITION_FADE_BLACK,
    TINYUI_SCREEN_TRANSITION_SLIDE_LEFT,
    TINYUI_SCREEN_TRANSITION_SLIDE_RIGHT,
    TINYUI_SCREEN_TRANSITION_SLIDE_UP,
    TINYUI_SCREEN_TRANSITION_SLIDE_DOWN,
    TINYUI_SCREEN_TRANSITION_ERASE_LEFT,
    TINYUI_SCREEN_TRANSITION_ERASE_RIGHT,
    TINYUI_SCREEN_TRANSITION_ERASE_UP,
    TINYUI_SCREEN_TRANSITION_ERASE_DOWN,
    TINYUI_SCREEN_TRANSITION_FLY_IN_LEFT,
    TINYUI_SCREEN_TRANSITION_FLY_IN_RIGHT,
    TINYUI_SCREEN_TRANSITION_FLY_IN_TOP,
    TINYUI_SCREEN_TRANSITION_FLY_IN_BOTTOM,
} tinyui_screen_transition_t;

tinyui_result_t tinyui_init(void);
void tinyui_deinit(void);

tinyui_obj_t *tinyui_screen_create(void);
tinyui_obj_t *tinyui_screen_create_with_props(const tinyui_window_props_t *props);
tinyui_obj_t *tinyui_background_create(void);
tinyui_obj_t *tinyui_background_create_with_props(const tinyui_background_props_t *props);

tinyui_result_t tinyui_screen_load(tinyui_obj_t *screen,
                                   tinyui_screen_transition_t transition,
                                   uint32_t duration_ms);
tinyui_obj_t *tinyui_screen_active(void);
tinyui_result_t tinyui_process(uint32_t *next_ms);

#endif
