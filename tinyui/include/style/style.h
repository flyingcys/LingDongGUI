#ifndef TINYUI_STYLE_H
#define TINYUI_STYLE_H

#include "core/obj.h"

#include <stdint.h>

typedef struct tinyui_font tinyui_font_t;

typedef enum tinyui_style_field {
    TINYUI_STYLE_BG_COLOR = UINT32_C(1) << 0,
    TINYUI_STYLE_TEXT_COLOR = UINT32_C(1) << 1,
    TINYUI_STYLE_BORDER_COLOR = UINT32_C(1) << 2,
    TINYUI_STYLE_BORDER_WIDTH = UINT32_C(1) << 3,
    TINYUI_STYLE_RADIUS = UINT32_C(1) << 4,
    TINYUI_STYLE_PADDING = UINT32_C(1) << 5,
    TINYUI_STYLE_OPACITY = UINT32_C(1) << 6,
    TINYUI_STYLE_FONT = UINT32_C(1) << 7,
} tinyui_style_field_t;

typedef struct tinyui_style tinyui_style_t;

struct tinyui_style {
    uint32_t fields;
    uint32_t bg_color;
    uint32_t text_color;
    uint32_t border_color;
    int16_t border_width;
    int16_t radius;
    int16_t padding;
    uint8_t opacity;
    const tinyui_font_t *font;
};

typedef enum tinyui_part {
    TINYUI_PART_MAIN = 0,
    TINYUI_PART_TEXT,
    TINYUI_PART_INDICATOR,
    TINYUI_PART_KNOB,
    TINYUI_PART_TRACK,
} tinyui_part_t;

typedef enum tinyui_state {
    TINYUI_STATE_DEFAULT = 0,
    TINYUI_STATE_DISABLED,
    TINYUI_STATE_PRESSED,
    TINYUI_STATE_CHECKED,
    TINYUI_STATE_FOCUSED,
} tinyui_state_t;

tinyui_result_t tinyui_obj_apply_style(tinyui_obj_t *obj,
                                       tinyui_part_t part,
                                       tinyui_state_t state,
                                       const tinyui_style_t *style);

#endif
