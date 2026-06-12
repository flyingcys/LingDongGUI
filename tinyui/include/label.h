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

#ifndef TINYUI_LABEL_H
#define TINYUI_LABEL_H

#include "obj.h"

struct picoui_window;
struct picoui_label;
struct picoui_image_source;

struct picoui_label_props {
    const char *id;
    const char *text;
    const struct picoui_font *font;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    int transparent;
    enum picoui_align align;
    struct picoui_image_source *background_source;
};

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id);

struct picoui_label *picoui_label_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_label_props *props);

int picoui_label_set_text(struct picoui_label *label, const char *text);

const char *picoui_label_get_text(struct picoui_label *label);

int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font);

int picoui_label_set_text_color(struct picoui_label *label, unsigned int rgb);

int picoui_label_get_text_color(struct picoui_label *label, unsigned int *rgb);

int picoui_label_set_bg_color(struct picoui_label *label, unsigned int rgb);

int picoui_label_get_bg_color(struct picoui_label *label, unsigned int *rgb);

int picoui_label_set_transparent(struct picoui_label *label, int transparent);

int picoui_label_get_transparent(struct picoui_label *label, int *transparent);

int picoui_label_set_align(struct picoui_label *label, enum picoui_align align);

int picoui_label_get_align(struct picoui_label *label, enum picoui_align *align);

int picoui_label_set_background_source(struct picoui_label *label,
                                       struct picoui_image_source *source);

static inline int tinyui_label_set_text(tinyui_obj_t *label, const char *text)
{
    return picoui_label_set_text((struct picoui_label *)label, text);
}

static inline const char *tinyui_label_get_text(tinyui_obj_t *label)
{
    return picoui_label_get_text((struct picoui_label *)label);
}

static inline int tinyui_label_set_align(tinyui_obj_t *label, tinyui_align_t align)
{
    return picoui_label_set_align((struct picoui_label *)label, align);
}

#endif
