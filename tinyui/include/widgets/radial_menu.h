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

#ifndef TINYUI_RADIAL_MENU_H
#define TINYUI_RADIAL_MENU_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_RADIAL_MENU
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_RADIAL_MENU is disabled"
#  endif
#endif

#include "core/obj.h"
#include <stdint.h>

struct tinyui_image_source;

typedef enum tinyui_radial_menu_field {
    TINYUI_RADIAL_MENU_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_RADIAL_MENU_FIELD_STYLE_CLASS = UINT32_C(1) << 1,
    TINYUI_RADIAL_MENU_FIELD_USER_DATA = UINT32_C(1) << 2,
    TINYUI_RADIAL_MENU_FIELD_WIDTH = UINT32_C(1) << 3,
    TINYUI_RADIAL_MENU_FIELD_HEIGHT = UINT32_C(1) << 4,
    TINYUI_RADIAL_MENU_FIELD_X_AXIS = UINT32_C(1) << 5,
    TINYUI_RADIAL_MENU_FIELD_Y_AXIS = UINT32_C(1) << 6,
    TINYUI_RADIAL_MENU_FIELD_ITEM_MAX = UINT32_C(1) << 7,
    TINYUI_RADIAL_MENU_FIELD_DEFAULT_INDEX = UINT32_C(1) << 8,
} tinyui_radial_menu_field_t;

typedef struct tinyui_radial_menu_props {
    uint32_t fields;
    uint16_t id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int x_axis;
    int y_axis;
    int item_max;
    int default_index;
} tinyui_radial_menu_props_t;

tinyui_obj_t *tinyui_radial_menu_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_radial_menu_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_radial_menu_props_t *props);

int tinyui_radial_menu_add_item(tinyui_obj_t *radial_menu, const char *id);

int tinyui_radial_menu_add_item_with_source(tinyui_obj_t *radial_menu, const char *id, struct tinyui_image_source *source);

int tinyui_radial_menu_add_item_with_image(tinyui_obj_t *radial_menu, const char *id, struct tinyui_image_source *source);

int tinyui_radial_menu_set_geometry(tinyui_obj_t *radial_menu, int width, int height, int x_axis, int y_axis, int item_max);

int tinyui_radial_menu_set_selected_index(tinyui_obj_t *radial_menu, int index);

int tinyui_radial_menu_get_selected_index(const tinyui_obj_t *radial_menu);

int tinyui_radial_menu_offset_selection(tinyui_obj_t *radial_menu, int offset);

int tinyui_radial_menu_set_default_item(tinyui_obj_t *radial_menu, int index);

int tinyui_radial_menu_click_item(tinyui_obj_t *radial_menu, int index);

int tinyui_radial_menu_set_click_item(tinyui_obj_t *radial_menu, int index);

int tinyui_radial_menu_offset_item(tinyui_obj_t *radial_menu, int offset);

void tinyui_radial_menu_set_on_selected(tinyui_obj_t *radial_menu, void (*callback)(tinyui_obj_t *radial_menu, int index, void *user_data), void *user_data);

#endif /* TINYUI_RADIAL_MENU_H */
