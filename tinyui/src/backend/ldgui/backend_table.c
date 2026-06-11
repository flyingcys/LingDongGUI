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

#include "backend.h"
#include "internal.h"
#include "runtime_bridge.h"
#include "ldBase.h"
#include "ldTable.h"

#include <stdlib.h>

extern const arm_2d_a1_font_t ARM_2D_FONT_6x8;

static ldTable_t *picoui_backend_table_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldTable_t *)widget->ld_widget;
}

static int picoui_backend_table_align_to_ld(enum picoui_align align, arm_2d_align_t *out)
{
    if (out == NULL) {
        return -1;
    }

    switch (align) {
    case PICOUI_ALIGN_START:
        *out = ARM_2D_ALIGN_LEFT;
        return 0;
    case PICOUI_ALIGN_CENTER:
        *out = ARM_2D_ALIGN_CENTRE;
        return 0;
    case PICOUI_ALIGN_END:
        *out = ARM_2D_ALIGN_RIGHT;
        return 0;
    default:
        return -1;
    }
}

static int picoui_backend_table_sync_host_current_cell(struct picoui_backend_widget *backend,
                                                       int *row_out,
                                                       int *column_out)
{
    struct picoui_table *table;
    ldTable_t *ld_table;
    int row;
    int column;

    if (backend == NULL || backend->host_widget == NULL) {
        return -1;
    }

    ld_table = picoui_backend_table_get_ld(backend);
    if (ld_table == NULL) {
        return -1;
    }

    row = (int)ld_table->currentRow;
    column = (int)ld_table->currentColumn;
    table = (struct picoui_table *)backend->host_widget;
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

static bool picoui_backend_table_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
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
    ld_table = picoui_backend_table_get_ld(backend);
    if (ld_table == NULL) {
        return false;
    }

    backend->last_native_signal = msg.signal;
    backend->last_native_value = msg.value;

    (void)picoui_backend_table_sync_host_current_cell(backend, &row, &column);
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

/**
 * @brief Set keyboard binding of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_keyboard_binding(void *backend_widget, unsigned int keyboard_binding)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || keyboard_binding == 0U || keyboard_binding > 0xFFFFU) {
        return -1;
    }

    ldTableSetKeyboard(ld_table, (uint16_t)keyboard_binding);
    return 0;
}

/**
 * @brief Get keyboard binding from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_get_keyboard_binding(void *backend_widget, unsigned int *keyboard_binding)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || keyboard_binding == NULL) {
        return -1;
    }

    *keyboard_binding = (unsigned int)ld_table->kbNameId;
    return 0;
}

/**
 * @brief Set cell text of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_cell_text(void *backend_widget, int row, int column, const char *text)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);
    ldTableItem_t *item;

    if (ld_table == NULL || text == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    item = ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);
    if (item != NULL && item->ptFont == NULL) {
        ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    }
    ldTableSetItemText(ld_table, (uint8_t)row, (uint8_t)column, (uint8_t *)text);
    return 0;
}

/**
 * @brief Get cell text from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char *picoui_backend_table_get_cell_text(void *backend_widget, int row, int column)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return NULL;
    }

    return (const char *)ldTableGetItemText(ld_table, (uint8_t)row, (uint8_t)column);
}

/**
 * @brief Set cell editable of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] editable editable
 * @param[in] text_max text max
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_cell_editable(void *backend_widget,
                                           int row,
                                           int column,
                                           int editable,
                                           unsigned int text_max)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount ||
        text_max > 255U) {
        return -1;
    }

    ldTableSetItemEditable(ld_table,
                           (uint8_t)row,
                           (uint8_t)column,
                           editable != 0,
                           (uint8_t)text_max);
    if (editable != 0) {
        ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    }
    return 0;
}

/**
 * @brief Set item image of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @param[in] source Image source
 * @param[in] mask_color mask color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_image(void *backend_widget,
                                        int row,
                                        int column,
                                        int x,
                                        int y,
                                        struct picoui_image_source *source,
                                        unsigned int mask_color)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || source == NULL || source->img_tile == NULL || source->mask_tile == NULL ||
        row < 0 || column < 0 || row >= ld_table->rowCount || column >= ld_table->columnCount ||
        mask_color > 0xFFFFFFU) {
        return -1;
    }

    ldTableSetItemImage(ld_table,
                        (uint8_t)row,
                        (uint8_t)column,
                        (int16_t)x,
                        (int16_t)y,
                        source->img_tile,
                        source->mask_tile,
                        (ldColor)mask_color);
    return 0;
}

/**
 * @brief Set item button of table backend
 *
 * @param[in] backend_widget backend widget
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

int picoui_backend_table_set_item_button(void *backend_widget,
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
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || release_source == NULL || press_source == NULL ||
        release_source->img_tile == NULL || release_source->mask_tile == NULL ||
        press_source->img_tile == NULL || press_source->mask_tile == NULL ||
        row < 0 || column < 0 || row >= ld_table->rowCount || column >= ld_table->columnCount ||
        release_mask_color > 0xFFFFFFU || press_mask_color > 0xFFFFFFU) {
        return -1;
    }

    ldTableSetItemButton(ld_table,
                         (uint8_t)row,
                         (uint8_t)column,
                         (int16_t)x,
                         (int16_t)y,
                         release_source->img_tile,
                         release_source->mask_tile,
                         (ldColor)release_mask_color,
                         press_source->img_tile,
                         press_source->mask_tile,
                         (ldColor)press_mask_color,
                         checkable != 0);
    return 0;
}

/**
 * @brief Set excel type of table backend
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_excel_type(void *backend_widget)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL) {
        return -1;
    }

    ldTableSetExcelType(ld_table, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    return 0;
}

/**
 * @brief Set item width of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] column Column index
 * @param[in] width Width in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_width(void *backend_widget, int column, int width)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || column < 0 || column >= ld_table->columnCount || width <= 0) {
        return -1;
    }

    ldTableSetItemWidth(ld_table, (uint8_t)column, (int16_t)width);
    return 0;
}

/**
 * @brief Set item height of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] height Height in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_height(void *backend_widget, int row, int height)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || row >= ld_table->rowCount || height <= 0) {
        return -1;
    }

    ldTableSetItemHeight(ld_table, (uint8_t)row, (int16_t)height);
    return 0;
}

/**
 * @brief Set item color of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text_color Text color
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_color(void *backend_widget,
                                        int row,
                                        int column,
                                        unsigned int text_color,
                                        unsigned int bg_color)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 || row >= ld_table->rowCount ||
        column >= ld_table->columnCount || text_color > 0xFFFFFFU || bg_color > 0xFFFFFFU) {
        return -1;
    }

    ldTableSetItemColor(ld_table, (uint8_t)row, (uint8_t)column, (ldColor)text_color, (ldColor)bg_color);
    return 0;
}

/**
 * @brief Set bg color of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_bg_color(void *backend_widget, unsigned int bg_color)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || bg_color > 0xFFFFFFU) {
        return -1;
    }

    ldTableSetBackgroundColor(ld_table, (ldColor)bg_color);
    return 0;
}

/**
 * @brief Set item static text of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_static_text(void *backend_widget, int row, int column, const char *text)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || text == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemStaticText(ld_table, (uint8_t)row, (uint8_t)column, (uint8_t *)text);
    return 0;
}

/**
 * @brief Set item font of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_font(void *backend_widget, int row, int column)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 || row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    return 0;
}

/**
 * @brief Set item align of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_item_align(void *backend_widget,
                                        int row,
                                        int column,
                                        enum picoui_align align)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);
    arm_2d_align_t ld_align;

    if (ld_table == NULL || row < 0 || column < 0 || row >= ld_table->rowCount || column >= ld_table->columnCount ||
        picoui_backend_table_align_to_ld(align, &ld_align) != 0) {
        return -1;
    }

    ldTableSetItemAlign(ld_table, (uint8_t)row, (uint8_t)column, ld_align);
    return 0;
}

/**
 * @brief Get item align from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_backend_table_get_item_align(void *backend_widget, int row, int column)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);
    arm_2d_align_t ld_align;

    if (ld_table == NULL || row < 0 || column < 0 || row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ld_align = ldTableGetItemAlign(ld_table, (uint8_t)row, (uint8_t)column);
    switch (ld_align) {
    case ARM_2D_ALIGN_LEFT:
        return PICOUI_ALIGN_START;
    case ARM_2D_ALIGN_CENTRE:
        return PICOUI_ALIGN_CENTER;
    case ARM_2D_ALIGN_RIGHT:
        return PICOUI_ALIGN_END;
    default:
        return -1;
    }
}

/**
 * @brief Get item editable from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_backend_table_get_item_editable(void *backend_widget, int row, int column)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (ld_table == NULL || row < 0 || column < 0 || row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    return ldTableGetItemEditable(ld_table, (uint8_t)row, (uint8_t)column) ? 1 : 0;
}

/**
 * @brief table: navigate
 *
 * @param[in] backend_widget backend widget
 * @param[in] dir dir
 * @return -1 on failure
 */

int picoui_backend_table_navigate(void *backend_widget, enum picoui_native_nav_dir dir)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);
    int ld_dir;

    if (backend == NULL || ld_table == NULL) {
        return -1;
    }

    ld_dir = picoui_native_nav_dir_to_ld(dir);
    if (ld_dir < 0) {
        return -1;
    }

    ldTableNavigate(ld_table, (ldNavDir_t)ld_dir);
    return picoui_backend_table_sync_host_current_cell(backend, NULL, NULL);
}

