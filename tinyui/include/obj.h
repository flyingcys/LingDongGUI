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

#ifndef TINYUI_OBJ_H
#define TINYUI_OBJ_H

#include "widget.h"

typedef struct tinyui_widget tinyui_obj_t;
typedef enum tinyui_align tinyui_align_t;
typedef tinyui_event_cb tinyui_event_cb;
typedef tinyui_value_changed_cb tinyui_value_changed_cb;

static inline int tinyui_obj_set_pos(tinyui_obj_t *obj, int x, int y)
{
    return tinyui_widget_set_pos(obj, x, y);
}

static inline int tinyui_obj_set_size(tinyui_obj_t *obj, int width, int height)
{
    return tinyui_widget_set_size(obj, width, height);
}

static inline int tinyui_obj_set_text(tinyui_obj_t *obj, const char *text)
{
    return tinyui_widget_set_text(obj, text);
}

static inline int tinyui_obj_set_style_class(tinyui_obj_t *obj, const char *style_class)
{
    return tinyui_widget_set_style_class(obj, style_class);
}

static inline int tinyui_obj_set_user_data(tinyui_obj_t *obj, void *user_data)
{
    return tinyui_widget_set_user_data(obj, user_data);
}

static inline int tinyui_obj_set_bg_color(tinyui_obj_t *obj, unsigned int rgb)
{
    return tinyui_widget_set_bg_color(obj, rgb);
}

static inline int tinyui_obj_set_text_color(tinyui_obj_t *obj, unsigned int rgb)
{
    return tinyui_widget_set_text_color(obj, rgb);
}

static inline int tinyui_obj_set_border_color(tinyui_obj_t *obj, unsigned int rgb)
{
    return tinyui_widget_set_border_color(obj, rgb);
}

static inline int tinyui_obj_set_radius(tinyui_obj_t *obj, int radius)
{
    return tinyui_widget_set_radius(obj, radius);
}

static inline int tinyui_obj_set_padding(tinyui_obj_t *obj, int padding)
{
    return tinyui_widget_set_padding(obj, padding);
}

static inline int tinyui_obj_set_center(tinyui_obj_t *obj)
{
    return tinyui_widget_set_center(obj);
}

static inline int tinyui_obj_set_visible(tinyui_obj_t *obj, int visible)
{
    return tinyui_widget_set_visible(obj, visible);
}

static inline int tinyui_obj_destroy(tinyui_obj_t *obj)
{
    return tinyui_widget_destroy(obj);
}

#endif
