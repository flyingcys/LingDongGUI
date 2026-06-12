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

#ifndef PICOUI_LINE_EDIT_H
#define PICOUI_LINE_EDIT_H

#include "../layout.h"
#include "../widget.h"

struct picoui_window;
struct picoui_line_edit;
typedef void (*picoui_line_edit_finished_cb)(struct picoui_line_edit *line_edit, void *user_data);

enum picoui_line_edit_type {
    PICOUI_LINE_EDIT_TYPE_STRING = 0,
    PICOUI_LINE_EDIT_TYPE_INT,
    PICOUI_LINE_EDIT_TYPE_FLOAT,
};

struct picoui_line_edit_props {
    const char *id;
    const char *text;
    enum picoui_line_edit_type type;
    unsigned int keyboard_binding;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    int has_type;
    int has_keyboard_binding;
};

/**
 * @brief Create line edit widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_line_edit *picoui_line_edit_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create line edit widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_line_edit *picoui_line_edit_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_line_edit_props *props);

/**
 * @brief Set text of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_text(struct picoui_line_edit *line_edit, const char *text);

/**
 * @brief Get text of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 */

const char *picoui_line_edit_get_text(const struct picoui_line_edit *line_edit);

/**
 * @brief Set align of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_align(struct picoui_line_edit *line_edit, enum picoui_align align);

/**
 * @brief Set color of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] text_color Text color
 * @param[in] background_color background color
 * @param[in] frame_color frame color
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_color(struct picoui_line_edit *line_edit,
                               unsigned int text_color,
                               unsigned int background_color,
                               unsigned int frame_color);

/**
 * @brief Set type of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_type(struct picoui_line_edit *line_edit, enum picoui_line_edit_type type);

/**
 * @brief Get type of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[out] type Type
 * @return The property value, negative on error
 */

int picoui_line_edit_get_type(const struct picoui_line_edit *line_edit,
                              enum picoui_line_edit_type *type);

/**
 * @brief Set keyboard of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_keyboard(struct picoui_line_edit *line_edit, unsigned int keyboard_binding);

/**
 * @brief Set keyboard binding of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_keyboard_binding(struct picoui_line_edit *line_edit,
                                          unsigned int keyboard_binding);

/**
 * @brief Get keyboard binding of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] keyboard_binding keyboard binding
 * @return The property value, negative on error
 */

int picoui_line_edit_get_keyboard_binding(const struct picoui_line_edit *line_edit,
                                          unsigned int *keyboard_binding);

/**
 * @brief Get editing of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] editing editing
 * @return The property value, negative on error
 */

int picoui_line_edit_get_editing(const struct picoui_line_edit *line_edit, int *editing);

/**
 * @brief Set on edit finished of line edit widget
 *
 * @param[in] line_edit Line edit widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_line_edit_set_on_edit_finished(struct picoui_line_edit *line_edit,
                                          picoui_line_edit_finished_cb cb,
                                          void *user_data);

#endif
