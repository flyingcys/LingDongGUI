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

#ifndef PICOUI_MESSAGE_BOX_H
#define PICOUI_MESSAGE_BOX_H

#include "../widget.h"

struct picoui_message_box;

typedef void (*picoui_message_box_callback_t)(struct picoui_message_box *box, void *user_data);
typedef void (*picoui_message_box_indexed_callback_t)(struct picoui_message_box *box,
                                                      int index,
                                                      void *user_data);

struct picoui_message_box_props {
    const char *id;
    const char *style_class;
    void *user_data;
    const char *title;
    const char *message;
    const char *confirm_text;
};

/**
 * @brief Create message box widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_message_box *picoui_message_box_create(struct picoui_widget *parent, const char *id);

/**
 * @brief message box init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_message_box *picoui_message_box_init(struct picoui_widget *parent, const char *id);

/**
 * @brief Create message box widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_message_box *picoui_message_box_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_message_box_props *props);

/**
 * @brief Set title of message box widget
 *
 * @param[in] box box
 * @param[in] title title
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_title(struct picoui_message_box *box, const char *title);

/**
 * @brief Set message of message box widget
 *
 * @param[in] box box
 * @param[in] message message
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_message(struct picoui_message_box *box, const char *message);

/**
 * @brief Set msg of message box widget
 *
 * @param[in] box box
 * @param[in] message message
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_msg(struct picoui_message_box *box, const char *message);

/**
 * @brief Set confirm text of message box widget
 *
 * @param[in] box box
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_confirm_text(struct picoui_message_box *box, const char *text);

/**
 * @brief Set buttons of message box widget
 *
 * @param[in] box box
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_buttons(struct picoui_message_box *box, const char *const *buttons, int count);

/**
 * @brief Set btn of message box widget
 *
 * @param[in] box box
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_btn(struct picoui_message_box *box, const char *const *buttons, int count);

/**
 * @brief Set string colors of message box widget
 *
 * @param[in] box box
 * @param[in] title_color title color
 * @param[in] message_color message color
 * @param[in] button_color button color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_string_colors(struct picoui_message_box *box,
                                         unsigned int title_color,
                                         unsigned int message_color,
                                         unsigned int button_color);

/**
 * @brief Set string color of message box widget
 *
 * @param[in] box box
 * @param[in] title_color title color
 * @param[in] message_color message color
 * @param[in] button_color button color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_string_color(struct picoui_message_box *box,
                                        unsigned int title_color,
                                        unsigned int message_color,
                                        unsigned int button_color);

/**
 * @brief Set button colors of message box widget
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_button_colors(struct picoui_message_box *box,
                                         unsigned int release_color,
                                         unsigned int press_color);

/**
 * @brief Set button color of message box widget
 *
 * @param[in] box box
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_button_color(struct picoui_message_box *box,
                                        unsigned int release_color,
                                        unsigned int press_color);

/**
 * @brief Set bg color of message box widget
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_bg_color(struct picoui_message_box *box, unsigned int bg_color);

/**
 * @brief Set background color of message box widget
 *
 * @param[in] box box
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_message_box_set_background_color(struct picoui_message_box *box, unsigned int bg_color);

/**
 * @brief Set on confirm of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void picoui_message_box_set_on_confirm(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data);

/**
 * @brief Set callback of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void picoui_message_box_set_callback(
    struct picoui_message_box *box,
    picoui_message_box_callback_t callback,
    void *user_data);

/**
 * @brief Set on confirm indexed of message box widget
 *
 * @param[in] box box
 * @param[in] callback callback
 * @param[in] user_data User data pointer
 */

void picoui_message_box_set_on_confirm_indexed(
    struct picoui_message_box *box,
    picoui_message_box_indexed_callback_t callback,
    void *user_data);

/**
 * @brief Get title of message box widget
 *
 * @param[in] box box
 */

const char *picoui_message_box_get_title(const struct picoui_message_box *box);

/**
 * @brief Get message of message box widget
 *
 * @param[in] box box
 */

const char *picoui_message_box_get_message(const struct picoui_message_box *box);

/**
 * @brief Get confirm text of message box widget
 *
 * @param[in] box box
 */

const char *picoui_message_box_get_confirm_text(const struct picoui_message_box *box);

#endif
