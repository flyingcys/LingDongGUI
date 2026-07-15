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

#include "theme_showcase/theme_showcase.h"
#include "tinyui.h"

#include <stdio.h>

/* Theme descriptor lives for the process; apply may retain references. */
static tinyui_theme_t s_theme;
static tinyui_font_t s_font;

tinyui_result_t tinyui_demo_theme_showcase_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *body;
    tinyui_obj_t *accent;
    tinyui_style_t style;
    tinyui_result_t rc;
    int i;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    for (i = 0; i < TINYUI_COLOR_COUNT; ++i) {
        s_theme.colors[i] = 0xF6F8FAU;
    }
    for (i = 0; i < TINYUI_METRIC_COUNT; ++i) {
        s_theme.metrics[i] = 8;
    }
    s_theme.colors[TINYUI_COLOR_TEXT_PRIMARY] = 0x102030U;
    s_theme.colors[TINYUI_COLOR_BG] = 0xEEF2F7U;
    s_theme.colors[TINYUI_COLOR_PANEL] = 0xD8E2F0U;
    s_theme.colors[TINYUI_COLOR_BORDER] = 0x8B949EU;
    s_theme.colors[TINYUI_COLOR_ACCENT] = 0x1F6FEBU;
    s_theme.colors[TINYUI_COLOR_DISABLED] = 0xA0A8B0U;
    s_theme.metrics[TINYUI_METRIC_PADDING] = 12;
    s_theme.metrics[TINYUI_METRIC_RADIUS] = 6;
    s_theme.metrics[TINYUI_METRIC_BORDER_WIDTH] = 1;
    s_theme.metrics[TINYUI_METRIC_CONTROL_HEIGHT] = 36;

    if (tinyui_theme_set(&s_theme) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    (void)tinyui_obj_set_bg_color(screen, s_theme.colors[TINYUI_COLOR_BG]);
    rc = tinyui_theme_apply(screen);
    if (rc == TINYUI_ERROR_NOT_SUPPORTED) {
        /* Theme apply may be partial on some backends. */
    } else if (rc != TINYUI_OK) {
        return rc;
    }

    (void)tinyui_font_from_builtin(TINYUI_FONT_6X8, &s_font);

    title = tinyui_label_create(screen);
    body = tinyui_text_create(screen);
    accent = tinyui_button_create(screen);
    if (title == NULL || body == NULL || accent == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_obj_set_pos(title, 32, 32) != TINYUI_OK
        || tinyui_obj_set_size(title, 240, 32) != TINYUI_OK
        || tinyui_label_set_text(title, "Theme") != 0
        || tinyui_label_set_text_color(title, s_theme.colors[TINYUI_COLOR_TEXT_PRIMARY]) != 0
        || tinyui_label_set_bg_color(title, s_theme.colors[TINYUI_COLOR_PANEL]) != 0
        || tinyui_label_set_transparent(title, 0) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    /* Temporary style descriptor — valid only through apply return. */
    style.fields = TINYUI_STYLE_BG_COLOR
                 | TINYUI_STYLE_TEXT_COLOR
                 | TINYUI_STYLE_BORDER_COLOR
                 | TINYUI_STYLE_BORDER_WIDTH
                 | TINYUI_STYLE_RADIUS
                 | TINYUI_STYLE_PADDING;
    style.bg_color = s_theme.colors[TINYUI_COLOR_PANEL];
    style.text_color = s_theme.colors[TINYUI_COLOR_TEXT_PRIMARY];
    style.border_color = s_theme.colors[TINYUI_COLOR_BORDER];
    style.border_width = s_theme.metrics[TINYUI_METRIC_BORDER_WIDTH];
    style.radius = s_theme.metrics[TINYUI_METRIC_RADIUS];
    style.padding = s_theme.metrics[TINYUI_METRIC_PADDING];
    style.opacity = 255;
    style.font = &s_font;

    rc = tinyui_obj_apply_style(title, TINYUI_PART_MAIN, TINYUI_STATE_DEFAULT, &style);
    if (rc == TINYUI_ERROR_NOT_SUPPORTED) {
        /* part/state unsupported — do not invent fake painted state */
    } else if (rc != TINYUI_OK) {
        return rc;
    }

    if (tinyui_obj_set_pos(body, 32, 90) != TINYUI_OK
        || tinyui_obj_set_size(body, 300, 40) != TINYUI_OK
        || tinyui_text_set_text(body, "Accent preview") != 0
        || tinyui_text_set_text_color(body, s_theme.colors[TINYUI_COLOR_TEXT_PRIMARY]) != 0
        || tinyui_text_set_bg_color(body, s_theme.colors[TINYUI_COLOR_PANEL]) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_obj_set_pos(accent, 32, 160) != TINYUI_OK
        || tinyui_obj_set_size(accent, 140, 40) != TINYUI_OK
        || tinyui_button_set_text(accent, "Primary") != 0
        || tinyui_button_set_color(accent,
                                  s_theme.colors[TINYUI_COLOR_ACCENT],
                                  0x1158C7U) != 0
        || tinyui_button_set_text_color(accent, 0xFFFFFFU) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    printf("TINYUI_SCENARIO=theme_showcase\n");
    fflush(stdout);
    return TINYUI_OK;
}
