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

#include "internal.h"
#include "picoui/table.h"
#include "../backend/ldgui/backend.h"
#include "../../../src/gui/ldTable.h"

#include <stdlib.h>
#include <string.h>

int picoui_native_table_set_current_cell(struct picoui_table *table, int row, int column, int emit_callback);
void picoui_native_table_reset_render_state(struct picoui_table *table);
int picoui_backend_table_set_current_cell(void *backend_widget, int row, int column);
int picoui_backend_table_sync_current_cell(struct picoui_table *table, int *row_out, int *column_out);

struct picoui_table_ext {
    struct picoui_table table;
    const char *cell_texts[];
};

static struct picoui_table_ext *picoui_table_ext_from_table(struct picoui_table *table)
{
    if (table == 0) {
        return 0;
    }

    return (struct picoui_table_ext *)table;
}

static const struct picoui_table_ext *picoui_table_ext_from_table_const(const struct picoui_table *table)
{
    if (table == 0) {
        return 0;
    }

    return (const struct picoui_table_ext *)table;
}

static const char **picoui_table_cell_slot(struct picoui_table *table, int row, int column)
{
    struct picoui_table_ext *ext;

    if (table == 0 || row < 0 || column < 0 || row >= table->row_count || column >= table->column_count) {
        return 0;
    }

    ext = picoui_table_ext_from_table(table);
    if (ext == 0) {
        return 0;
    }

    return &ext->cell_texts[(row * table->column_count) + column];
}

static const char *const *picoui_table_cell_slot_const(const struct picoui_table *table, int row, int column)
{
    const struct picoui_table_ext *ext;

    if (table == 0 || row < 0 || column < 0 || row >= table->row_count || column >= table->column_count) {
        return 0;
    }

    ext = picoui_table_ext_from_table_const(table);
    if (ext == 0) {
        return 0;
    }

    return &ext->cell_texts[(row * table->column_count) + column];
}

static int picoui_table_default_column_width(const struct picoui_table *table)
{
    int width;

    if (table == 0 || table->column_count <= 0) {
        return -1;
    }

    width = picoui_widget_get_width((const struct picoui_widget *)table);
    if (width <= table->item_space) {
        return -1;
    }

    width = ((width - table->item_space) / table->column_count) - table->item_space;
    return width > 0 ? width : -1;
}

static int picoui_table_default_row_height(const struct picoui_table *table)
{
    int height;

    if (table == 0 || table->row_count <= 0) {
        return -1;
    }

    height = picoui_widget_get_height((const struct picoui_widget *)table);
    if (height <= table->item_space) {
        return -1;
    }

    height = ((height - table->item_space) / table->row_count) - table->item_space;
    return height > 0 ? height : -1;
}

static int picoui_table_get_local_column_width(const struct picoui_table *table, int column)
{
    if (table == 0 || column < 0 || column >= table->column_count) {
        return -1;
    }

    if (table->column_widths[column] > 0) {
        return table->column_widths[column];
    }

    return picoui_table_default_column_width(table);
}

static int picoui_table_get_local_row_height(const struct picoui_table *table, int row)
{
    if (table == 0 || row < 0 || row >= table->row_count) {
        return -1;
    }

    if (table->row_heights[row] > 0) {
        return table->row_heights[row];
    }

    return picoui_table_default_row_height(table);
}

static int picoui_table_dims_are_valid(int rows, int columns)
{
    return rows > 0 && rows <= 255 && columns > 0 && columns <= 255;
}

static int picoui_table_keyboard_binding_is_valid(unsigned int keyboard_binding)
{
    return keyboard_binding > 0U && keyboard_binding <= 0xFFFFU;
}

static int picoui_table_props_are_valid(const struct picoui_table_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           picoui_table_dims_are_valid(props->rows, props->columns) &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->radius >= 0 &&
           props->padding >= 0 &&
           (props->has_keyboard_binding == 0 ||
            picoui_table_keyboard_binding_is_valid(props->keyboard_binding));
}

