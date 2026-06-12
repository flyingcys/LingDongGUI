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

#ifndef PICOUI_KEYBOARD_H
#define PICOUI_KEYBOARD_H

#include "../widget.h"

struct picoui_window;
struct picoui_keyboard;

struct picoui_keyboard_button {
    int x;
    int y;
    int width;
    int height;
    const char *text;
    unsigned int key_code;
    unsigned int press_color;
    unsigned int release_color;
};

typedef void (*picoui_keyboard_event_cb)(struct picoui_keyboard *keyboard,
                                         unsigned int key_code,
                                         enum picoui_native_signal signal,
                                         void *user_data);
typedef void (*picoui_keyboard_draw_cb)(struct picoui_keyboard *keyboard,
                                        const struct picoui_keyboard_button *button,
                                        void *user_data);

struct picoui_keyboard_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

/**
 * @brief Create keyboard widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_keyboard *picoui_keyboard_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create keyboard widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_keyboard *picoui_keyboard_create_with_props(struct picoui_window *parent,
                                                          const struct picoui_keyboard_props *props);

/**
 * @brief keyboard input ascii
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] ascii ascii
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_input_ascii(struct picoui_keyboard *keyboard, unsigned int ascii);

/**
 * @brief keyboard navigate
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] direction direction
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_navigate(struct picoui_keyboard *keyboard, int direction);

/**
 * @brief keyboard update
 *
 * @param[in] keyboard Keyboard widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_update(struct picoui_keyboard *keyboard);

/**
 * @brief keyboard button update
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] key_code key code
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_button_update(struct picoui_keyboard *keyboard, unsigned int key_code);

/**
 * @brief keyboard click
 *
 * @param[in] keyboard Keyboard widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_click(struct picoui_keyboard *keyboard);

/**
 * @brief keyboard exit
 *
 * @param[in] keyboard Keyboard widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_exit(struct picoui_keyboard *keyboard);

/**
 * @brief Set buttons of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_buttons(struct picoui_keyboard *keyboard,
                                const struct picoui_keyboard_button *buttons,
                                int count);

/**
 * @brief Get buttons of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return The property value, negative on error
 */

int picoui_keyboard_get_buttons(const struct picoui_keyboard *keyboard,
                                const struct picoui_keyboard_button **buttons,
                                int *count);

/**
 * @brief Set on key event of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_on_key_event(struct picoui_keyboard *keyboard,
                                     picoui_keyboard_event_cb cb,
                                     void *user_data);

/**
 * @brief Get selected key code of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @return The property value, negative on error
 */

int picoui_keyboard_get_selected_key_code(const struct picoui_keyboard *keyboard);

/**
 * @brief Set layout of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] buttons buttons
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_layout(struct picoui_keyboard *keyboard,
                               const struct picoui_keyboard_button *buttons,
                               int count);

/**
 * @brief Set event callback of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_event_callback(struct picoui_keyboard *keyboard,
                                       picoui_keyboard_event_cb cb,
                                       void *user_data);

/**
 * @brief Set draw callback of keyboard widget
 *
 * @param[in] keyboard Keyboard widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_keyboard_set_draw_callback(struct picoui_keyboard *keyboard,
                                      picoui_keyboard_draw_cb cb,
                                      void *user_data);

#endif
