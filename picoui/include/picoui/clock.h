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

#ifndef PICOUI_CLOCK_H
#define PICOUI_CLOCK_H

struct picoui_widget;
struct picoui_clock;
struct picoui_image_source;

struct picoui_clock_props {
    const char *id;
    const char *style_class;
    void *user_data;
    struct picoui_image_source *background_source;
    struct picoui_image_source *hour_pointer_source;
    struct picoui_image_source *minute_pointer_source;
    struct picoui_image_source *second_pointer_source;
    unsigned int mask_color;
    float hour_anchor_x;
    float hour_anchor_y;
    float minute_anchor_x;
    float minute_anchor_y;
    float second_anchor_x;
    float second_anchor_y;
    int step_second;
};

/**
 * @brief Create clock widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_clock *picoui_clock_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create clock widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_clock *picoui_clock_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_clock_props *props);

/**
 * @brief clock init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_clock *picoui_clock_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Set use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_use_system_time(struct picoui_clock *clock, int enabled);

/**
 * @brief Get use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return The property value, negative on error
 */

int picoui_clock_get_use_system_time(const struct picoui_clock *clock);

/**
 * @brief Set step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] step_second step second
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_step_second(struct picoui_clock *clock, int step_second);

/**
 * @brief Get step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return The property value, negative on error
 */

int picoui_clock_get_step_second(const struct picoui_clock *clock);

/**
 * @brief Set manual time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] hour Hour value [0, 23]
 * @param[in] minute Minute value [0, 59]
 * @param[in] second Second value [0, 59]
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_time(struct picoui_clock *clock, int hour, int minute, int second);

/**
 * @brief Get manual time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[out] hour Hour value
 * @param[out] minute Minute value
 * @param[out] second Second value
 * @return 0 on success, -1 on failure
 */

int picoui_clock_get_time(const struct picoui_clock *clock, int *hour, int *minute, int *second);

/**
 * @brief Advance clock widget by one second
 *
 * @param[in] clock Clock widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_clock_tick(struct picoui_clock *clock);

/**
 * @brief Set background image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_background_image(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set background source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_background_source(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set hour pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_hour_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set hour pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_hour_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set minute pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_minute_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set minute pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_minute_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set second pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_second_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set second pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_second_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source);

/**
 * @brief Set mask color of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_mask_color(struct picoui_clock *clock, unsigned int mask_color);

/**
 * @brief Set hour anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_hour_anchor(struct picoui_clock *clock, float x, float y);

/**
 * @brief Set minute anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_minute_anchor(struct picoui_clock *clock, float x, float y);

/**
 * @brief Set second anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_second_anchor(struct picoui_clock *clock, float x, float y);

#endif
