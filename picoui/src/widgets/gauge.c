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
#include "backend.h"
#include "picoui/gauge.h"
#include "picoui/widget.h"

#include <stdlib.h>

struct picoui_gauge_ext {
    struct picoui_gauge gauge;
    int min_value;
    int max_value;
    int value;
    int tick_count;
    int tick_step;
    int render_ready;
    int rendered_min_value;
    int rendered_max_value;
    int rendered_value;
    int rendered_tick_count;
    int rendered_tick_step;
    float rendered_needle_angle;
};

int picoui_native_gauge_init_state(struct picoui_gauge *gauge);
void picoui_native_gauge_reset_render_state(struct picoui_gauge *gauge);
int picoui_native_gauge_set_state(struct picoui_gauge *gauge,
                                  int min_value,
                                  int max_value,
                                  int value,
                                  int tick_count);

static struct picoui_gauge_ext *picoui_gauge_ext_from_gauge(struct picoui_gauge *gauge)
{
    if (gauge == 0) {
        return 0;
    }

    return (struct picoui_gauge_ext *)gauge;
}

static const struct picoui_gauge_ext *picoui_gauge_ext_from_gauge_const(const struct picoui_gauge *gauge)
{
    if (gauge == 0) {
        return 0;
    }

    return (const struct picoui_gauge_ext *)gauge;
}

