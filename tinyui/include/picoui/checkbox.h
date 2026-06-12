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

#ifndef PICOUI_CHECKBOX_H
#define PICOUI_CHECKBOX_H

#include "../widget.h"

struct picoui_window;
struct picoui_checkbox;
struct picoui_image_source;

struct picoui_checkbox_props {
    const char *id;
    const char *text;
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
    unsigned int check_color;
    struct picoui_image_source *unchecked_source;
    struct picoui_image_source *checked_source;
    int radio_group;
    int string_left_space;

    int has_check_color;
    int has_unchecked_source;
    int has_checked_source;
    int has_radio_group;
    int has_string_left_space;
};

/**
 * @brief Create checkbox widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_checkbox *picoui_checkbox_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create checkbox widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_checkbox *picoui_checkbox_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_checkbox_props *props);

/**
 * @brief Set checked of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] checked Checked state
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked);

/**
 * @brief checkbox is checked
 *
 * @param[in] checkbox Checkbox widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_is_checked(struct picoui_checkbox *checkbox);

/**
 * @brief Set text of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_text(struct picoui_checkbox *checkbox, const char *text);

/**
 * @brief Set check color of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_check_color(struct picoui_checkbox *checkbox, unsigned int rgb);

/**
 * @brief Set text color of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_text_color(struct picoui_checkbox *checkbox, unsigned int rgb);

/**
 * @brief Set unchecked source of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_unchecked_source(struct picoui_checkbox *checkbox,
                                         struct picoui_image_source *source);

/**
 * @brief Set checked source of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_checked_source(struct picoui_checkbox *checkbox,
                                       struct picoui_image_source *source);

/**
 * @brief Set radio group of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] radio_group Radio button group ID
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_radio_group(struct picoui_checkbox *checkbox, int radio_group);

/**
 * @brief Set string left space of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] space Spacing
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_string_left_space(struct picoui_checkbox *checkbox, int space);

/**
 * @brief Set on toggled of checkbox widget
 *
 * @param[in] checkbox Checkbox widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_checkbox_set_on_toggled(struct picoui_checkbox *checkbox,
                                   picoui_value_changed_cb cb,
                                   void *user_data);

#endif
