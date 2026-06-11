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

#ifndef PICOUI_THEME_H
#define PICOUI_THEME_H

struct picoui_app;
struct picoui_theme;
struct picoui_widget;

enum picoui_state {
    PICOUI_STATE_DEFAULT = 0,
    PICOUI_STATE_DISABLED = 1,
    PICOUI_STATE_PRESSED = 2,
    PICOUI_STATE_CHECKED = 3,
    PICOUI_STATE_FOCUSED = 4,
};

enum picoui_part {
    PICOUI_PART_MAIN,
    PICOUI_PART_TEXT,
    PICOUI_PART_INDICATOR,
    PICOUI_PART_KNOB,
    PICOUI_PART_TRACK,
};

enum picoui_color_id {
    PICOUI_COLOR_TEXT_PRIMARY,
    PICOUI_COLOR_BG,
    PICOUI_COLOR_PANEL,
    PICOUI_COLOR_BORDER,
    PICOUI_COLOR_ACCENT,
    PICOUI_COLOR_DISABLED,
    PICOUI_COLOR_COUNT,
};

enum picoui_metric_id {
    PICOUI_METRIC_PADDING,
    PICOUI_METRIC_RADIUS,
    PICOUI_METRIC_BORDER_WIDTH,
    PICOUI_METRIC_CONTROL_HEIGHT,
    PICOUI_METRIC_COUNT,
};

/**
 * @brief Create theme instance
 *
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_theme *picoui_theme_create(void);

/**
 * @brief Destroy theme instance
 *
 * @param[in] theme Theme instance
 */

void picoui_theme_destroy(struct picoui_theme *theme);

/**
 * @brief Set color of theme
 *
 * @param[in] theme Theme instance
 * @param[in] id Widget identifier string
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_theme_set_color(struct picoui_theme *theme, enum picoui_color_id id, unsigned int rgb);

/**
 * @brief Set metric of theme
 *
 * @param[in] theme Theme instance
 * @param[in] id Widget identifier string
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_theme_set_metric(struct picoui_theme *theme, enum picoui_metric_id id, int value);

/**
 * @brief Theme: apply to widget
 *
 * @param[in] theme Theme instance
 * @param[in] widget Widget instance
 * @param[in] part part
 * @param[in] state State value
 * @return 0 on success, -1 on failure
 */

int picoui_theme_apply_to_widget(struct picoui_theme *theme,
                                 struct picoui_widget *widget,
                                 enum picoui_part part,
                                 enum picoui_state state);

/**
 * @brief Set theme of app
 *
 * @param[in] app Application instance
 * @param[in] theme Theme instance
 * @return 0 on success, -1 on failure
 */

int picoui_app_set_theme(struct picoui_app *app, struct picoui_theme *theme);

#endif
