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

#ifndef TINYUI_LIST_H
#define TINYUI_LIST_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_LIST
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_LIST is disabled"
#  endif
#endif

#include "layout/layout.h"

#include "core/obj.h"
#include <stdint.h>

typedef enum tinyui_list_field {
    TINYUI_LIST_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_LIST_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_LIST_FIELD_USER_DATA = UINT32_C(1) << 2,
} tinyui_list_field_t;

typedef struct tinyui_list_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
} tinyui_list_props_t;

tinyui_obj_t *tinyui_list_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_list_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_list_props_t *props);

int tinyui_list_add_item(tinyui_obj_t *list, const char *id, const char *text);

int tinyui_list_set_item_height(tinyui_obj_t *list, int item_height);

int tinyui_list_set_padding_group(tinyui_obj_t *list, int top, int bottom, int left, int right);

int tinyui_list_set_margin_group(tinyui_obj_t *list, int top, int bottom, int left, int right);

int tinyui_list_set_text_color(tinyui_obj_t *list, unsigned int rgb);

int tinyui_list_set_bg_color(tinyui_obj_t *list, unsigned int rgb);

int tinyui_list_set_select_color(tinyui_obj_t *list, unsigned int rgb);

int tinyui_list_set_align(tinyui_obj_t *list, enum tinyui_align align);

int tinyui_list_set_item_widget(tinyui_obj_t *list, int index, tinyui_obj_t *item_widget);

int tinyui_list_set_selected_index(tinyui_obj_t *list, int index);

int tinyui_list_get_selected_index(const tinyui_obj_t *list);

void tinyui_list_set_on_selected(tinyui_obj_t *list, void (*callback)(tinyui_obj_t *list, int index, void *user_data), void *user_data);

#endif /* TINYUI_LIST_H */
