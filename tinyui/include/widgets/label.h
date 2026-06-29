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

#include "core/obj.h"

struct tinyui_window;
struct tinyui_label;
struct tinyui_image_source;

struct tinyui_label_props {
    const char *id;
    const char *text;
    const struct tinyui_font *font;
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
    enum tinyui_align align;
    struct tinyui_image_source *background_source;
};

struct tinyui_label *tinyui_label_create(struct tinyui_window *parent, const char *id);

struct tinyui_label *tinyui_label_create_with_props(struct tinyui_window *parent,
                                                    const struct tinyui_label_props *props);

int tinyui_label_set_text(struct tinyui_label *label, const char *text);

const char *tinyui_label_get_text(struct tinyui_label *label);

int tinyui_label_set_font(struct tinyui_label *label, const struct tinyui_font *font);

int tinyui_label_set_text_color(struct tinyui_label *label, unsigned int rgb);

int tinyui_label_get_text_color(struct tinyui_label *label, unsigned int *rgb);

int tinyui_label_set_bg_color(struct tinyui_label *label, unsigned int rgb);

int tinyui_label_get_bg_color(struct tinyui_label *label, unsigned int *rgb);

int tinyui_label_set_transparent(struct tinyui_label *label, int transparent);

int tinyui_label_get_transparent(struct tinyui_label *label, int *transparent);

int tinyui_label_set_align(struct tinyui_label *label, enum tinyui_align align);

int tinyui_label_get_align(struct tinyui_label *label, enum tinyui_align *align);

int tinyui_label_set_text_align(struct tinyui_label *label,
                                enum tinyui_align x_align,
                                enum tinyui_align y_align);

int tinyui_label_set_background_source(struct tinyui_label *label,
                                       struct tinyui_image_source *source);

#endif
