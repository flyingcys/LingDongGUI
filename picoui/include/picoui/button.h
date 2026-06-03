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

#ifndef PICOUI_BUTTON_H
#define PICOUI_BUTTON_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_button;
struct picoui_image_source;

enum picoui_button_action_state {
    PICOUI_BUTTON_ACTION_PRESS = 1,
    PICOUI_BUTTON_ACTION_HOLD_DOWN = 2,
    PICOUI_BUTTON_ACTION_RELEASE = 3,
    PICOUI_BUTTON_ACTION_CLICK = 4,
    PICOUI_BUTTON_ACTION_DOUBLE_CLICK = 5,
    PICOUI_BUTTON_ACTION_REPEAT_COUNT = 6,
    PICOUI_BUTTON_ACTION_HOLD_TIME = 7,
    PICOUI_BUTTON_ACTION_LONG_START = 8,
    PICOUI_BUTTON_ACTION_LONG_SHOOT = 9,
};

struct picoui_button_props {
    const char *id;
    const char *text;
    const struct picoui_font *font;
    struct picoui_image_source *release_image;
    struct picoui_image_source *press_image;
    int transparent;
    int checkable;
    unsigned int key_value;
    int pressed;
    int width;
    int height;
    picoui_event_cb on_clicked;
    void *user_data;
    const char *style_class;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

/**
 * @brief Create button widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create button widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_button *picoui_button_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_button_props *props);

/**
 * @brief button init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_button *picoui_button_init(struct picoui_window *parent, const char *id);

/**
 * @brief Set text of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_text(struct picoui_button *button, const char *text);

/**
 * @brief Get text of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] text Text widget instance
 * @return The property value, negative on error
 */

int picoui_button_get_text(struct picoui_button *button, const char **text);

/**
 * @brief Set font of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_font(struct picoui_button *button, const struct picoui_font *font);

/**
 * @brief Get font of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] font font
 * @return The property value, negative on error
 */

int picoui_button_get_font(struct picoui_button *button, const struct picoui_font **font);

/**
 * @brief Set color of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] release_color release color
 * @param[in] press_color press color
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_color(struct picoui_button *button,
                            unsigned int release_color,
                            unsigned int press_color);

/**
 * @brief Get release color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_button_get_release_color(struct picoui_button *button, unsigned int *rgb);

/**
 * @brief Get press color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_button_get_press_color(struct picoui_button *button, unsigned int *rgb);

/**
 * @brief Set release image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_release_image(struct picoui_button *button,
                                    struct picoui_image_source *source);

/**
 * @brief Set press image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_press_image(struct picoui_button *button,
                                  struct picoui_image_source *source);

/**
 * @brief Set image of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] release_source release source
 * @param[in] press_source press source
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_image(struct picoui_button *button,
                            struct picoui_image_source *release_source,
                            struct picoui_image_source *press_source);

/**
 * @brief Set transparent of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_transparent(struct picoui_button *button, int transparent);

/**
 * @brief Get transparent of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] transparent transparent
 * @return The property value, negative on error
 */

int picoui_button_get_transparent(struct picoui_button *button, int *transparent);

/**
 * @brief Set checkable of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] checkable checkable
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_checkable(struct picoui_button *button, int checkable);

/**
 * @brief Get checkable of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] checkable checkable
 * @return The property value, negative on error
 */

int picoui_button_get_checkable(struct picoui_button *button, int *checkable);

/**
 * @brief Set key value of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] key_value key value
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_key_value(struct picoui_button *button, unsigned int key_value);

/**
 * @brief Get key value of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] key_value key value
 * @return The property value, negative on error
 */

int picoui_button_get_key_value(struct picoui_button *button, unsigned int *key_value);

/**
 * @brief Set press of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] pressed Pressed state
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_press(struct picoui_button *button, int pressed);

/**
 * @brief Set pressed of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] pressed Pressed state
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_pressed(struct picoui_button *button, int pressed);

/**
 * @brief Get press of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] pressed Pressed state
 * @return The property value, negative on error
 */

int picoui_button_get_press(struct picoui_button *button, int *pressed);

/**
 * @brief Get pressed of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] pressed Pressed state
 * @return The property value, negative on error
 */

int picoui_button_get_pressed(struct picoui_button *button, int *pressed);

/**
 * @brief Get pressed by name id of button widget
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @param[in] pressed Pressed state
 * @return The property value, negative on error
 */

int picoui_button_get_pressed_by_name_id(const struct picoui_widget *root,
                                         int name_id,
                                         int *pressed);

/**
 * @brief Get action state by name id of button widget
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @param[in] action action
 * @return The property value, negative on error
 */

int picoui_button_get_action_state_by_name_id(const struct picoui_widget *root,
                                              int name_id,
                                              enum picoui_button_action_state action);

/**
 * @brief Set text color of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] text_color Text color
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_text_color(struct picoui_button *button, unsigned int text_color);

/**
 * @brief Get text color of button widget
 *
 * @param[out] button Button widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_button_get_text_color(struct picoui_button *button, unsigned int *rgb);

/**
 * @brief Set on clicked of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_on_clicked(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data);

/**
 * @brief Set on pressed of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_on_pressed(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data);

/**
 * @brief Set on released of button widget
 *
 * @param[in] button Button widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_button_set_on_released(struct picoui_button *button,
                                  picoui_event_cb cb,
                                  void *user_data);

#endif
