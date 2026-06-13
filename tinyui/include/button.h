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

#ifndef TINYUI_BUTTON_H
#define TINYUI_BUTTON_H

#include "obj.h"

struct tinyui_window;
struct tinyui_button;
struct tinyui_image_source;

enum tinyui_button_action_state {
    TINYUI_BUTTON_ACTION_PRESS = 1,
    TINYUI_BUTTON_ACTION_HOLD_DOWN = 2,
    TINYUI_BUTTON_ACTION_RELEASE = 3,
    TINYUI_BUTTON_ACTION_CLICK = 4,
    TINYUI_BUTTON_ACTION_DOUBLE_CLICK = 5,
    TINYUI_BUTTON_ACTION_REPEAT_COUNT = 6,
    TINYUI_BUTTON_ACTION_HOLD_TIME = 7,
    TINYUI_BUTTON_ACTION_LONG_START = 8,
    TINYUI_BUTTON_ACTION_LONG_SHOOT = 9,
};

struct tinyui_button_props {
    const char *id;
    const char *text;
    const struct tinyui_font *font;
    struct tinyui_image_source *release_image;
    struct tinyui_image_source *press_image;
    int transparent;
    int checkable;
    unsigned int key_value;
    int pressed;
    int width;
    int height;
    tinyui_event_cb on_clicked;
    void *user_data;
    const char *style_class;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
};

struct tinyui_button *tinyui_button_create(struct tinyui_window *parent, const char *id);

struct tinyui_button *tinyui_button_create_with_props(struct tinyui_window *parent,
                                                      const struct tinyui_button_props *props);

struct tinyui_button *tinyui_button_init(struct tinyui_window *parent, const char *id);

int tinyui_button_set_text(struct tinyui_button *button, const char *text);

int tinyui_button_get_text(struct tinyui_button *button, const char **text);

int tinyui_button_set_font(struct tinyui_button *button, const struct tinyui_font *font);

int tinyui_button_get_font(struct tinyui_button *button, const struct tinyui_font **font);

int tinyui_button_set_color(struct tinyui_button *button,
                            unsigned int release_color,
                            unsigned int press_color);

int tinyui_button_get_release_color(struct tinyui_button *button, unsigned int *rgb);

int tinyui_button_get_press_color(struct tinyui_button *button, unsigned int *rgb);

int tinyui_button_set_text_color(struct tinyui_button *button, unsigned int text_color);

int tinyui_button_get_text_color(struct tinyui_button *button, unsigned int *rgb);

int tinyui_button_set_release_image(struct tinyui_button *button,
                                    struct tinyui_image_source *source);

int tinyui_button_set_press_image(struct tinyui_button *button,
                                  struct tinyui_image_source *source);

int tinyui_button_set_image(struct tinyui_button *button,
                            struct tinyui_image_source *release_source,
                            struct tinyui_image_source *press_source);

int tinyui_button_set_transparent(struct tinyui_button *button, int transparent);

int tinyui_button_get_transparent(struct tinyui_button *button, int *transparent);

int tinyui_button_set_checkable(struct tinyui_button *button, int checkable);

int tinyui_button_get_checkable(struct tinyui_button *button, int *checkable);

int tinyui_button_set_key_value(struct tinyui_button *button, unsigned int key_value);

int tinyui_button_get_key_value(struct tinyui_button *button, unsigned int *key_value);

int tinyui_button_set_press(struct tinyui_button *button, int pressed);

int tinyui_button_set_pressed(struct tinyui_button *button, int pressed);

int tinyui_button_get_press(struct tinyui_button *button, int *pressed);

int tinyui_button_get_pressed(struct tinyui_button *button, int *pressed);

int tinyui_button_get_pressed_by_name_id(const struct tinyui_widget *root,
                                         int name_id,
                                         int *pressed);

int tinyui_button_get_action_state_by_name_id(const struct tinyui_widget *root,
                                              int name_id,
                                              enum tinyui_button_action_state action);

int tinyui_button_set_on_clicked(struct tinyui_button *button,
                                 tinyui_event_cb cb,
                                 void *user_data);

int tinyui_button_set_on_pressed(struct tinyui_button *button,
                                 tinyui_event_cb cb,
                                 void *user_data);

int tinyui_button_set_on_released(struct tinyui_button *button,
                                  tinyui_event_cb cb,
                                  void *user_data);

#endif