static ldTable_t *picoui_table_get_ld_widget(const struct picoui_table *table)
{
    const struct picoui_backend_widget *backend;

    if (table == 0 || table->widget.backend_widget == 0) {
        return 0;
    }

    backend = (const struct picoui_backend_widget *)table->widget.backend_widget;
    return (ldTable_t *)backend->ld_widget;
}

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
                                         int columns)
{
    struct picoui_table *table;
    struct picoui_table_ext *ext;
    size_t cell_count;

    if (parent == 0 || id == 0 || !picoui_table_dims_are_valid(rows, columns)) {
        return 0;
    }

    cell_count = (size_t)rows * (size_t)columns;
    ext = calloc(1, sizeof(*ext) + (cell_count * sizeof(ext->cell_texts[0])));
    if (ext == 0) {
        return 0;
    }
    table = &ext->table;

    table->widget.backend_widget =
        picoui_backend_create_table(parent->widget.backend_widget, id, rows, columns);
    if (table->widget.backend_widget == 0) {
        free(ext);
        return 0;
    }

    table->id = id;
    table->row_count = rows;
    table->column_count = columns;
    table->current_row = 0;
    table->current_column = 0;
    table->item_space = 4;
    table->widget.visible = 1;
    table->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(table->widget.backend_widget, &table->widget) != 0) {
        free(ext);
        return 0;
    }
    if (picoui_backend_table_bind_host(table->widget.backend_widget) != 0) {
        free(ext);
        return 0;
    }
    return table;
}

/**
 * @brief Create table widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_table *picoui_table_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_table_props *props)
{
    struct picoui_table *table;

    if (!picoui_table_props_are_valid(props)) {
        return 0;
    }

    table = picoui_table_create(parent, props->id, props->rows, props->columns);
    if (table == 0) {
        return 0;
    }

    if ((props->has_keyboard_binding != 0 &&
         picoui_table_set_keyboard_binding(table, props->keyboard_binding) != 0) ||
        picoui_widget_set_user_data(&table->widget, props->user_data) != 0 ||
        picoui_widget_set_bg_color(&table->widget, props->bg_color) != 0 ||
        picoui_widget_set_text_color(&table->widget, props->text_color) != 0 ||
        picoui_widget_set_border_color(&table->widget, props->border_color) != 0 ||
        picoui_widget_set_radius(&table->widget, props->radius) != 0 ||
        picoui_widget_set_padding(&table->widget, props->padding) != 0) {
        free(table);
        return 0;
    }
    if (props->style_class != 0 &&
        picoui_widget_set_style_class(&table->widget, props->style_class) != 0) {
        free(table);
        return 0;
    }
    if ((props->width > 0 || props->height > 0) &&
        picoui_widget_set_size(&table->widget, props->width, props->height) != 0) {
        free(table);
        return 0;
    }

    return table;
}

/**
 * @brief table init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] rows Row definitions
 * @param[in] columns Column definitions
 * @return Pointer to the object
 */

struct picoui_table *picoui_table_init(struct picoui_window *parent,
                                       const char *id,
                                       int rows,
                                       int columns)
{
    return picoui_table_create(parent, id, rows, columns);
}

/**
 * @brief Set keyboard of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_keyboard(struct picoui_table *table, unsigned int keyboard_binding)
{
    return picoui_table_set_keyboard_binding(table, keyboard_binding);
}

/**
 * @brief Set keyboard binding of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_keyboard_binding(struct picoui_table *table, unsigned int keyboard_binding)
{
    if (table == 0 || !picoui_table_keyboard_binding_is_valid(keyboard_binding)) {
        return -1;
    }

    if (picoui_backend_table_set_keyboard_binding(table->widget.backend_widget,
                                                  keyboard_binding) != 0) {
        return -1;
    }
    table->keyboard_binding = keyboard_binding;
    return 0;
}

/**
 * @brief Get keyboard binding of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return -1 on failure
 */

int picoui_table_get_keyboard_binding(const struct picoui_table *table, unsigned int *keyboard_binding)
{
    if (table == 0 || keyboard_binding == 0) {
        return -1;
    }

    return picoui_backend_table_get_keyboard_binding((void *)table->widget.backend_widget,
                                                     keyboard_binding);
}

/**
 * @brief tabel show keyboard
 *
 * @param[in] table table
 * @return 0 on success, -1 on failure
 */

