#ifndef TINYUI_EVENT_H
#define TINYUI_EVENT_H

#include "core/obj.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum tinyui_event_code {
    TINYUI_EVENT_PRESSED = 0,
    TINYUI_EVENT_RELEASED,
    TINYUI_EVENT_CLICKED,
    TINYUI_EVENT_VALUE_CHANGED,
    TINYUI_EVENT_FOCUSED,
    TINYUI_EVENT_DEFOCUSED,
    TINYUI_EVENT_KEY,
    TINYUI_EVENT_DELETE,
} tinyui_event_code_t;

typedef struct tinyui_key_event_data {
    uint16_t key;
    bool pressed;
} tinyui_key_event_data_t;

typedef struct tinyui_event {
    tinyui_event_code_t code;
    tinyui_obj_t *target;
    void *user_data;
    union {
        int32_t value;
        tinyui_key_event_data_t key;
    } data;
} tinyui_event_t;

typedef void (*tinyui_event_cb_t)(const tinyui_event_t *event);
typedef uint32_t tinyui_event_handle_t;

#ifndef TINYUI_EVENT_CB_CAPACITY
#define TINYUI_EVENT_CB_CAPACITY 16
#endif

#define TINYUI_EVENT_MASK(code) (UINT32_C(1) << (code))
#define TINYUI_EVENT_MASK_ALL UINT32_MAX

tinyui_result_t tinyui_obj_add_event_cb(tinyui_obj_t *obj,
                                        uint32_t event_mask,
                                        tinyui_event_cb_t cb,
                                        void *user_data,
                                        tinyui_event_handle_t *handle);
tinyui_result_t tinyui_obj_remove_event_cb(tinyui_obj_t *obj,
                                           tinyui_event_handle_t handle);

#endif
