/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "internal.h"
#include "runtime_bridge.h"
#include "picoui/theme.h"

#include <stdlib.h>
#include <stdint.h>

typedef struct ldBase_t ldBase_t;
void ldBaseSetHeight(ldBase_t *ptWidget, int16_t height);

static const unsigned int PICOUI_THEME_V0_COLORS[PICOUI_COLOR_COUNT] = {
    [PICOUI_COLOR_TEXT_PRIMARY] = 0x1F2328U,
    [PICOUI_COLOR_BG] = 0xF6F8FAU,
    [PICOUI_COLOR_PANEL] = 0xFFFFFFU,
    [PICOUI_COLOR_BORDER] = 0xD0D7DEU,
    [PICOUI_COLOR_ACCENT] = 0x0969DAU,
    [PICOUI_COLOR_DISABLED] = 0x8C959FU,
};

static const int PICOUI_THEME_V0_METRICS[PICOUI_METRIC_COUNT] = {
    [PICOUI_METRIC_PADDING] = 8,
    [PICOUI_METRIC_RADIUS] = 6,
    [PICOUI_METRIC_BORDER_WIDTH] = 1,
    [PICOUI_METRIC_CONTROL_HEIGHT] = 32,
};

static int picoui_theme_state_is_valid(enum picoui_state state)
{
    return state >= PICOUI_STATE_DEFAULT && state <= PICOUI_STATE_FOCUSED;
}

static int picoui_theme_part_is_valid(enum picoui_part part)
{
    return part >= PICOUI_PART_MAIN && part <= PICOUI_PART_TRACK;
}

static int picoui_theme_part_supported(enum picoui_backend_widget_kind kind, enum picoui_part part)
{
    switch (kind) {
        case PICOUI_BACKEND_WIDGET_WINDOW:
            return part == PICOUI_PART_MAIN;
        case PICOUI_BACKEND_WIDGET_IMAGE:
            return part == PICOUI_PART_MAIN;
        case PICOUI_BACKEND_WIDGET_CALENDAR:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_TEXT;
        case PICOUI_BACKEND_WIDGET_LABEL:
        case PICOUI_BACKEND_WIDGET_TEXT:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_TEXT;
        case PICOUI_BACKEND_WIDGET_LIST:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_TEXT;
        case PICOUI_BACKEND_WIDGET_BUTTON:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_TEXT;
        case PICOUI_BACKEND_WIDGET_CHECKBOX:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_TEXT
                   || part == PICOUI_PART_INDICATOR;
        case PICOUI_BACKEND_WIDGET_SWITCH:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_INDICATOR
                   || part == PICOUI_PART_KNOB || part == PICOUI_PART_TRACK;
        case PICOUI_BACKEND_WIDGET_SLIDER:
            return part == PICOUI_PART_MAIN || part == PICOUI_PART_KNOB || part == PICOUI_PART_TRACK;
        default:
            return 0;
    }
}

static void picoui_theme_map_widget_colors(const struct picoui_theme *theme,
                                           enum picoui_part part,
                                           enum picoui_state state,
                                           unsigned int *bg,
                                           unsigned int *text,
                                           unsigned int *border)
{
    unsigned int bg_color = theme->colors[PICOUI_COLOR_PANEL];
    unsigned int text_color = theme->colors[PICOUI_COLOR_TEXT_PRIMARY];
    unsigned int border_color = theme->colors[PICOUI_COLOR_BORDER];
    int active_state = state == PICOUI_STATE_PRESSED || state == PICOUI_STATE_CHECKED
                       || state == PICOUI_STATE_FOCUSED;

    if (part == PICOUI_PART_TRACK) {
        bg_color = theme->colors[PICOUI_COLOR_BORDER];
    }

    if (state == PICOUI_STATE_DISABLED) {
        bg_color = theme->colors[PICOUI_COLOR_BG];
        text_color = theme->colors[PICOUI_COLOR_DISABLED];
        border_color = theme->colors[PICOUI_COLOR_DISABLED];
    } else if (active_state) {
        if (part == PICOUI_PART_TEXT) {
            text_color = theme->colors[PICOUI_COLOR_ACCENT];
        } else if (part == PICOUI_PART_KNOB) {
            bg_color = theme->colors[PICOUI_COLOR_ACCENT];
        } else {
            bg_color = theme->colors[PICOUI_COLOR_ACCENT];
            border_color = theme->colors[PICOUI_COLOR_ACCENT];
        }
    }

    *bg = bg_color;
    *text = text_color;
    *border = border_color;
}

