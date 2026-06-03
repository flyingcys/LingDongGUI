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

#ifndef PICOUI_ANIMATION_H
#define PICOUI_ANIMATION_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_animation;
struct picoui_image_source;

struct picoui_animation_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int period_ms;
    struct picoui_image_source *source;
};

/**
 * @brief Create animation widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_animation *picoui_animation_create(struct picoui_widget *parent, const char *id);

/**
 * @brief animation init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_animation *picoui_animation_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Create animation widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_animation *picoui_animation_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_animation_props *props);

/**
 * @brief Set source of animation widget
 *
 * @param[in] animation Animation widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_animation_set_source(struct picoui_animation *animation, struct picoui_image_source *source);

/**
 * @brief Set period ms of animation widget
 *
 * @param[in] animation Animation widget instance
 * @param[in] period_ms Period in milliseconds
 * @return 0 on success, -1 on failure
 */

int picoui_animation_set_period_ms(struct picoui_animation *animation, int period_ms);

/**
 * @brief animation show frame
 *
 * @param[in] animation Animation widget instance
 * @param[in] frame_index Frame index
 * @return 0 on success, -1 on failure
 */

int picoui_animation_show_frame(struct picoui_animation *animation, int frame_index);

#endif