int picoui_tabel_show_keyboard(struct picoui_table *table)
{
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;
    ldTableItem_t *item;

    if (table == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    ld_table = picoui_table_get_ld_widget(table);
    if (backend == 0 || ld_table == 0) {
        return -1;
    }
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    if (app_state == 0 || app_state->ld_scene == 0) {
        return -1;
    }

    item = ldTableGetItem(ld_table, (uint8_t)table->current_row, (uint8_t)table->current_column);
    if (item == 0) {
        return -1;
    }

    _ldTabelShowKeyboard(app_state->ld_scene, ld_table, item);
    return 0;
}

/**
 * @brief Set cell text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int picoui_table_set_cell_text(struct picoui_table *table,
                               int row,
                               int column,
                               const char *text)
{
    const char **slot;

    if (table == 0 || text == 0) {
        return -1;
    }

    slot = picoui_table_cell_slot(table, row, column);
    if (slot == 0) {
        return -1;
    }

    *slot = text;
    if (table->widget.backend_widget != 0
        && picoui_backend_table_set_cell_text(table->widget.backend_widget, row, column, text) != 0) {
        return -1;
    }
    picoui_native_table_reset_render_state(table);
    return 0;
}

/**
 * @brief Set item text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_item_text(struct picoui_table *table, int row, int column, const char *text)
{
    return picoui_table_set_cell_text(table, row, column, text);
}

/**
 * @brief Get cell text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char *picoui_table_get_cell_text(const struct picoui_table *table, int row, int column)
{
    const char *backend_text;
    const char **mutable_slot;
    const char *const *slot;

    if (table == 0) {
        return 0;
    }

    slot = picoui_table_cell_slot_const(table, row, column);
    if (slot == 0) {
        return 0;
    }

    if (table->widget.last_edit_result == PICOUI_EDIT_RESULT_CANCEL) {
        return *slot;
    }

    if (table->widget.backend_widget != 0) {
        backend_text = picoui_backend_table_get_cell_text(table->widget.backend_widget, row, column);
        if (backend_text != 0) {
            mutable_slot = picoui_table_cell_slot((struct picoui_table *)table, row, column);
            if (mutable_slot != 0) {
                *mutable_slot = backend_text;
            }
            return backend_text;
        }
    }

    return *slot;
}

/**
 * @brief Get item text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char *picoui_table_get_item_text(const struct picoui_table *table, int row, int column)
{
    return picoui_table_get_cell_text(table, row, column);
}

/**
 * @brief Set cell editable of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] editable editable
 * @param[in] text_max text max
 * @return -1 on failure
 */

int picoui_table_set_cell_editable(struct picoui_table *table,
                                   int row,
                                   int column,
                                   int editable,
                                   unsigned int text_max)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_cell_editable(table->widget.backend_widget,
                                                  row,
                                                  column,
                                                  editable != 0,
                                                  text_max);
}

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
                                   unsigned int text_max)
{
    return picoui_table_set_cell_editable(table, row, column, editable, text_max);
}

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
 * @return -1 on failure
 */

int picoui_table_set_item_image(struct picoui_table *table,
                                int row,
                                int column,
                                int x,
                                int y,
                                struct picoui_image_source *source,
                                unsigned int mask_color)
{
    if (table == 0 || source == 0 || source->img_tile == 0 || source->mask_tile == 0 ||
        mask_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_item_image(table->widget.backend_widget,
                                               row,
                                               column,
                                               x,
                                               y,
                                               source,
                                               mask_color);
}

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
 * @return -1 on failure
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
                                 int checkable)
{
    if (table == 0 || release_source == 0 || press_source == 0 ||
        release_source->img_tile == 0 || release_source->mask_tile == 0 ||
        press_source->img_tile == 0 || press_source->mask_tile == 0 ||
        release_mask_color > 0xFFFFFFU || press_mask_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_item_button(table->widget.backend_widget,
                                                row,
                                                column,
                                                x,
                                                y,
                                                release_source,
                                                release_mask_color,
                                                press_source,
                                                press_mask_color,
                                                checkable != 0);
}

/**
 * @brief Set excel type of table widget
 *
 * @param[in] table table
 * @return -1 on failure
 */

int picoui_table_set_excel_type(struct picoui_table *table)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_excel_type(table->widget.backend_widget);
}

/**
 * @brief Set background color of table widget
 *
 * @param[in] table table
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_background_color(struct picoui_table *table, unsigned int bg_color)
{
    return picoui_table_set_bg_color(table, bg_color);
}

/**
 * @brief Set align grid of table widget
 *
 * @param[in] table table
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_align_grid(struct picoui_table *table, int enabled)
{
    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = picoui_table_get_ld_widget(table);
    if (ld_table == 0) {
        return -1;
    }

    ldTableSetAlignGrid(ld_table, enabled != 0);
    return 0;
}

/**
 * @brief Set item width of table widget
 *
 * @param[in] table table
 * @param[in] column Column index
 * @param[in] width Width in pixels
 * @return -1 on failure
 */

