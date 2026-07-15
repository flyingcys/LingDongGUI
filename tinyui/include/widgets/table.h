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

#ifndef TINYUI_TABLE_H
#define TINYUI_TABLE_H

#include "tinyui_config.h"
#if !TINYUI_ENABLE_TABLE
#  ifndef TINYUI_INTERNAL_FEATURE_HEADER
#  error "TINYUI_ENABLE_TABLE is disabled"
#  endif
#endif


#include "layout/layout.h"

#include "core/obj.h"
#include <stdint.h>
#include "core/nav.h"




struct tinyui_image_source;
struct tinyui_keyboard;
struct tinyui_table_region;

struct tinyui_table_region {
    int x;
    int y;
    int width;
    int height;
};

typedef enum tinyui_table_field {
    TINYUI_TABLE_FIELD_ID = UINT32_C(1) << 0,
    TINYUI_TABLE_FIELD_KEYBOARD_BINDING = UINT32_C(1) << 1,
    TINYUI_TABLE_FIELD_ROWS = UINT32_C(1) << 2,
    TINYUI_TABLE_FIELD_COLUMNS = UINT32_C(1) << 3,
    TINYUI_TABLE_FIELD_STYLE_CLASS = UINT32_C(1) << 4,
    TINYUI_TABLE_FIELD_USER_DATA = UINT32_C(1) << 5,
    TINYUI_TABLE_FIELD_WIDTH = UINT32_C(1) << 6,
    TINYUI_TABLE_FIELD_HEIGHT = UINT32_C(1) << 7,
    TINYUI_TABLE_FIELD_BG_COLOR = UINT32_C(1) << 8,
    TINYUI_TABLE_FIELD_TEXT_COLOR = UINT32_C(1) << 9,
    TINYUI_TABLE_FIELD_BORDER_COLOR = UINT32_C(1) << 10,
    TINYUI_TABLE_FIELD_RADIUS = UINT32_C(1) << 11,
    TINYUI_TABLE_FIELD_PADDING = UINT32_C(1) << 12,
} tinyui_table_field_t;

typedef struct tinyui_table_props {
    uint32_t fields;
    uint16_t id;
    unsigned int keyboard_binding;
    int rows;
    int columns;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
} tinyui_table_props_t;

tinyui_obj_t *tinyui_table_create(tinyui_obj_t *parent);

tinyui_obj_t *tinyui_table_create_with_props(tinyui_obj_t *parent,
                                               const tinyui_table_props_t *props);

int tinyui_table_set_keyboard(tinyui_obj_t *table, unsigned int keyboard_binding);

int tinyui_table_set_keyboard_binding(tinyui_obj_t *table, unsigned int keyboard_binding);

int tinyui_table_set_keyboard_widget(tinyui_obj_t *table, tinyui_obj_t *keyboard);

int tinyui_table_get_keyboard_binding(const tinyui_obj_t *table, unsigned int *keyboard_binding);

int tinyui_table_set_item_space(tinyui_obj_t *table, unsigned int item_space);

int tinyui_table_show_keyboard(tinyui_obj_t *table);


int tinyui_table_set_item_text(tinyui_obj_t *table, int row, int column, const char *text);

int tinyui_table_set_cell_text(tinyui_obj_t *table, int row, int column, const char *text);

const char *tinyui_table_get_item_text(const tinyui_obj_t *table, int row, int column);

const char *tinyui_table_get_cell_text(const tinyui_obj_t *table, int row, int column);

int tinyui_table_set_item_editable(tinyui_obj_t *table, int row, int column, int editable, unsigned int text_max);

int tinyui_table_set_cell_editable(tinyui_obj_t *table, int row, int column, int editable, unsigned int text_max);

int tinyui_table_set_item_image(tinyui_obj_t *table, int row, int column, int x, int y, struct tinyui_image_source *source, unsigned int mask_color);

int tinyui_table_set_item_button(tinyui_obj_t *table, int row, int column, int x, int y, struct tinyui_image_source *release_source, unsigned int release_mask_color, struct tinyui_image_source *press_source, unsigned int press_mask_color, int checkable);

int tinyui_table_set_background_color(tinyui_obj_t *table, unsigned int bg_color);

int tinyui_table_set_excel_type(tinyui_obj_t *table);

int tinyui_table_set_align_grid(tinyui_obj_t *table, int enabled);

int tinyui_table_set_item_width(tinyui_obj_t *table, int column, int width);

int tinyui_table_set_item_height(tinyui_obj_t *table, int row, int height);

int tinyui_table_set_item_color(tinyui_obj_t *table, int row, int column, unsigned int text_color, unsigned int bg_color);

int tinyui_table_set_bg_color(tinyui_obj_t *table, unsigned int bg_color);

int tinyui_table_set_item_static_text(tinyui_obj_t *table, int row, int column, const char *text);

int tinyui_table_set_item_font(tinyui_obj_t *table, int row, int column);

int tinyui_table_get_align_grid(const tinyui_obj_t *table);

unsigned int tinyui_table_get_background_color(const tinyui_obj_t *table);

void *tinyui_table_get_item(const tinyui_obj_t *table, int row, int column);

int tinyui_table_set_item_align(tinyui_obj_t *table, int row, int column, enum tinyui_align align);

int tinyui_table_get_item_align(const tinyui_obj_t *table, int row, int column);

int tinyui_table_get_item_editable(const tinyui_obj_t *table, int row, int column);

void *tinyui_table_get_item_font(const tinyui_obj_t *table, int row, int column);

int tinyui_table_get_item_height(const tinyui_obj_t *table, int row);

unsigned int tinyui_table_get_item_text_color(const tinyui_obj_t *table, int row, int column);

unsigned int tinyui_table_get_item_background_color(const tinyui_obj_t *table, int row, int column);

int tinyui_table_get_item_width(const tinyui_obj_t *table, int column);

int tinyui_table_navigate(tinyui_obj_t *table, tinyui_nav_dir_t dir);

int tinyui_table_set_item_select(tinyui_obj_t *table, int row, int column, int selected);

struct tinyui_table_region tinyui_table_get_item_region(const tinyui_obj_t *table, int row, int column);

int tinyui_table_set_selected_cell(tinyui_obj_t *table, int row, int column);

int tinyui_table_set_current_cell(tinyui_obj_t *table, int row, int column);

int tinyui_table_get_current_row(const tinyui_obj_t *table);

int tinyui_table_get_current_column(const tinyui_obj_t *table);

#endif /* TINYUI_TABLE_H */
