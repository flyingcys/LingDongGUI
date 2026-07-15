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

#ifndef TINYUI_COMBO_BOX_H
#define TINYUI_COMBO_BOX_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_COMBO_BOX
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_COMBO_BOX is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_combo_box_field {
    TINYUI_COMBO_BOX_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_COMBO_BOX_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_COMBO_BOX_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_COMBO_BOX_FIELD_WIDTH = UINT32_C(1) << 3,
    TINYUI_COMBO_BOX_FIELD_HEIGHT = UINT32_C(1) << 4,
    TINYUI_COMBO_BOX_FIELD_BG_COLOR = UINT32_C(1) << 5,
    TINYUI_COMBO_BOX_FIELD_TEXT_COLOR = UINT32_C(1) << 6,
    TINYUI_COMBO_BOX_FIELD_BORDER_COLOR = UINT32_C(1) << 7,
    TINYUI_COMBO_BOX_FIELD_RADIUS = UINT32_C(1) << 8,
    TINYUI_COMBO_BOX_FIELD_PADDING = UINT32_C(1) << 9,
} tinyui_combo_box_field_t;

typedef struct tinyui_combo_box_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_combo_box_props_t;

tinyui_obj_t *tinyui_combo_box_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_combo_box_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_combo_box_props_t *props);

int tinyui_combo_box_add_item(tinyui_obj_t *combo_box, const char *id, const char *text);

int tinyui_combo_box_set_select_item(tinyui_obj_t *combo_box, int index);

int tinyui_combo_box_set_selected_index(tinyui_obj_t *combo_box, int index);

int tinyui_combo_box_get_select_item(const tinyui_obj_t *combo_box);

int tinyui_combo_box_get_selected_index(const tinyui_obj_t *combo_box);

const char *tinyui_combo_box_get_text(const tinyui_obj_t *combo_box, int index);

int tinyui_combo_box_set_static_items( tinyui_obj_t *combo_box, const char *const *item_ids, const char *const *texts, int item_count);

int tinyui_combo_box_is_open(const tinyui_obj_t *combo_box, int *is_open);

int tinyui_combo_box_set_text_color(tinyui_obj_t *combo_box, unsigned int rgb);

int tinyui_combo_box_set_background_color(tinyui_obj_t *combo_box, unsigned int rgb);

int tinyui_combo_box_set_bg_color(tinyui_obj_t *combo_box, unsigned int rgb);

int tinyui_combo_box_set_frame_color(tinyui_obj_t *combo_box, unsigned int rgb);

int tinyui_combo_box_set_select_color(tinyui_obj_t *combo_box, unsigned int rgb);

int tinyui_combo_box_set_item_max(tinyui_obj_t *combo_box, int item_max);

int tinyui_combo_box_set_dropdown_image(tinyui_obj_t *combo_box, struct tinyui_image_source *source);

int tinyui_combo_box_set_dropdown_source(tinyui_obj_t *combo_box, struct tinyui_image_source *source);

void tinyui_combo_box_set_on_selected(tinyui_obj_t *combo_box, void (*callback)(tinyui_obj_t *combo_box, int index, void *user_data), void *user_data);

#endif /* TINYUI_COMBO_BOX_H */