int picoui_table_set_item_width(struct picoui_table *table, int column, int width)
{
    int previous_width;

    if (table == 0 || column < 0 || column >= table->column_count || width <= 0) {
        return -1;
    }

    previous_width = table->column_widths[column];
    table->column_widths[column] = width;
    if (picoui_backend_table_set_item_width(table->widget.backend_widget, column, width) != 0) {
        table->column_widths[column] = previous_width;
        return -1;
    }

    picoui_native_table_reset_render_state(table);
    return 0;
}

/**
 * @brief Set item height of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] height Height in pixels
 * @return -1 on failure
 */

int picoui_table_set_item_height(struct picoui_table *table, int row, int height)
{
    int previous_height;

    if (table == 0 || row < 0 || row >= table->row_count || height <= 0) {
        return -1;
    }

    previous_height = table->row_heights[row];
    table->row_heights[row] = height;
    if (picoui_backend_table_set_item_height(table->widget.backend_widget, row, height) != 0) {
        table->row_heights[row] = previous_height;
        return -1;
    }

    picoui_native_table_reset_render_state(table);
    return 0;
}

/**
 * @brief Set item color of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text_color Text color
 * @param[in] bg_color Background color
 * @return -1 on failure
 */

int picoui_table_set_item_color(struct picoui_table *table,
                                int row,
                                int column,
                                unsigned int text_color,
                                unsigned int bg_color)
{
    if (table == 0 || text_color > 0xFFFFFFU || bg_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_item_color(table->widget.backend_widget,
                                               row,
                                               column,
                                               text_color,
                                               bg_color);
}

/**
 * @brief Set bg color of table widget
 *
 * @param[in] table table
 * @param[in] bg_color Background color
 * @return -1 on failure
 */

int picoui_table_set_bg_color(struct picoui_table *table, unsigned int bg_color)
{
    if (table == 0 || bg_color > 0xFFFFFFU) {
        return -1;
    }

    return picoui_backend_table_set_bg_color(table->widget.backend_widget, bg_color);
}

/**
 * @brief Set item static text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return -1 on failure
 */

int picoui_table_set_item_static_text(struct picoui_table *table, int row, int column, const char *text)
{
    if (table == 0 || text == 0) {
        return -1;
    }

    return picoui_backend_table_set_item_static_text(table->widget.backend_widget, row, column, text);
}

/**
 * @brief Set item font of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_table_set_item_font(struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_item_font(table->widget.backend_widget, row, column);
}

/**
 * @brief Get align grid of table widget
 *
 * @param[in] table table
 * @return -1 on failure
 */

int picoui_table_get_align_grid(const struct picoui_table *table)
{
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0) {
        return -1;
    }

    return ldTableGetAlignGrid(ld_table) ? 1 : 0;
}

/**
 * @brief Get background color of table widget
 *
 * @param[in] table table
 */

unsigned int picoui_table_get_background_color(const struct picoui_table *table)
{
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0) {
        return 0U;
    }

    return (unsigned int)ldTableGetBackgroundColor(ld_table);
}

/**
 * @brief Get item of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

void *picoui_table_get_item(const struct picoui_table *table, int row, int column)
{
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0 || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return 0;
    }

    return ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);
}

/**
 * @brief Set item align of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] align align
 * @return -1 on failure
 */

int picoui_table_set_item_align(struct picoui_table *table,
                                int row,
                                int column,
                                enum picoui_align align)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_set_item_align(table->widget.backend_widget, row, column, align);
}

/**
 * @brief Get item align of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_table_get_item_align(const struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_get_item_align((void *)table->widget.backend_widget, row, column);
}

/**
 * @brief Get item editable of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_table_get_item_editable(const struct picoui_table *table, int row, int column)
{
    if (table == 0) {
        return -1;
    }

    return picoui_backend_table_get_item_editable((void *)table->widget.backend_widget, row, column);
}

/**
 * @brief Get item font of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

void *picoui_table_get_item_font(const struct picoui_table *table, int row, int column)
{
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0 || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return 0;
    }

    return ldTableGetItemFont(ld_table, (uint8_t)row, (uint8_t)column);
}

/**
 * @brief Get item height of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @return -1 on failure
 */

int picoui_table_get_item_height(const struct picoui_table *table, int row)
{
    return picoui_table_get_local_row_height(table, row);
}

