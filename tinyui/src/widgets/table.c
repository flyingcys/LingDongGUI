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
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/ldMsg.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

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

static int picoui_table_sync_current_cell_local(struct picoui_table *table,
                                                int *row_out,
                                                int *column_out);

static bool picoui_table_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_table *table;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    int row = 0;
    int column = 0;
    int was_focus_owner = 0;

    (void)scene;

    if (msg.ptSender == NULL) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    table = (struct picoui_table *)backend->host_widget;
    ld_table = picoui_table_get_ld_widget(table);
    if (ld_table == NULL) {
        return false;
    }

    backend->last_native_signal = msg.signal;
    backend->last_native_value = msg.value;

    (void)picoui_table_sync_current_cell_local(table, &row, &column);
    item = ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);

    if (msg.signal == SIGNAL_PRESS) {
        was_focus_owner = picoui_widget_is_focus_owner(&table->widget);
        (void)picoui_backend_widget_claim_focus(backend);
        if (item != NULL && item->isEditable && (item->isEditing || was_focus_owner)) {
            backend->edit_result_on_finish = PICOUI_EDIT_RESULT_COMMIT;
            (void)picoui_widget_claim_editing(&table->widget);
        }
        return false;
    }

    if (msg.signal != SIGNAL_FINISHED) {
        return false;
    }

    if (item != NULL) {
        item->isEditing = false;
    }
    (void)picoui_widget_mark_edit_result(&table->widget, backend->edit_result_on_finish);
    (void)picoui_widget_release_editing(&table->widget);
    backend->edit_result_on_finish = PICOUI_EDIT_RESULT_NONE;
    return false;
}

static int picoui_table_bind_native_slot(struct picoui_table *table)
{
    ldTable_t *ld_table;

    if (table == NULL || table->widget.backend_widget == NULL) {
        return -1;
    }

    ld_table = picoui_table_get_ld_widget(table);
    if (ld_table == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_table, SIGNAL_PRESS, picoui_table_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_table, SIGNAL_FINISHED, picoui_table_native_slot)) {
        return -1;
    }
    return 0;
}

static int picoui_table_sync_current_cell_local(struct picoui_table *table,
                                                int *row_out,
                                                int *column_out)
{
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;
    int row;
    int column;

    if (table == NULL || table->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    if (backend->host_widget == NULL) {
        return -1;
    }

    ld_table = picoui_table_get_ld_widget(table);
    if (ld_table == NULL) {
        return -1;
    }

    row = (int)ld_table->currentRow;
    column = (int)ld_table->currentColumn;
    table->current_row = row;
    table->current_column = column;
    if (row_out != NULL) {
        *row_out = row;
    }
    if (column_out != NULL) {
        *column_out = column;
    }
    return 0;
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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;
    uint16_t name_id;

    if (parent == 0 || id == 0 || !picoui_table_dims_are_valid(rows, columns)) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    app_state = parent_backend != 0
        ? picoui_runtime_bridge_backend_state_from_parent(parent_backend)
        : 0;
    if (parent_backend == 0 || parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    table = calloc(1, sizeof(*table));
    if (table == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(table);
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(table);
        return 0;
    }

    ld_table = ldTable_init(app_state->ld_scene,
                            NULL,
                            name_id,
                            parent_backend->ld_name_id,
                            0,
                            0,
                            220,
                            120,
                            (uint8_t)rows,
                            (uint8_t)columns,
                            4);
    if (ld_table == 0) {
        free(backend);
        free(table);
        return 0;
    }

    if (picoui_backend_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_TABLE,
                                         id,
                                         parent_backend->theme) != 0) {
        ldTable_depose(app_state->ld_scene, ld_table);
        free(backend);
        free(table);
        return 0;
    }
    backend->ld_widget = ld_table;
    backend->ld_name_id = name_id;
    if (picoui_backend_widget_attach_child(parent_backend, backend) != 0) {
        ldTable_depose(app_state->ld_scene, ld_table);
        free(backend);
        free(table);
        return 0;
    }

    table->widget.backend_widget = backend;
    table->id = id;
    table->row_count = rows;
    table->column_count = columns;
    table->current_row = 0;
    table->current_column = 0;
    table->widget.visible = 1;
    table->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(table->widget.backend_widget, &table->widget) != 0) {
        (void)picoui_backend_widget_detach_from_parent(table->widget.backend_widget);
        ldTable_depose(app_state->ld_scene, ld_table);
        free(backend);
        free(table);
        return 0;
    }
    if (picoui_table_bind_native_slot(table) != 0) {
        (void)picoui_backend_widget_detach_from_parent(table->widget.backend_widget);
        ldTable_depose(app_state->ld_scene, ld_table);
        free(backend);
        free(table);
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
    if (table == 0 || text == 0) {
        return -1;
    }

    return picoui_backend_table_set_cell_text(table->widget.backend_widget, row, column, text);
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
    if (table == 0) {
        return 0;
    }

    return picoui_backend_table_get_cell_text((void *)table->widget.backend_widget, row, column);
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
    if (table == 0 || width <= 0) {
        return -1;
    }

    return picoui_backend_table_set_item_width(table->widget.backend_widget, column, width);
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
    if (table == 0 || height <= 0) {
        return -1;
    }

    return picoui_backend_table_set_item_height(table->widget.backend_widget, row, height);
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
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0 || row < 0 || row >= ld_table->rowCount) {
        return -1;
    }

    return (int)ldTableGetItemHeight(ld_table, (uint8_t)row);
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
    ldTable_t *ld_table = picoui_table_get_ld_widget(table);

    if (ld_table == 0 || column < 0 || column >= ld_table->columnCount) {
        return -1;
    }

    return (int)ldTableGetItemWidth(ld_table, (uint8_t)column);
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
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;
    int ld_dir;

    if (table == 0 ||
        (dir != PICOUI_NATIVE_NAV_LEFT &&
         dir != PICOUI_NATIVE_NAV_RIGHT &&
         dir != PICOUI_NATIVE_NAV_UP &&
         dir != PICOUI_NATIVE_NAV_DOWN)) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    ld_table = picoui_table_get_ld_widget(table);
    if (backend == NULL || ld_table == NULL) {
        return -1;
    }

    ld_dir = picoui_native_nav_dir_to_ld(dir);
    if (ld_dir < 0) {
        return -1;
    }

    ldTableNavigate(ld_table, (ldNavDir_t)ld_dir);
    return picoui_table_sync_current_cell_local(table, NULL, NULL);
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

    if (table == 0) {
        return region;
    }

    if (picoui_backend_table_get_item_region((void *)table->widget.backend_widget, row, column, &region) != 0) {
        struct picoui_table_region empty = {0};
        return empty;
    }
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
    if (table == 0) {
        return -1;
    }

    if (picoui_backend_table_set_selected_cell(table->widget.backend_widget, row, column) != 0) {
        return -1;
    }
    table->current_row = row;
    table->current_column = column;
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
    if (table == 0) {
        return -1;
    }

    if (picoui_backend_table_set_current_cell(table->widget.backend_widget, row, column) != 0) {
        return -1;
    }
    table->current_row = row;
    table->current_column = column;
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
    int column;

    if (table == 0) {
        return -1;
    }

    if (picoui_table_sync_current_cell_local((struct picoui_table *)table, &row, &column) == 0) {
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
    int row;
    int column;

    if (table == 0) {
        return -1;
    }

    if (picoui_table_sync_current_cell_local((struct picoui_table *)table, &row, &column) == 0) {
        return column;
    }
    return table->current_column;
}
