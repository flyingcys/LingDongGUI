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

#ifndef TINYUI_CANVAS_H
#define TINYUI_CANVAS_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_CANVAS
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_CANVAS is disabled"
#  endif
#endif

#include "layout/layout.h"

#include "core/obj.h"
#include <stdint.h>
#include "widgets/image.h"
#include "theme/theme.h"

struct tinyui_image_source;

typedef enum tinyui_canvas_field {
    TINYUI_CANVAS_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_CANVAS_FIELD_WIDTH = UINT32_C(1) << 1,
    TINYUI_CANVAS_FIELD_HEIGHT = UINT32_C(1) << 2,
    TINYUI_CANVAS_FIELD_USER_DATA = UINT32_C(1) << 3,
    TINYUI_CANVAS_FIELD_STYLE_CLASS = UINT32_C(1) << 4,
} tinyui_canvas_field_t;

typedef struct tinyui_canvas_props {
    uint32_t fields;
    uint16_t id;
    int width;
    int height;
    void *user_data;
    const char *style_class;
} tinyui_canvas_props_t;

tinyui_obj_t *tinyui_canvas_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_canvas_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_canvas_props_t *props);

int tinyui_canvas_clear(tinyui_obj_t *canvas);

int tinyui_canvas_fill_rect(tinyui_obj_t *canvas, int x, int y, int width, int height, unsigned int rgb, int opacity);

int tinyui_canvas_draw_line(tinyui_obj_t *canvas, int x0, int y0, int x1, int y1, int line_size, unsigned int rgb, int opacity_max, int opacity_min);

int tinyui_canvas_draw_image(tinyui_obj_t *canvas, int x, int y, int width, int height, struct tinyui_image_source *source, unsigned int mask_color, int opacity);

int tinyui_canvas_draw_image_scaled(tinyui_obj_t *canvas, int x, int y, int width, int height, struct tinyui_image_source *source, float scale, int opacity);

int tinyui_canvas_draw_text(tinyui_obj_t *canvas, int x, int y, int width, int height, const char *text, enum tinyui_align align, unsigned int text_color, int opacity);

int tinyui_canvas_get_command_count(const tinyui_obj_t *canvas, int *count);

#endif /* TINYUI_CANVAS_H */
