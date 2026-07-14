#ifndef TINYUI_INTEGRATION_INPUT_H
#define TINYUI_INTEGRATION_INPUT_H

#include "core/result.h"

#include <stdbool.h>

typedef enum tinyui_key {
    TINYUI_KEY_ENTER = 0,
    TINYUI_KEY_BACK,
    TINYUI_KEY_LEFT,
    TINYUI_KEY_RIGHT,
    TINYUI_KEY_UP,
    TINYUI_KEY_DOWN,
    TINYUI_KEY_NEXT,
    TINYUI_KEY_PREVIOUS,
} tinyui_key_t;

tinyui_result_t tinyui_input_send_key(tinyui_key_t key, bool pressed);

#endif
