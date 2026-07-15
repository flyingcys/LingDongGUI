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

#ifndef TINYUI_SCROLL_SELECTOR_H
#define TINYUI_SCROLL_SELECTOR_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_SCROLL_SELECTOR
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_SCROLL_SELECTOR is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_scroll_selector_field {
    TINYUI_SCROLL_SELECTOR_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_SCROLL_SELECTOR_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_SCROLL_SELECTOR_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_SCROLL_SELECTOR_FIELD_WIDTH = UINT32_C(1) << 3,
    TINYUI_SCROLL_SELECTOR_FIELD_HEIGHT = UINT32_C(1) << 4,
    TINYUI_SCROLL_SELECTOR_FIELD_BG_COLOR = UINT32_C(1) << 5,
    TINYUI_SCROLL_SELECTOR_FIELD_TEXT_COLOR = UINT32_C(1) << 6,
    TINYUI_SCROLL_SELECTOR_FIELD_BORDER_COLOR = UINT32_C(1) << 7,
    TINYUI_SCROLL_SELECTOR_FIELD_RADIUS = UINT32_C(1) << 8,
    TINYUI_SCROLL_SELECTOR_FIELD_PADDING = UINT32_C(1) << 9,
} tinyui_scroll_selector_field_t;

typedef struct tinyui_scroll_selector_props {
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
} tinyui_scroll_selector_props_t;

tinyui_obj_t *tinyui_scroll_selector_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_scroll_selector_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_scroll_selector_props_t *props);

int tinyui_scroll_selector_set_items(tinyui_obj_t *scroll_selector, const char *const *item_ids, const char *const *texts, int item_count);

int tinyui_scroll_selector_add_item(tinyui_obj_t *scroll_selector, const char *id, const char *text);

int tinyui_scroll_selector_set_select_item_num(tinyui_obj_t *scroll_selector, int index);

int tinyui_scroll_selector_set_selected_index(tinyui_obj_t *scroll_selector, int index);

int tinyui_scroll_selector_get_select_item_num(const tinyui_obj_t *scroll_selector);

int tinyui_scroll_selector_get_selected_index(const tinyui_obj_t *scroll_selector);

const char *tinyui_scroll_selector_get_select_text(const tinyui_obj_t *scroll_selector);

int tinyui_scroll_selector_set_text_color(tinyui_obj_t *scroll_selector, unsigned int rgb);

int tinyui_scroll_selector_set_background_color(tinyui_obj_t *scroll_selector, unsigned int rgb);

int tinyui_scroll_selector_set_bg_color(tinyui_obj_t *scroll_selector, unsigned int rgb);

int tinyui_scroll_selector_set_indicator_color(tinyui_obj_t *scroll_selector, unsigned int rgb);

int tinyui_scroll_selector_set_background_image(tinyui_obj_t *scroll_selector, struct tinyui_image_source *source);

int tinyui_scroll_selector_set_bg_source(tinyui_obj_t *scroll_selector, struct tinyui_image_source *source);

int tinyui_scroll_selector_set_indicator_image(tinyui_obj_t *scroll_selector, struct tinyui_image_source *source);

int tinyui_scroll_selector_set_indicator_source(tinyui_obj_t *scroll_selector, struct tinyui_image_source *source);

int tinyui_scroll_selector_set_transparent(tinyui_obj_t *scroll_selector, int transparent);

int tinyui_scroll_selector_set_speed(tinyui_obj_t *scroll_selector, int speed);

int tinyui_scroll_selector_set_select_text(tinyui_obj_t *scroll_selector, const char *text);

int tinyui_scroll_selector_set_edit_mode(tinyui_obj_t *scroll_selector, int is_edit);

int tinyui_scroll_selector_get_edit_mode(const tinyui_obj_t *scroll_selector, int *is_edit);

const char *tinyui_scroll_selector_get_selected_text(const tinyui_obj_t *scroll_selector);

#endif /* TINYUI_SCROLL_SELECTOR_H */
