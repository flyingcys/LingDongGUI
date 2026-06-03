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

#ifndef PICOUI_ARC_H
#define PICOUI_ARC_H

struct picoui_widget;
struct picoui_arc;
struct picoui_image_source;

struct picoui_arc_props {
    const char *id;
    const char *style_class;
    void *user_data;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    float rotation_angle;
    struct picoui_image_source *quarter_source;
    unsigned int parent_color;
    unsigned int bg_color;
    unsigned int fg_color;
};

/**
 * @brief Create arc widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_arc *picoui_arc_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create arc widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_arc *picoui_arc_create_with_props(struct picoui_widget *parent,
                                                const struct picoui_arc_props *props);

/**
 * @brief arc init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_arc *picoui_arc_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Set background angle of arc widget
 *
 * @param[in] arc Arc widget instance
 * @param[in] bg_start_angle Background arc start angle
 * @param[in] bg_end_angle Background arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_arc_set_background_angle(struct picoui_arc *arc, float bg_start_angle, float bg_end_angle);

/**
 * @brief Set foreground angle of arc widget
 *
 * @param[in] arc Arc widget instance
 * @param[in] fg_end_angle Foreground arc end angle
 * @return 0 on success, -1 on failure
 */

int picoui_arc_set_foreground_angle(struct picoui_arc *arc, float fg_end_angle);

/**
 * @brief Set rotation angle of arc widget
 *
 * @param[in] arc Arc widget instance
 * @param[in] rotation_angle Arc rotation angle
 * @return 0 on success, -1 on failure
 */

int picoui_arc_set_rotation_angle(struct picoui_arc *arc, float rotation_angle);

/**
 * @brief Set quarter source of arc widget
 *
 * @param[in] arc Arc widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_arc_set_quarter_source(struct picoui_arc *arc, struct picoui_image_source *source);

/**
 * @brief Set parent color of arc widget
 *
 * @param[in] arc Arc widget instance
 * @param[in] parent_color parent color
 * @return 0 on success, -1 on failure
 */

int picoui_arc_set_parent_color(struct picoui_arc *arc, unsigned int parent_color);

/**
 * @brief Set color of arc widget
 *
 * @param[in] arc Arc widget instance
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int picoui_arc_set_color(struct picoui_arc *arc, unsigned int bg_color, unsigned int fg_color);

/**
 * @brief Get background start angle of arc widget
 *
 * @param[in] arc Arc widget instance
 */

float picoui_arc_get_background_start_angle(const struct picoui_arc *arc);

/**
 * @brief Get background angle of arc widget
 *
 * @param[in] arc Arc widget instance
 */

float picoui_arc_get_background_angle(const struct picoui_arc *arc);

/**
 * @brief Get foreground angle of arc widget
 *
 * @param[in] arc Arc widget instance
 */

float picoui_arc_get_foreground_angle(const struct picoui_arc *arc);

/**
 * @brief Get rotation angle of arc widget
 *
 * @param[in] arc Arc widget instance
 */

float picoui_arc_get_rotation_angle(const struct picoui_arc *arc);

/**
 * @brief Get background color of arc widget
 *
 * @param[in] arc Arc widget instance
 */

unsigned int picoui_arc_get_background_color(const struct picoui_arc *arc);

/**
 * @brief Get foreground color of arc widget
 *
 * @param[in] arc Arc widget instance
 */

unsigned int picoui_arc_get_foreground_color(const struct picoui_arc *arc);

#endif
