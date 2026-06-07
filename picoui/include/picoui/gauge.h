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

#ifndef PICOUI_GAUGE_H
#define PICOUI_GAUGE_H

struct picoui_widget;
struct picoui_gauge;
struct picoui_image_source;

struct picoui_gauge_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float angle;
    struct picoui_image_source *bg_source;
    struct picoui_image_source *pointer_source;
    int centre_offset_x;
    int centre_offset_y;
    unsigned int pointer_color;
    int auto_move;
};

/**
 * @brief Create gauge widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_gauge *picoui_gauge_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create gauge widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_gauge *picoui_gauge_create_with_props(struct picoui_widget *parent,
                                                    const struct picoui_gauge_props *props);

/**
 * @brief gauge init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_gauge *picoui_gauge_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Set angle of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] angle Angle in degrees
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_angle(struct picoui_gauge *gauge, float angle);

/**
 * @brief Set value range of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] min_value Minimum value
 * @param[in] max_value Maximum value
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_range(struct picoui_gauge *gauge, int min_value, int max_value);

/**
 * @brief Set current value of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] value Current value
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_value(struct picoui_gauge *gauge, int value);

/**
 * @brief Set tick count of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] tick_count Tick count
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_tick_count(struct picoui_gauge *gauge, int tick_count);

/**
 * @brief Get angle of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 */

float picoui_gauge_get_angle(const struct picoui_gauge *gauge);

/**
 * @brief Get minimum value of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @return minimum value, 0 on failure
 */

int picoui_gauge_get_min_value(const struct picoui_gauge *gauge);

/**
 * @brief Get maximum value of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @return maximum value, 0 on failure
 */

int picoui_gauge_get_max_value(const struct picoui_gauge *gauge);

/**
 * @brief Get current value of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @return current value, 0 on failure
 */

int picoui_gauge_get_value(const struct picoui_gauge *gauge);

/**
 * @brief Get tick count of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @return tick count, 0 on failure
 */

int picoui_gauge_get_tick_count(const struct picoui_gauge *gauge);

/**
 * @brief Set bg source of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_bg_source(struct picoui_gauge *gauge, struct picoui_image_source *source);

/**
 * @brief Set pointer source of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_pointer_source(struct picoui_gauge *gauge, struct picoui_image_source *source);

/**
 * @brief Set centre offset of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] centre_offset_x centre offset x
 * @param[in] centre_offset_y centre offset y
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_centre_offset(struct picoui_gauge *gauge, int centre_offset_x, int centre_offset_y);

/**
 * @brief Set trail of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] bg_trail_source bg trail source
 * @param[in] pointer_trail_source pointer trail source
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_trail(struct picoui_gauge *gauge,
                           struct picoui_image_source *bg_trail_source,
                           struct picoui_image_source *pointer_trail_source);

/**
 * @brief Set progress bar of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] bg_progress_source bg progress source
 * @param[in] pointer_progress_source pointer progress source
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_progress_bar(struct picoui_gauge *gauge,
                                  struct picoui_image_source *bg_progress_source,
                                  struct picoui_image_source *pointer_progress_source);

/**
 * @brief Set pointer color of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] pointer_color pointer color
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color);

/**
 * @brief Get pointer color of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 */

unsigned int picoui_gauge_get_pointer_color(const struct picoui_gauge *gauge);

/**
 * @brief Set auto move of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] auto_move auto move
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move);

/**
 * @brief Get auto move of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @return The property value, negative on error
 */

int picoui_gauge_get_auto_move(const struct picoui_gauge *gauge);

#endif
