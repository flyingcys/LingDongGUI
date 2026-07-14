#ifndef TINYUI_THEME_H
#define TINYUI_THEME_H

#include "core/obj.h"
#include "core/result.h"
#include "style/style.h"

#include <stdint.h>

typedef enum tinyui_color_id {
    TINYUI_COLOR_TEXT_PRIMARY = 0,
    TINYUI_COLOR_BG,
    TINYUI_COLOR_PANEL,
    TINYUI_COLOR_BORDER,
    TINYUI_COLOR_ACCENT,
    TINYUI_COLOR_DISABLED,
    TINYUI_COLOR_COUNT,
} tinyui_color_id_t;

typedef enum tinyui_metric_id {
    TINYUI_METRIC_PADDING = 0,
    TINYUI_METRIC_RADIUS,
    TINYUI_METRIC_BORDER_WIDTH,
    TINYUI_METRIC_CONTROL_HEIGHT,
    TINYUI_METRIC_COUNT,
} tinyui_metric_id_t;

typedef struct tinyui_theme tinyui_theme_t;

struct tinyui_theme {
    uint32_t colors[TINYUI_COLOR_COUNT];
    int16_t metrics[TINYUI_METRIC_COUNT];
};

tinyui_result_t tinyui_theme_set(const tinyui_theme_t *theme);
const tinyui_theme_t *tinyui_theme_get(void);
tinyui_result_t tinyui_theme_apply(tinyui_obj_t *obj);

#endif