static int picoui_gauge_clamp_value(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static float picoui_gauge_value_to_angle(int value, int min_value, int max_value)
{
    float ratio;

    if (max_value <= min_value) {
        return 0.0f;
    }

    ratio = (float)(value - min_value) / (float)(max_value - min_value);
    return ratio * 180.0f;
}

static int picoui_gauge_props_are_valid(const struct picoui_gauge_props *props)
{
    return props != 0 && props->id != 0;
}

/**
 * @brief Create gauge widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_gauge *picoui_gauge_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_gauge *gauge;
    struct picoui_gauge_ext *ext;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    ext = calloc(1, sizeof(*ext));
    if (ext == 0) {
        return 0;
    }
    gauge = &ext->gauge;

    gauge->widget.backend_widget = picoui_backend_create_gauge(parent->backend_widget, id);
    if (gauge->widget.backend_widget == 0) {
        free(ext);
        return 0;
    }

    gauge->id = id;
    gauge->widget.visible = 1;
    gauge->widget.enabled = 1;
    if (picoui_native_gauge_init_state(gauge) != 0) {
        free(ext);
        return 0;
    }
    if (picoui_backend_widget_bind_host(gauge->widget.backend_widget, &gauge->widget) != 0
        || picoui_gauge_set_angle(gauge, 0.0f) != 0
        || picoui_gauge_set_pointer_color(gauge, 0x000000) != 0
        || picoui_gauge_set_auto_move(gauge, 0) != 0) {
        free(ext);
        return 0;
    }
    return gauge;
}

int picoui_gauge_set_range(struct picoui_gauge *gauge, int min_value, int max_value)
{
    struct picoui_gauge_ext *ext;
    int old_min_value;
    int old_max_value;
    int old_value;

    if (gauge == 0 || min_value > max_value) {
        return -1;
    }

    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0) {
        return -1;
    }

    old_min_value = ext->min_value;
    old_max_value = ext->max_value;
    old_value = ext->value;

    if (picoui_native_gauge_set_state(gauge,
                                      min_value,
                                      max_value,
                                      picoui_gauge_clamp_value(old_value, min_value, max_value),
                                      ext->tick_count) != 0) {
        return -1;
    }

    if (picoui_gauge_set_value(gauge, picoui_gauge_clamp_value(old_value, min_value, max_value)) != 0) {
        (void)picoui_native_gauge_set_state(gauge,
                                            old_min_value,
                                            old_max_value,
                                            old_value,
                                            ext->tick_count);
        return -1;
    }

    return 0;
}

int picoui_gauge_set_value(struct picoui_gauge *gauge, int value)
{
    struct picoui_gauge_ext *ext;
    struct picoui_backend_widget *backend;
    int clamped_value;
    int old_value;
    float angle;
    float old_angle;

    if (gauge == 0) {
        return -1;
    }

    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)gauge->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    old_value = ext->value;
    old_angle = gauge->angle;
    clamped_value = picoui_gauge_clamp_value(value, ext->min_value, ext->max_value);
    angle = picoui_gauge_value_to_angle(clamped_value, ext->min_value, ext->max_value);

    if (picoui_gauge_set_angle(gauge, angle) != 0) {
        return -1;
    }

    if (picoui_backend_widget_update_value(backend,
                                           clamped_value,
                                           0,
                                           &gauge->widget,
                                           0) != 0) {
        (void)picoui_gauge_set_angle(gauge, old_angle);
        return -1;
    }

    if (picoui_native_gauge_set_state(gauge,
                                      ext->min_value,
                                      ext->max_value,
                                      clamped_value,
                                      ext->tick_count) != 0) {
        (void)picoui_backend_widget_update_value(backend,
                                                 old_value,
                                                 0,
                                                 &gauge->widget,
                                                 0);
        (void)picoui_gauge_set_angle(gauge, old_angle);
        return -1;
    }

    return 0;
}

int picoui_gauge_set_tick_count(struct picoui_gauge *gauge, int tick_count)
{
    struct picoui_gauge_ext *ext;
    int old_tick_count;

    if (gauge == 0 || tick_count <= 0) {
        return -1;
    }

    ext = picoui_gauge_ext_from_gauge(gauge);
    if (ext == 0) {
        return -1;
    }

    old_tick_count = ext->tick_count;
    if (picoui_native_gauge_set_state(gauge,
                                      ext->min_value,
                                      ext->max_value,
                                      ext->value,
                                      tick_count) != 0) {
        return -1;
    }

    if (ext->tick_count != tick_count) {
        (void)picoui_native_gauge_set_state(gauge,
                                            ext->min_value,
                                            ext->max_value,
                                            ext->value,
                                            old_tick_count);
        return -1;
    }

    return 0;
}

/**
 * @brief gauge init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct picoui_gauge *picoui_gauge_init(struct picoui_widget *parent, const char *id)
{
    return picoui_gauge_create(parent, id);
}

/**
 * @brief Create gauge widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_gauge *picoui_gauge_create_with_props(struct picoui_widget *parent,
                                                    const struct picoui_gauge_props *props)
{
    struct picoui_gauge *gauge;

    if (!picoui_gauge_props_are_valid(props)) {
        return 0;
    }

    gauge = picoui_gauge_create(parent, props->id);
    if (gauge == 0) {
        return 0;
    }

    if ((props->style_class != 0
         && picoui_widget_set_style_class(&gauge->widget, props->style_class) != 0)
        || picoui_widget_set_user_data(&gauge->widget, props->user_data) != 0
        || picoui_gauge_set_angle(gauge, props->angle) != 0
        || (props->bg_source != 0 && picoui_gauge_set_bg_source(gauge, props->bg_source) != 0)
        || (props->pointer_source != 0 && picoui_gauge_set_pointer_source(gauge, props->pointer_source) != 0)
        || picoui_gauge_set_centre_offset(gauge, props->centre_offset_x, props->centre_offset_y) != 0
        || picoui_gauge_set_pointer_color(gauge, props->pointer_color) != 0
        || picoui_gauge_set_auto_move(gauge, props->auto_move) != 0) {
        free(gauge);
        return 0;
    }

    return gauge;
}

/**
 * @brief Set angle of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] angle Angle in degrees
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_angle(struct picoui_gauge *gauge, float angle)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_angle(gauge, angle) != 0) {
        return -1;
    }
    gauge->angle = angle;
    return 0;
}

/**
 * @brief Set bg source of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_bg_source(struct picoui_gauge *gauge, struct picoui_image_source *source)
{
    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_bg_source(gauge, source) != 0) {
        return -1;
    }
    gauge->bg_source = source;
    return 0;
}

/**
 * @brief Set pointer source of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_pointer_source(struct picoui_gauge *gauge, struct picoui_image_source *source)
{
    if (gauge == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_pointer_source(gauge, source) != 0) {
        return -1;
    }
    gauge->pointer_source = source;
    return 0;
}

/**
 * @brief Set centre offset of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] centre_offset_x centre offset x
 * @param[in] centre_offset_y centre offset y
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_centre_offset(struct picoui_gauge *gauge, int centre_offset_x, int centre_offset_y)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_centre_offset(gauge, centre_offset_x, centre_offset_y) != 0) {
        return -1;
    }
    gauge->centre_offset_x = centre_offset_x;
    gauge->centre_offset_y = centre_offset_y;
    return 0;
}

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
                           struct picoui_image_source *pointer_trail_source)
{
    if (gauge == 0
        || bg_trail_source == 0
        || pointer_trail_source == 0
        || bg_trail_source->mask_tile == 0
        || pointer_trail_source->mask_tile == 0) {
        return -1;
    }

    if (picoui_backend_gauge_set_trail(gauge, bg_trail_source, pointer_trail_source) != 0) {
        return -1;
    }

    return 0;
}

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
                                  struct picoui_image_source *pointer_progress_source)
{
    if (gauge == 0
        || bg_progress_source == 0
        || pointer_progress_source == 0
        || bg_progress_source->mask_tile == 0
        || pointer_progress_source->mask_tile == 0) {
        return -1;
    }

    if (picoui_backend_gauge_set_progress_bar(gauge, bg_progress_source, pointer_progress_source) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Get angle of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 */

float picoui_gauge_get_angle(const struct picoui_gauge *gauge)
{
    float angle = 0.0f;

    if (gauge == 0) {
        return 0.0f;
    }
    if (picoui_backend_gauge_get_angle((struct picoui_gauge *)gauge, &angle) != 0) {
        return 0.0f;
    }
    return angle;
}

int picoui_gauge_get_min_value(const struct picoui_gauge *gauge)
{
    const struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge_const(gauge);
    if (ext == 0) {
        return 0;
    }

    return ext->min_value;
}

int picoui_gauge_get_max_value(const struct picoui_gauge *gauge)
{
    const struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge_const(gauge);
    if (ext == 0) {
        return 0;
    }

    return ext->max_value;
}

int picoui_gauge_get_value(const struct picoui_gauge *gauge)
{
    const struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge_const(gauge);
    if (ext == 0) {
        return 0;
    }

    return ext->value;
}

int picoui_gauge_get_tick_count(const struct picoui_gauge *gauge)
{
    const struct picoui_gauge_ext *ext;

    ext = picoui_gauge_ext_from_gauge_const(gauge);
    if (ext == 0) {
        return 0;
    }

    return ext->tick_count;
}

/**
 * @brief Set pointer color of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] pointer_color pointer color
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_pointer_color(struct picoui_gauge *gauge, unsigned int pointer_color)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_pointer_color(gauge, pointer_color) != 0) {
        return -1;
    }
    gauge->pointer_color = pointer_color;
    return 0;
}

/**
 * @brief Get pointer color of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 */

unsigned int picoui_gauge_get_pointer_color(const struct picoui_gauge *gauge)
{
    unsigned int pointer_color = 0;

    if (gauge == 0) {
        return 0;
    }
    if (picoui_backend_gauge_get_pointer_color((struct picoui_gauge *)gauge, &pointer_color) != 0) {
        return 0;
    }
    return pointer_color;
}

/**
 * @brief Set auto move of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @param[in] auto_move auto move
 * @return 0 on success, -1 on failure
 */

int picoui_gauge_set_auto_move(struct picoui_gauge *gauge, int auto_move)
{
    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_set_auto_move(gauge, auto_move != 0) != 0) {
        return -1;
    }
    gauge->auto_move = auto_move != 0 ? 1 : 0;
    return 0;
}

/**
 * @brief Get auto move of gauge widget
 *
 * @param[in] gauge Gauge widget instance
 * @return -1 on failure
 */

int picoui_gauge_get_auto_move(const struct picoui_gauge *gauge)
{
    int auto_move = 0;

    if (gauge == 0) {
        return -1;
    }
    if (picoui_backend_gauge_get_auto_move((struct picoui_gauge *)gauge, &auto_move) != 0) {
        return -1;
    }
    return auto_move;
}