static int picoui_theme_apply_widget_metrics(const struct picoui_theme *theme,
                                             struct picoui_widget *widget,
                                             enum picoui_backend_widget_kind kind)
{
    struct picoui_backend_widget *backend_widget = (struct picoui_backend_widget *)widget->backend_widget;

    if (kind != PICOUI_BACKEND_WIDGET_WINDOW && kind != PICOUI_BACKEND_WIDGET_IMAGE) {
        widget->height = theme->metrics[PICOUI_METRIC_CONTROL_HEIGHT];
        if (backend_widget != 0 && backend_widget->ld_widget != 0) {
            ldBaseSetHeight((ldBase_t *)backend_widget->ld_widget,
                            (int16_t)theme->metrics[PICOUI_METRIC_CONTROL_HEIGHT]);
        }
    }

    if (picoui_widget_set_radius(widget, theme->metrics[PICOUI_METRIC_RADIUS]) != 0
        || picoui_widget_set_padding(widget, theme->metrics[PICOUI_METRIC_PADDING]) != 0) {
        return -1;
    }

    return 0;
}

static int picoui_theme_backend_can_apply_style(const struct picoui_backend_widget *backend_widget)
{
    return backend_widget != 0
        && backend_widget->ld_widget != 0
        && backend_widget->theme != 0;
}

/**
 * @brief Create theme instance
 *
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_theme *picoui_theme_create(void)
{
    struct picoui_theme *theme = calloc(1, sizeof(struct picoui_theme));
    int i;

    if (theme == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_COLOR_COUNT; ++i) {
        theme->colors[i] = PICOUI_THEME_V0_COLORS[i];
    }

    for (i = 0; i < PICOUI_METRIC_COUNT; ++i) {
        theme->metrics[i] = PICOUI_THEME_V0_METRICS[i];
    }

    return theme;
}

/**
 * @brief Destroy theme instance
 *
 * @param[in] theme Theme instance
 */

void picoui_theme_destroy(struct picoui_theme *theme)
{
    free(theme);
}

/**
 * @brief Set color of theme
 *
 * @param[in] theme Theme instance
 * @param[in] id Widget identifier string
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_theme_set_color(struct picoui_theme *theme, enum picoui_color_id id, unsigned int rgb)
{
    if (theme == 0 || id < 0 || id >= PICOUI_COLOR_COUNT) {
        return -1;
    }

    theme->colors[id] = rgb;
    return 0;
}

/**
 * @brief Set metric of theme
 *
 * @param[in] theme Theme instance
 * @param[in] id Widget identifier string
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_theme_set_metric(struct picoui_theme *theme, enum picoui_metric_id id, int value)
{
    if (theme == 0 || id < 0 || id >= PICOUI_METRIC_COUNT || value < 0) {
        return -1;
    }

    theme->metrics[id] = value;
    return 0;
}

/**
 * @brief Theme: apply to widget
 *
 * @param[in] theme Theme instance
 * @param[in] widget Widget instance
 * @param[in] part part
 * @param[in] state State value
 * @return -1 on failure
 */

int picoui_theme_apply_to_widget(struct picoui_theme *theme,
                                 struct picoui_widget *widget,
                                 enum picoui_part part,
                                 enum picoui_state state)
{
    struct picoui_backend_widget *backend_widget;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;

    if (theme == 0 || widget == 0 || widget->backend_widget == 0) {
        return -1;
    }

    if (!picoui_theme_part_is_valid(part) || !picoui_theme_state_is_valid(state)) {
        return -1;
    }

    backend_widget = (struct picoui_backend_widget *)widget->backend_widget;
    if (!picoui_theme_part_supported(backend_widget->kind, part)) {
        return -1;
    }
    if (!picoui_theme_backend_can_apply_style(backend_widget)) {
        return -1;
    }

    picoui_theme_map_widget_colors(theme, part, state, &bg_color, &text_color, &border_color);
    if (picoui_theme_apply_widget_metrics(theme, widget, backend_widget->kind) != 0) {
        return -1;
    }

    widget->bg_color = bg_color;
    widget->text_color = text_color;
    widget->border_color = border_color;
    return picoui_backend_widget_apply_style(widget->backend_widget,
                                             part,
                                             state,
                                             bg_color,
                                             text_color,
                                             border_color);
}

/**
 * @brief Set theme of app
 *
 * @param[in] app Application instance
 * @param[in] theme Theme instance
 * @return -1 on failure
 */

int picoui_app_set_theme(struct picoui_app *app, struct picoui_theme *theme)
{
    if (app == 0 || theme == 0) {
        return -1;
    }

    return picoui_runtime_bridge_bind_theme(app, theme);
}