/**
 * @brief Get item text color of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

unsigned int picoui_table_get_item_text_color(const struct picoui_table *table, int row, int column)
{
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0 || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return 0U;
    }

    return (unsigned int)ldTableGetItemTextColor(ld_table, (uint8_t)row, (uint8_t)column);
}

/**
 * @brief Get item background color of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

unsigned int picoui_table_get_item_background_color(const struct picoui_table *table, int row, int column)
{
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0 || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return 0U;
    }

    return (unsigned int)ldTableGetItemBackgroundColor(ld_table, (uint8_t)row, (uint8_t)column);
}

/**
 * @brief Get item width of table widget
 *
 * @param[in] table table
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_table_get_item_width(const struct picoui_table *table, int column)
{
    return picoui_table_get_local_column_width(table, column);
}

/**
 * @brief table navigate
 *
 * @param[in] table table
 * @param[in] dir dir
 * @return -1 on failure
 */

int picoui_table_navigate(struct picoui_table *table, enum picoui_native_nav_dir dir)
{
    if (table == 0 ||
        (dir != PICOUI_NATIVE_NAV_LEFT &&
         dir != PICOUI_NATIVE_NAV_RIGHT &&
         dir != PICOUI_NATIVE_NAV_UP &&
         dir != PICOUI_NATIVE_NAV_DOWN)) {
        return -1;
    }

    return picoui_backend_table_navigate(table->widget.backend_widget, dir);
}

/**
 * @brief Get item region of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return Pointer to the object
 */

struct picoui_table_region picoui_table_get_item_region(const struct picoui_table *table, int row, int column)
{
    struct picoui_table_region region = {0};
    int i;
    int width;
    int height;

    if (table == 0) {
        return region;
    }

    width = picoui_table_get_local_column_width(table, column);
    height = picoui_table_get_local_row_height(table, row);
    if (width <= 0 || height <= 0) {
        return region;
    }

    region.x = table->item_space;
    for (i = 0; i < column; ++i) {
        int prev_width = picoui_table_get_local_column_width(table, i);

        if (prev_width <= 0) {
            return (struct picoui_table_region){0};
        }
        region.x += prev_width + table->item_space;
    }

    region.y = table->item_space;
    for (i = 0; i < row; ++i) {
        int prev_height = picoui_table_get_local_row_height(table, i);

        if (prev_height <= 0) {
            return (struct picoui_table_region){0};
        }
        region.y += prev_height + table->item_space;
    }

    region.width = width;
    region.height = height;
    return region;
}

/**
 * @brief Set selected cell of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_selected_cell(struct picoui_table *table, int row, int column)
{
    if (picoui_table_cell_slot(table, row, column) == 0) {
        return -1;
    }

    if (picoui_native_table_set_current_cell(table, row, column, 1) != 0) {
        return -1;
    }
    if (table->widget.backend_widget != 0
        && picoui_backend_table_set_current_cell(table->widget.backend_widget, row, column) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Set item select of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] selected selected
 * @return -1 on failure
 */

int picoui_table_set_item_select(struct picoui_table *table, int row, int column, int selected)
{
    if (selected == 0) {
        return -1;
    }

    return picoui_table_set_selected_cell(table, row, column);
}

/**
 * @brief Set current cell of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_table_set_current_cell(struct picoui_table *table, int row, int column)
{
    if (picoui_table_cell_slot(table, row, column) == 0) {
        return -1;
    }

    if (picoui_native_table_set_current_cell(table, row, column, 0) != 0) {
        return -1;
    }
    if (table->widget.backend_widget != 0
        && picoui_backend_table_set_current_cell(table->widget.backend_widget, row, column) != 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief Get current row of table widget
 *
 * @param[in] table table
 * @return -1 on failure
 */

int picoui_table_get_current_row(const struct picoui_table *table)
{
    int row;

    if (table == 0) {
        return -1;
    }

    if (table->widget.backend_widget != 0
        && picoui_backend_table_sync_current_cell((struct picoui_table *)table, &row, NULL) == 0) {
        return row;
    }

    return table->current_row;
}

/**
 * @brief Get current column of table widget
 *
 * @param[in] table table
 * @return -1 on failure
 */

int picoui_table_get_current_column(const struct picoui_table *table)
{
    int column;

    if (table == 0) {
        return -1;
    }

    if (table->widget.backend_widget != 0
        && picoui_backend_table_sync_current_cell((struct picoui_table *)table, NULL, &column) == 0) {
        return column;
    }

    return table->current_column;
}

void picoui_table_set_on_selected(struct picoui_table *table,
                                  void (*callback)(struct picoui_table *table,
                                                   int row,
                                                   int column,
                                                   void *user_data),
                                  void *user_data)
{
    if (table == 0) {
        return;
    }

    table->on_selected = callback;
    table->on_selected_user_data = user_data;
}