/**
 * @brief Get item region from table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @param[in] region_out region out
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_get_item_region(void *backend_widget, int row, int column, void *region_out)
{
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);
    struct picoui_table_region *out = region_out;
    arm_2d_region_t region;

    if (ld_table == NULL || out == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    region = ldTableGetItemRegion(ld_table, (uint8_t)row, (uint8_t)column);
    out->x = region.tLocation.iX;
    out->y = region.tLocation.iY;
    out->width = region.tSize.iWidth;
    out->height = region.tSize.iHeight;
    return 0;
}

/**
 * @brief Set selected cell of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_set_selected_cell(void *backend_widget, int row, int column)
{
    return picoui_backend_table_set_current_cell(backend_widget, row, column);
}

/**
 * @brief Set current cell of table backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int picoui_backend_table_set_current_cell(void *backend_widget, int row, int column)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldTable_t *ld_table = picoui_backend_table_get_ld(backend_widget);

    if (backend == NULL || ld_table == NULL || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemSelect(ld_table, (uint8_t)row, (uint8_t)column, true);
    return picoui_backend_table_sync_host_current_cell(backend, NULL, NULL);
}

/**
 * @brief table: sync current cell
 *
 * @param[in] table table
 * @param[in] row_out row out
 * @param[in] column_out column out
 * @return -1 on failure
 */

int picoui_backend_table_sync_current_cell(struct picoui_table *table, int *row_out, int *column_out)
{
    struct picoui_backend_widget *backend;

    if (table == NULL || table->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    return picoui_backend_table_sync_host_current_cell(backend, row_out, column_out);
}

/**
 * @brief table: bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_table_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldTable_t *ld_table;

    if (backend == NULL) {
        return -1;
    }

    ld_table = picoui_backend_table_get_ld(backend_widget);
    if (ld_table == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_table, SIGNAL_PRESS, picoui_backend_table_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_table, SIGNAL_FINISHED, picoui_backend_table_native_slot)) {
        return -1;
    }
    return 0;
}
