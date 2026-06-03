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
#include "picoui/clock.h"
#include "picoui/widget.h"

#include <stdlib.h>

static int picoui_clock_props_are_valid(const struct picoui_clock_props *props)
{
    return props != 0 && props->id != 0 && (props->step_second == 0 || props->step_second == 1);
}

/**
 * @brief Create clock widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_clock *picoui_clock_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_clock *clock;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    clock = calloc(1, sizeof(*clock));
    if (clock == 0) {
        return 0;
    }

    clock->widget.backend_widget = picoui_backend_create_clock(parent->backend_widget, id);
    if (clock->widget.backend_widget == 0) {
        free(clock);
        return 0;
    }

    clock->id = id;
    clock->widget.visible = 1;
    clock->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(clock->widget.backend_widget, &clock->widget) != 0) {
        free(clock);
        return 0;
    }
    clock->mask_color = 0;
    clock->hour_anchor_x = 0.0f;
    clock->hour_anchor_y = 67.0f;
    clock->minute_anchor_x = 0.0f;
    clock->minute_anchor_y = 100.0f;
    clock->second_anchor_x = 0.0f;
    clock->second_anchor_y = 100.0f;
    clock->use_system_time = 1;
    if (picoui_clock_set_step_second(clock, 0) != 0) {
        free(clock);
        return 0;
    }
    return clock;
}

/**
 * @brief clock init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_clock *picoui_clock_init(struct picoui_widget *parent, const char *id)
{
    return picoui_clock_create(parent, id);
}

/**
 * @brief Set use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_use_system_time(struct picoui_clock *clock, int enabled)
{
    if (clock == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_use_system_time(clock, enabled) != 0) {
        return -1;
    }

    clock->use_system_time = enabled != 0;
    return 0;
}

/**
 * @brief Get use system time of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return -1 on failure
 */

int picoui_clock_get_use_system_time(const struct picoui_clock *clock)
{
    int enabled = 0;

    if (clock == 0) {
        return -1;
    }

    if (picoui_backend_clock_get_use_system_time((struct picoui_clock *)clock, &enabled) != 0) {
        return -1;
    }

    ((struct picoui_clock *)clock)->use_system_time = enabled;
    return enabled;
}

/**
 * @brief Create clock widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_clock *picoui_clock_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_clock_props *props)
{
    struct picoui_clock *clock;

    if (!picoui_clock_props_are_valid(props)) {
        return 0;
    }

    clock = picoui_clock_create(parent, props->id);
    if (clock == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&clock->widget, props->style_class) != 0) {
        free(clock);
        return 0;
    }
    if (picoui_widget_set_user_data(&clock->widget, props->user_data) != 0
        || picoui_clock_set_step_second(clock, props->step_second) != 0
        || (props->background_source != 0
            && picoui_clock_set_background_source(clock, props->background_source) != 0)
        || (props->hour_pointer_source != 0
            && picoui_clock_set_hour_pointer_source(clock, props->hour_pointer_source) != 0)
        || (props->minute_pointer_source != 0
            && picoui_clock_set_minute_pointer_source(clock, props->minute_pointer_source) != 0)
        || (props->second_pointer_source != 0
            && picoui_clock_set_second_pointer_source(clock, props->second_pointer_source) != 0)
        || picoui_clock_set_mask_color(clock, props->mask_color) != 0
        || picoui_clock_set_hour_anchor(clock, props->hour_anchor_x, props->hour_anchor_y) != 0
        || picoui_clock_set_minute_anchor(clock, props->minute_anchor_x, props->minute_anchor_y) != 0
        || picoui_clock_set_second_anchor(clock, props->second_anchor_x, props->second_anchor_y) != 0) {
        free(clock);
        return 0;
    }

    return clock;
}

/**
 * @brief Set step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] step_second step second
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_step_second(struct picoui_clock *clock, int step_second)
{
    if (clock == 0 || (step_second != 0 && step_second != 1)) {
        return -1;
    }

    if (picoui_backend_clock_set_step_second(clock, step_second) != 0) {
        return -1;
    }

    clock->step_second = step_second;
    return 0;
}

/**
 * @brief Get step second of clock widget
 *
 * @param[in] clock Clock widget instance
 * @return -1 on failure
 */

int picoui_clock_get_step_second(const struct picoui_clock *clock)
{
    int step_second = 0;

    if (clock == 0) {
        return -1;
    }

    if (picoui_backend_clock_get_step_second((struct picoui_clock *)clock, &step_second) != 0) {
        return -1;
    }

    return step_second;
}

/**
 * @brief Set background source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_background_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_background_source(clock, source) != 0) {
        return -1;
    }

    clock->background_source = source;
    return 0;
}

/**
 * @brief Set background image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_background_image(struct picoui_clock *clock, struct picoui_image_source *source)
{
    return picoui_clock_set_background_source(clock, source);
}

/**
 * @brief Set hour pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_hour_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_hour_pointer_source(clock, source) != 0) {
        return -1;
    }

    clock->hour_pointer_source = source;
    return 0;
}

/**
 * @brief Set hour pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_hour_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source)
{
    return picoui_clock_set_hour_pointer_source(clock, source);
}

/**
 * @brief Set minute pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_minute_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_minute_pointer_source(clock, source) != 0) {
        return -1;
    }

    clock->minute_pointer_source = source;
    return 0;
}

/**
 * @brief Set minute pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_minute_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source)
{
    return picoui_clock_set_minute_pointer_source(clock, source);
}

/**
 * @brief Set second pointer source of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_second_pointer_source(struct picoui_clock *clock, struct picoui_image_source *source)
{
    if (clock == 0 || source == 0 || source->img_tile == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_second_pointer_source(clock, source) != 0) {
        return -1;
    }

    clock->second_pointer_source = source;
    return 0;
}

/**
 * @brief Set second pointer image of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_second_pointer_image(struct picoui_clock *clock, struct picoui_image_source *source)
{
    return picoui_clock_set_second_pointer_source(clock, source);
}

/**
 * @brief Set mask color of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_mask_color(struct picoui_clock *clock, unsigned int mask_color)
{
    if (clock == 0 || mask_color > 0xFFFFFFU) {
        return -1;
    }

    if (picoui_backend_clock_set_mask_color(clock, mask_color) != 0) {
        return -1;
    }

    clock->mask_color = mask_color;
    return 0;
}

/**
 * @brief Set hour anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_hour_anchor(struct picoui_clock *clock, float x, float y)
{
    if (clock == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_hour_anchor(clock, x, y) != 0) {
        return -1;
    }

    clock->hour_anchor_x = x;
    clock->hour_anchor_y = y;
    return 0;
}

/**
 * @brief Set minute anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_minute_anchor(struct picoui_clock *clock, float x, float y)
{
    if (clock == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_minute_anchor(clock, x, y) != 0) {
        return -1;
    }

    clock->minute_anchor_x = x;
    clock->minute_anchor_y = y;
    return 0;
}

/**
 * @brief Set second anchor of clock widget
 *
 * @param[in] clock Clock widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_clock_set_second_anchor(struct picoui_clock *clock, float x, float y)
{
    if (clock == 0) {
        return -1;
    }

    if (picoui_backend_clock_set_second_anchor(clock, x, y) != 0) {
        return -1;
    }

    clock->second_anchor_x = x;
    clock->second_anchor_y = y;
    return 0;
}
