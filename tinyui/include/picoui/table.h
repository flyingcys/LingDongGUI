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

#ifndef PICOUI_TABLE_H
#define PICOUI_TABLE_H

#include "picoui/native.h"
#include "picoui/widget.h"

struct picoui_window;
struct picoui_table;
struct picoui_image_source;

struct picoui_table_region {
    int x;
    int y;
    int width;
    int height;
};

struct picoui_table_props {
    const char *id;
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
    int has_keyboard_binding;
};

/**
 * @brief Create table widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] rows Row definitions
 * @param[in] columns Column definitions
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_table *picoui_table_create(struct picoui_window *parent,
                                         const char *id,
                                         int rows,
                                         int columns);

/**
 * @brief Create table widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_table *picoui_table_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_table_props *props);

/**
 * @brief table init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] rows Row definitions
 * @param[in] columns Column definitions
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_table *picoui_table_init(struct picoui_window *parent,
                                       const char *id,
                                       int rows,
                                       int columns);

/**
 * @brief Set keyboard of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_keyboard(struct picoui_table *table, unsigned int keyboard_binding);

/**
 * @brief Set keyboard binding of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_keyboard_binding(struct picoui_table *table, unsigned int keyboard_binding);

/**
 * @brief Get keyboard binding of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return The property value, negative on error
 */

int picoui_table_get_keyboard_binding(const struct picoui_table *table, unsigned int *keyboard_binding);

/**
 * @brief tabel show keyboard
 *
 * @param[in] table table
 * @return 0 on success, -1 on failure
 */

int picoui_tabel_show_keyboard(struct picoui_table *table);

/**
 * @brief Set item text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text);

/**
 * @brief Set cell text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_cell_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text);

/**
 * @brief Get item text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char *picoui_table_get_item_text(const struct picoui_table *table, int row, int column);

/**
 * @brief Get cell text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char *picoui_table_get_cell_text(const struct picoui_table *table, int row, int column);

/**
 * @brief Set item editable of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] editable editable
 * @param[in] text_max text max
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);

/**
 * @brief Set cell editable of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] editable editable
 * @param[in] text_max text max
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_cell_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max);

/**
 * @brief Set item image of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] source Image source
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_image(struct picoui_table *table,
                                int row,
                                int column,
                                int x,
                                int y,
                                struct picoui_image_source *source,
                                unsigned int mask_color);

/**
 * @brief Set item button of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] release_source release source
 * @param[in] release_mask_color release mask color
 * @param[in] press_source press source
 * @param[in] press_mask_color press mask color
 * @param[in] checkable checkable
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_button(struct picoui_table *table,
                                 int row,
                                 int column,
                                 int x,
                                 int y,
                                 struct picoui_image_source *release_source,
                                 unsigned int release_mask_color,
                                 struct picoui_image_source *press_source,
                                 unsigned int press_mask_color,
                                 int checkable);

/**
 * @brief Set background color of table widget
 *
 * @param[in] table table
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_background_color(struct picoui_table *table, unsigned int bg_color);

/**
 * @brief Set excel type of table widget
 *
 * @param[in] table table
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_excel_type(struct picoui_table *table);

/**
 * @brief Set align grid of table widget
 *
 * @param[in] table table
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_align_grid(struct picoui_table *table, int enabled);

/**
 * @brief Set item width of table widget
 *
 * @param[in] table table
 * @param[in] column Column index
 * @param[in] width Width in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_width(struct picoui_table *table, int column, int width);

/**
 * @brief Set item height of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] height Height in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_height(struct picoui_table *table, int row, int height);

/**
 * @brief Set item color of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text_color Text color
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_color(struct picoui_table *table,
                                int row,
                                int column,
                                unsigned int text_color,
                                unsigned int bg_color);

/**
 * @brief Set bg color of table widget
 *
 * @param[in] table table
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_bg_color(struct picoui_table *table, unsigned int bg_color);

/**
 * @brief Set item static text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_static_text(struct picoui_table *table, int row, int column, const char *text);

/**
 * @brief Set item font of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_font(struct picoui_table *table, int row, int column);

/**
 * @brief Get align grid of table widget
 *
 * @param[in] table table
 * @return The property value, negative on error
 */

int picoui_table_get_align_grid(const struct picoui_table *table);

/**
 * @brief Get background color of table widget
 *
 * @param[in] table table
 */

unsigned int picoui_table_get_background_color(const struct picoui_table *table);

/**
 * @brief Get item of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

void *picoui_table_get_item(const struct picoui_table *table, int row, int column);

/**
 * @brief Set item align of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_align(struct picoui_table *table,
                                int row,
                                int column,
                                enum picoui_align align);

/**
 * @brief Get item align of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return The property value, negative on error
 */

int picoui_table_get_item_align(const struct picoui_table *table, int row, int column);

/**
 * @brief Get item editable of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return The property value, negative on error
 */

int picoui_table_get_item_editable(const struct picoui_table *table, int row, int column);

/**
 * @brief Get item font of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

void *picoui_table_get_item_font(const struct picoui_table *table, int row, int column);

/**
 * @brief Get item height of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @return The property value, negative on error
 */

int picoui_table_get_item_height(const struct picoui_table *table, int row);

/**
 * @brief Get item text color of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

unsigned int picoui_table_get_item_text_color(const struct picoui_table *table, int row, int column);

/**
 * @brief Get item background color of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

unsigned int picoui_table_get_item_background_color(const struct picoui_table *table, int row, int column);

/**
 * @brief Get item width of table widget
 *
 * @param[in] table table
 * @param[in] column Column index
 * @return The property value, negative on error
 */

int picoui_table_get_item_width(const struct picoui_table *table, int column);

/**
 * @brief table navigate
 *
 * @param[in] table table
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

int picoui_table_navigate(struct picoui_table *table, enum picoui_native_nav_dir dir);

/**
 * @brief Set item select of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] selected selected
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_select(struct picoui_table *table, int row, int column, int selected);

/**
 * @brief Get item region of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_table_region picoui_table_get_item_region(const struct picoui_table *table,
                                                        int row,
                                                        int column);

/**
 * @brief Set selected cell of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_selected_cell(struct picoui_table *table, int row, int column);

/**
 * @brief Set current cell of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_current_cell(struct picoui_table *table, int row, int column);

/**
 * @brief Get current row of table widget
 *
 * @param[in] table table
 * @return The property value, negative on error
 */

int picoui_table_get_current_row(const struct picoui_table *table);

/**
 * @brief Get current column of table widget
 *
 * @param[in] table table
 * @return The property value, negative on error
 */

int picoui_table_get_current_column(const struct picoui_table *table);

#endif
