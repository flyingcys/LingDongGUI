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

struct picoui_button *picoui_button_create(struct picoui_window *parent, const char *id);

struct picoui_button *picoui_button_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_button_props *props);

struct picoui_button *picoui_button_init(struct picoui_window *parent, const char *id);

int picoui_button_set_text(struct picoui_button *button, const char *text);

int picoui_button_get_text(struct picoui_button *button, const char **text);

int picoui_button_set_font(struct picoui_button *button, const struct picoui_font *font);

int picoui_button_get_font(struct picoui_button *button, const struct picoui_font **font);

int picoui_button_set_color(struct picoui_button *button,
                            unsigned int release_color,
                            unsigned int press_color);

int picoui_button_get_release_color(struct picoui_button *button, unsigned int *rgb);

int picoui_button_get_press_color(struct picoui_button *button, unsigned int *rgb);

int picoui_button_set_text_color(struct picoui_button *button, unsigned int text_color);

int picoui_button_get_text_color(struct picoui_button *button, unsigned int *rgb);

int picoui_button_set_release_image(struct picoui_button *button,
                                    struct picoui_image_source *source);

int picoui_button_set_press_image(struct picoui_button *button,
                                  struct picoui_image_source *source);

int picoui_button_set_image(struct picoui_button *button,
                            struct picoui_image_source *release_source,
                            struct picoui_image_source *press_source);

int picoui_button_set_transparent(struct picoui_button *button, int transparent);

int picoui_button_get_transparent(struct picoui_button *button, int *transparent);

int picoui_button_set_checkable(struct picoui_button *button, int checkable);

int picoui_button_get_checkable(struct picoui_button *button, int *checkable);

int picoui_button_set_key_value(struct picoui_button *button, unsigned int key_value);

int picoui_button_get_key_value(struct picoui_button *button, unsigned int *key_value);

int picoui_button_set_press(struct picoui_button *button, int pressed);

int picoui_button_set_pressed(struct picoui_button *button, int pressed);

int picoui_button_get_press(struct picoui_button *button, int *pressed);

int picoui_button_get_pressed(struct picoui_button *button, int *pressed);

int picoui_button_get_pressed_by_name_id(const struct picoui_widget *root,
                                         int name_id,
                                         int *pressed);

int picoui_button_get_action_state_by_name_id(const struct picoui_widget *root,
                                              int name_id,
                                              enum picoui_button_action_state action);

int picoui_button_set_on_clicked(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data);

int picoui_button_set_on_pressed(struct picoui_button *button,
                                 picoui_event_cb cb,
                                 void *user_data);

int picoui_button_set_on_released(struct picoui_button *button,
                                  picoui_event_cb cb,
                                  void *user_data);

static inline int tinyui_button_set_text(tinyui_obj_t *button, const char *text)
{
    return picoui_button_set_text((struct picoui_button *)button, text);
}

static inline int tinyui_button_set_on_clicked(tinyui_obj_t *button,
                                               tinyui_event_cb cb,
                                               void *user_data)
{
    return picoui_button_set_on_clicked((struct picoui_button *)button, cb, user_data);
}

static inline int tinyui_button_set_on_pressed(tinyui_obj_t *button,
                                               tinyui_event_cb cb,
                                               void *user_data)
{
    return picoui_button_set_on_pressed((struct picoui_button *)button, cb, user_data);
}

static inline int tinyui_button_set_on_released(tinyui_obj_t *button,
                                                tinyui_event_cb cb,
                                                void *user_data)
{
    return picoui_button_set_on_released((struct picoui_button *)button, cb, user_data);
}

#endif
