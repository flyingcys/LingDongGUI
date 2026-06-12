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

#include "obj.h"

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

struct picoui_switch *picoui_switch_create(struct picoui_window *parent, const char *id);

struct picoui_switch *picoui_switch_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_switch_props *props);

int picoui_switch_set_checked(struct picoui_switch *sw, int checked);

int picoui_switch_is_checked(struct picoui_switch *sw);

int picoui_switch_set_off_source(struct picoui_switch *sw, struct picoui_image_source *source);

int picoui_switch_set_on_source(struct picoui_switch *sw, struct picoui_image_source *source);

int picoui_switch_set_knob_source(struct picoui_switch *sw, struct picoui_image_source *source);

int picoui_switch_set_horizontal(struct picoui_switch *sw, int horizontal);

int picoui_switch_get_horizontal(struct picoui_switch *sw, int *horizontal);

int picoui_switch_set_direction(struct picoui_switch *sw, int direction);

int picoui_switch_get_direction(struct picoui_switch *sw, int *direction);

int picoui_switch_set_disabled(struct picoui_switch *sw, int disabled);

int picoui_switch_get_disabled(struct picoui_switch *sw, int *disabled);

int picoui_switch_can_navigate(struct picoui_switch *sw, int direction, int *can_navigate);

int picoui_switch_navigate(struct picoui_switch *sw, int direction);

int picoui_switch_set_on_toggled(struct picoui_switch *sw,
                                 picoui_value_changed_cb cb,
                                 void *user_data);

static inline tinyui_obj_t *tinyui_switch_create(tinyui_obj_t *parent, const char *id)
{
    return (tinyui_obj_t *)picoui_switch_create((struct picoui_window *)parent, id);
}

static inline int tinyui_switch_set_checked(tinyui_obj_t *sw, int checked)
{
    return picoui_switch_set_checked((struct picoui_switch *)sw, checked);
}

static inline int tinyui_switch_is_checked(tinyui_obj_t *sw)
{
    return picoui_switch_is_checked((struct picoui_switch *)sw);
}

static inline int tinyui_switch_set_on_toggled(tinyui_obj_t *sw,
                                               tinyui_value_changed_cb cb,
                                               void *user_data)
{
    return picoui_switch_set_on_toggled((struct picoui_switch *)sw, cb, user_data);
}

#endif
