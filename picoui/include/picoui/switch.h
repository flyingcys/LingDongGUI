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

#ifndef PICOUI_SWITCH_H
#define PICOUI_SWITCH_H

#include "picoui/widget.h"
#include "picoui/window.h"

struct picoui_window;
struct picoui_switch;
struct picoui_image_source;

struct picoui_switch_props {
    const char *id;
    int checked;
    picoui_value_changed_cb on_toggled;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    struct picoui_image_source *off_source;
    struct picoui_image_source *on_source;
    struct picoui_image_source *knob_source;
    int horizontal;
    int direction;
    int disabled;

    int has_off_source;
    int has_on_source;
    int has_knob_source;
    int has_horizontal;
    int has_direction;
    int has_disabled;
};

/**
 * @brief Create switch widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_switch *picoui_switch_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create switch widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_switch *picoui_switch_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_switch_props *props);

/**
 * @brief Set checked of switch widget
 *
 * @param[in] sw sw
 * @param[in] checked Checked state
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_checked(struct picoui_switch *sw, int checked);

/**
 * @brief switch is checked
 *
 * @param[in] sw sw
 * @return 0 on success, -1 on failure
 */

int picoui_switch_is_checked(struct picoui_switch *sw);

/**
 * @brief Set off source of switch widget
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_off_source(struct picoui_switch *sw, struct picoui_image_source *source);

/**
 * @brief Set on source of switch widget
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_on_source(struct picoui_switch *sw, struct picoui_image_source *source);

/**
 * @brief Set knob source of switch widget
 *
 * @param[in] sw sw
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_knob_source(struct picoui_switch *sw, struct picoui_image_source *source);

/**
 * @brief Set horizontal of switch widget
 *
 * @param[in] sw sw
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_horizontal(struct picoui_switch *sw, int horizontal);

/**
 * @brief Get horizontal of switch widget
 *
 * @param[out] sw sw
 * @param[in] horizontal horizontal
 * @return The property value, negative on error
 */

int picoui_switch_get_horizontal(struct picoui_switch *sw, int *horizontal);

/**
 * @brief Set direction of switch widget
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_direction(struct picoui_switch *sw, int direction);

/**
 * @brief Get direction of switch widget
 *
 * @param[out] sw sw
 * @param[in] direction direction
 * @return The property value, negative on error
 */

int picoui_switch_get_direction(struct picoui_switch *sw, int *direction);

/**
 * @brief Set disabled of switch widget
 *
 * @param[in] sw sw
 * @param[in] disabled disabled
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_disabled(struct picoui_switch *sw, int disabled);

/**
 * @brief Get disabled of switch widget
 *
 * @param[out] sw sw
 * @param[in] disabled disabled
 * @return The property value, negative on error
 */

int picoui_switch_get_disabled(struct picoui_switch *sw, int *disabled);

/**
 * @brief switch can navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @param[in] can_navigate can navigate
 * @return 0 on success, -1 on failure
 */

int picoui_switch_can_navigate(struct picoui_switch *sw, int direction, int *can_navigate);

/**
 * @brief switch navigate
 *
 * @param[in] sw sw
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_switch_navigate(struct picoui_switch *sw, int direction);

/**
 * @brief Set on toggled of switch widget
 *
 * @param[in] sw sw
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_switch_set_on_toggled(struct picoui_switch *sw,
                                 picoui_value_changed_cb cb,
                                 void *user_data);

static inline tinyui_obj_t *tinyui_switch_create(tinyui_obj_t *parent, const char *id)
{
    return (tinyui_obj_t *)picoui_switch_create((struct picoui_window *)parent, id);
}

#endif
