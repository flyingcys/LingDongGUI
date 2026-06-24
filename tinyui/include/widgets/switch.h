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

#ifndef TINYUI_SWITCH_H
#define TINYUI_SWITCH_H

#include "core/obj.h"

struct tinyui_window;
struct tinyui_switch;
struct tinyui_image_source;

struct tinyui_switch_props {
    const char *id;
    int checked;
    tinyui_value_changed_cb on_toggled;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    /* Sentinel default: NULL = unset (no image source applied) */
    struct tinyui_image_source *off_source;
    /* Sentinel default: NULL = unset (no image source applied) */
    struct tinyui_image_source *on_source;
    /* Sentinel default: NULL = unset (no image source applied) */
    struct tinyui_image_source *knob_source;
    /* Sentinel default: -1 = unset (use backend default), 0 = horizontal off, 1 = horizontal on */
    int horizontal;
    /* Sentinel default: -1 = unset (use backend default direction) */
    int direction;
    /* Sentinel default: -1 = unset (use backend default enabled state) */
    int disabled;
};

struct tinyui_switch *tinyui_switch_create(struct tinyui_window *parent, const char *id);

struct tinyui_switch *tinyui_switch_create_with_props(struct tinyui_window *parent,
                                                      const struct tinyui_switch_props *props);

int tinyui_switch_set_checked(struct tinyui_switch *sw, int checked);

int tinyui_switch_is_checked(struct tinyui_switch *sw);

int tinyui_switch_set_off_source(struct tinyui_switch *sw, struct tinyui_image_source *source);

int tinyui_switch_set_on_source(struct tinyui_switch *sw, struct tinyui_image_source *source);

int tinyui_switch_set_knob_source(struct tinyui_switch *sw, struct tinyui_image_source *source);

int tinyui_switch_set_horizontal(struct tinyui_switch *sw, int horizontal);

int tinyui_switch_get_horizontal(struct tinyui_switch *sw, int *horizontal);

int tinyui_switch_set_direction(struct tinyui_switch *sw, int direction);

int tinyui_switch_get_direction(struct tinyui_switch *sw, int *direction);

int tinyui_switch_set_disabled(struct tinyui_switch *sw, int disabled);

int tinyui_switch_get_disabled(struct tinyui_switch *sw, int *disabled);

int tinyui_switch_can_navigate(struct tinyui_switch *sw, int direction, int *can_navigate);

int tinyui_switch_navigate(struct tinyui_switch *sw, int direction);

int tinyui_switch_set_on_toggled(struct tinyui_switch *sw,
                                 tinyui_value_changed_cb cb,
                                 void *user_data);

#endif
