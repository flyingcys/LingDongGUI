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
#include "widgets/table.h"
#include "widgets/keyboard.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/ldMsg.h"

#include <string.h>
#include "../../../src/gui/ldBase.h"


static struct tinyui_table *tinyui_table_as_table(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_TABLE)) {
        return 0;
    }
    return (struct tinyui_table *)w;
}

static const struct tinyui_table *tinyui_table_as_table_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_TABLE)) {
        return 0;
    }
    return (const struct tinyui_table *)w;
}

static int s_table_fail_next_set_keyboard_binding = 0;

static arm_2d_font_t *tinyui_table_default_font(void)
{
    return tinyui_resolve_ld_font(0, 12);
}

static int tinyui_table_dims_are_valid(int rows, int columns)
{
    return rows > 0 && rows <= 255 && columns > 0 && columns <= 255;
}

static int tinyui_table_keyboard_binding_is_valid(unsigned int keyboard_binding)
{
    return keyboard_binding > 0U && keyboard_binding <= 0xFFFFU;
}

static void tinyui_table_rollback(struct tinyui_table *table)
{
    if (table == 0) {
        return;
    }
    if (table->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&table->widget);
    } else {
        ldFree(table);
    }
}

static int tinyui_table_props_are_valid(const tinyui_table_props_t *props)
{
    int rows = 1;
    int columns = 1;

    if (props == 0) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_ROWS) != 0) {
        rows = props->rows;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_COLUMNS) != 0) {
        columns = props->columns;
    }
    if (!tinyui_table_dims_are_valid(rows, columns)) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_WIDTH) != 0 && props->width < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_HEIGHT) != 0 && props->height < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_RADIUS) != 0 && props->radius < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_PADDING) != 0 && props->padding < 0) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_KEYBOARD_BINDING) != 0
        && props->keyboard_binding != 0U
        && !tinyui_table_keyboard_binding_is_valid(props->keyboard_binding)) {
        return 0;
    }
    /*
     * Table has no LD global border/radius channel. Refuse those props before
     * create so create_with_props never claims success by writing only the
     * TinyUI host mirror (design §3.5: successful public setters must hit LD).
     */
    if ((props->fields
         & (TINYUI_TABLE_FIELD_BORDER_COLOR | TINYUI_TABLE_FIELD_RADIUS))
        != 0) {
        return 0;
    }
    return 1;
}

/* Apply props text color to every existing cell via real ldTableSetItemColor. */
static int tinyui_table_apply_global_text_color(struct tinyui_table *table,
                                                unsigned int rgb)
{
    ldTable_t *ld_table;
    ldColor text_color;
    uint8_t row;
    uint8_t column;

    if (table == 0 || table->widget.ld_widget == 0
        || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    text_color = (ldColor)tinyui_rgb_to_ld_color(rgb);
    for (row = 0; row < ld_table->rowCount; ++row) {
        for (column = 0; column < ld_table->columnCount; ++column) {
            ldTableItem_t *item = ldTableGetItem(ld_table, row, column);
            ldColor item_bg = __RGB(255, 255, 255);

            if (item != 0) {
                item_bg = item->itemBgColor;
            }
            ldTableSetItemColor(ld_table, row, column, text_color, item_bg);
        }
    }
    table->widget.text_color = rgb;
    return 0;
}

static int tinyui_table_sync_current_cell_local(struct tinyui_table *table,
                                                int *row_out,
                                                int *column_out);

static void *tinyui_runtime_internal_table_ld_init(void *ctx,
                                  struct ld_scene_t *scene,
                                  uint16_t name_id,
                                  uint16_t parent_name_id)
{
    const struct tinyui_table *table = (const struct tinyui_table *)ctx;

    if (table == NULL) {
        return 0;
    }

    return ldTable_init(scene,
                        NULL,
                        name_id,
                        parent_name_id,
                        0,
                        0,
                        220,
                        120,
                        (uint8_t)table->row_count,
                        (uint8_t)table->column_count,
                        4);
}

static bool tinyui_table_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct tinyui_widget *w;
    struct tinyui_table *table;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    int row = 0;
    int column = 0;
    int was_focus_owner = 0;

    if (msg.ptSender == NULL) {
        return false;
    }

    w = tinyui_runtime_internal_widget_from_ld_scene(scene, msg.ptSender);
    if (w == NULL) {
        return false;
    }

    table = (struct tinyui_table *)w;
    if (w->kind != TINYUI_BACKEND_WIDGET_TABLE || w->ld_widget == NULL) {
        return false;
    }
    ld_table = (ldTable_t *)w->ld_widget;

    (void)tinyui_table_sync_current_cell_local(table, &row, &column);
    item = ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);

    if (msg.signal == SIGNAL_PRESS) {
        was_focus_owner = tinyui_runtime_internal_widget_is_focus_owner(&table->widget);
        (void)tinyui_runtime_internal_widget_claim_backend_focus(w);
        if (item != NULL && item->isEditable && (item->isEditing || was_focus_owner)) {
            w->edit_result_on_finish = TINYUI_EDIT_RESULT_COMMIT;
            (void)tinyui_runtime_internal_widget_claim_editing(&table->widget);
        }
        return false;
    }

    if (msg.signal != SIGNAL_FINISHED) {
        return false;
    }

    if (item != NULL) {
        item->isEditing = false;
    }
    (void)tinyui_runtime_internal_widget_mark_edit_result(&table->widget,
                                         (enum tinyui_edit_result)w->edit_result_on_finish);
    (void)tinyui_runtime_internal_widget_release_editing(&table->widget);
    w->edit_result_on_finish = TINYUI_EDIT_RESULT_NONE;
    return false;
}

static int tinyui_table_bind_native_slot(struct tinyui_table *table)
{
    ldTable_t *ld_table;

    if (table == NULL || table->widget.ld_widget == NULL) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_table, SIGNAL_PRESS, tinyui_table_native_slot)) {
        return -1;
    }
    if (!ldMsgConnect(ld_table, SIGNAL_FINISHED, tinyui_table_native_slot)) {
        return -1;
    }
    return 0;
}

static int tinyui_table_sync_current_cell_local(struct tinyui_table *table,
                                                int *row_out,
                                                int *column_out)
{
    ldTable_t *ld_table;
    int row;
    int column;

    if (table == NULL || table->widget.ld_widget == NULL) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
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

tinyui_obj_t *tinyui_table_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "table";
    if (parent_w == 0) { return 0; }
    int rows = 1;
    int columns = 1;

    struct tinyui_table *table;
    struct tinyui_table ctx = {0};

    if (parent_w == 0 || id == 0 || !tinyui_table_dims_are_valid(rows, columns)) {
        return 0;
    }

    if (parent_w->ld_widget == 0 || parent_w->owner == 0) {
        return 0;
    }

    ctx.row_count = rows;
    ctx.column_count = columns;
    table = (struct tinyui_table *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                             TINYUI_BACKEND_WIDGET_TABLE,
                                                             tinyui_runtime_internal_table_ld_init,
                                                             &ctx,
                                                             sizeof(*table));
    if (table == 0) {
        return 0;
    }

    table->id = id;
    table->widget.visible    = 1;
    table->widget.enabled    = 1;
    table->row_count = rows;
    table->column_count = columns;
    table->current_row = 0;
    table->current_column = 0;
    if (tinyui_table_bind_native_slot(table) != 0) {
        tinyui_table_rollback(table);
        return 0;
    }
    return (tinyui_obj_t *)table;
}

/**
 * @brief Create table widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_table_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_table_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_table *table;
    struct tinyui_widget *parent_w;
    const char *id = "table";
    int rows = 1;
    int columns = 1;
    struct tinyui_table ctx = {0};

    if (props == 0) {
        return tinyui_table_create(parent);
    }
    if (!tinyui_table_props_are_valid(props)) {
        return 0;
    }

    parent_w = (struct tinyui_widget *)(void *)parent;
    if (parent_w == 0 || parent_w->ld_widget == 0 || parent_w->owner == 0) {
        return 0;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_ROWS) != 0) {
        rows = props->rows;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_COLUMNS) != 0) {
        columns = props->columns;
    }
    if (!tinyui_table_dims_are_valid(rows, columns)) {
        return 0;
    }

    ctx.row_count = rows;
    ctx.column_count = columns;
    table = (struct tinyui_table *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                             TINYUI_BACKEND_WIDGET_TABLE,
                                                             tinyui_runtime_internal_table_ld_init,
                                                             &ctx,
                                                             sizeof(*table));
    if (table == 0) {
        return 0;
    }
    table->id = id;
    table->widget.visible = 1;
    table->widget.enabled = 1;
    table->row_count = rows;
    table->column_count = columns;
    table->current_row = 0;
    table->current_column = 0;
    if (tinyui_table_bind_native_slot(table) != 0) {
        tinyui_table_rollback(table);
        return 0;
    }
    obj = (tinyui_obj_t *)table;

    if ((props->fields & TINYUI_TABLE_FIELD_ID) != 0) {
        (void)props->id;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_USER_DATA) != 0) {
        if (tinyui_runtime_internal_widget_set_user_data(&table->widget, props->user_data) != 0) {
            tinyui_table_rollback(table);
            return 0;
        }
    }
    if ((props->fields & TINYUI_TABLE_FIELD_STYLE_CLASS) != 0) {
        if (tinyui_runtime_internal_widget_set_style_class(&table->widget, props->style_class) != 0) {
            tinyui_table_rollback(table);
            return 0;
        }
    }
    if ((props->fields & TINYUI_TABLE_FIELD_WIDTH) != 0 || (props->fields & TINYUI_TABLE_FIELD_HEIGHT) != 0) {
        int w = tinyui_runtime_internal_widget_get_width(&table->widget);
        int h = tinyui_runtime_internal_widget_get_height(&table->widget);
        if (w < 0) { w = 0; }
        if (h < 0) { h = 0; }
        if ((props->fields & TINYUI_TABLE_FIELD_WIDTH) != 0) { w = props->width; }
        if ((props->fields & TINYUI_TABLE_FIELD_HEIGHT) != 0) { h = props->height; }
        if (tinyui_runtime_internal_widget_set_size(&table->widget, w, h) != 0) {
            tinyui_table_rollback(table);
            return 0;
        }
    }
    /* Table has native bg color; generic leaf style helpers reject TABLE kind. */
    if ((props->fields & TINYUI_TABLE_FIELD_BG_COLOR) != 0) {
        if (tinyui_table_set_bg_color((tinyui_obj_t *)table, props->bg_color) != 0) {
            tinyui_table_rollback(table);
            return 0;
        }
        table->widget.bg_color = props->bg_color;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_TEXT_COLOR) != 0) {
        /* Map to every cell via ldTableSetItemColor; no silent host-only cache. */
        if (tinyui_table_apply_global_text_color(table, props->text_color) != 0) {
            tinyui_table_rollback(table);
            return 0;
        }
    }
    /* BORDER_COLOR / RADIUS rejected in tinyui_table_props_are_valid. */
    if ((props->fields & TINYUI_TABLE_FIELD_PADDING) != 0) {
        /* Public "padding" for table maps to LD itemSpace (cell gap). */
        if (tinyui_table_set_item_space((tinyui_obj_t *)table,
                                       (unsigned int)props->padding)
            != 0) {
            tinyui_table_rollback(table);
            return 0;
        }
        table->widget.padding = props->padding;
    }
    if ((props->fields & TINYUI_TABLE_FIELD_KEYBOARD_BINDING) != 0) {
        if (props->keyboard_binding != 0U) {
            if (tinyui_table_set_keyboard_binding((tinyui_obj_t *)table, props->keyboard_binding) != 0) {
                tinyui_table_rollback(table);
                return 0;
            }
        }
    }
    return obj;
}



/**
 * @brief Set keyboard of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int tinyui_table_set_keyboard(tinyui_obj_t *table_obj, unsigned int keyboard_binding)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    return tinyui_table_set_keyboard_binding((tinyui_obj_t *)table, keyboard_binding);
}

/**
 * @brief Set keyboard binding of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return 0 on success, -1 on failure
 */

int tinyui_table_set_keyboard_binding(tinyui_obj_t *table_obj, unsigned int keyboard_binding)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || !tinyui_table_keyboard_binding_is_valid(keyboard_binding)) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }

    if (s_table_fail_next_set_keyboard_binding != 0) {
        s_table_fail_next_set_keyboard_binding = 0;
        return -1;
    }

    ldTableSetKeyboard(ld_table, (uint16_t)keyboard_binding);
    table->widget.value = (int)keyboard_binding;
    if (ld_table->kbNameId != (uint16_t)keyboard_binding) {
        return -1;
    }
    table->keyboard_binding = keyboard_binding;
    return 0;
}

int tinyui_table_set_keyboard_widget(tinyui_obj_t *table_obj, tinyui_obj_t *keyboard)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    if (table == 0 ||
        keyboard == 0 ||
        table->widget.owner == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->owner != table->widget.owner ||
        ((struct tinyui_widget *)(void *)keyboard)->ld_widget == 0 ||
        ((struct tinyui_widget *)(void *)keyboard)->kind != TINYUI_BACKEND_WIDGET_KEYBOARD) {
        return -1;
    }

    return tinyui_table_set_keyboard_binding((tinyui_obj_t *)table, ((struct tinyui_widget *)(void *)keyboard)->ld_name_id);
}

/**
 * @brief Get keyboard binding of table widget
 *
 * @param[in] table table
 * @param[in] keyboard_binding keyboard binding
 * @return -1 on failure
 */

int tinyui_table_get_keyboard_binding(const tinyui_obj_t *table_obj, unsigned int *keyboard_binding)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || keyboard_binding == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table != 0 && table->widget.kind == TINYUI_BACKEND_WIDGET_TABLE) {
        *keyboard_binding = (unsigned int)ld_table->kbNameId;
        return 0;
    }

    if (table->keyboard_binding == 0U) {
        return -1;
    }

    *keyboard_binding = table->keyboard_binding;
    return 0;
}

int tinyui_table_set_item_space(tinyui_obj_t *table_obj, unsigned int item_space)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || item_space > 255U) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }

    ld_table->itemSpace = (uint8_t)item_space;
    ld_table->use_as__ldBase_t.isDirtyRegionUpdate = true;
    return 0;
}

void tinyui_table_test_fail_next_set_keyboard_binding(void)
{
    s_table_fail_next_set_keyboard_binding = 1;
}

void tinyui_table_test_reset_state(void)
{
    s_table_fail_next_set_keyboard_binding = 0;
}

/**
 * @brief Show keyboard for current editable table cell
 *
 * @param[in] table table
 * @return 0 on success, -1 on failure
 */

int tinyui_table_show_keyboard(tinyui_obj_t *table_obj)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    struct tinyui_app *app_state;
    ldTable_t *ld_table;
    ldTableItem_t *item;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }
    app_state = table->widget.owner;
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

int tinyui_runtime_internal_tabel_show_keyboard(tinyui_obj_t *table_obj)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    return tinyui_table_show_keyboard((tinyui_obj_t *)table);
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

int tinyui_table_set_cell_text(tinyui_obj_t *table_obj, int row, int column, const char *text)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;
    ldTableItem_t *item;

    if (table == 0 || text == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }
    if (row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        tinyui_runtime_set_last_result(TINYUI_ERROR_OUT_OF_RANGE);
        return -1;
    }

    item = ldTableGetItem(ld_table, (uint8_t)row, (uint8_t)column);
    if (item != 0 && item->ptFont == 0) {
        ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, tinyui_table_default_font());
    }
    ldTableSetItemText(ld_table, (uint8_t)row, (uint8_t)column, (uint8_t *)text);
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

int tinyui_table_set_item_text(tinyui_obj_t *table_obj, int row, int column, const char *text)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    return tinyui_table_set_cell_text((tinyui_obj_t *)table, row, column, text);
}

/**
 * @brief Get cell text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char * tinyui_table_get_cell_text(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return 0; }

    ldTable_t *ld_table;

    if (table == 0) {
        return 0;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return 0;
    }

    return (const char *)ldTableGetItemText(ld_table, (uint8_t)row, (uint8_t)column);
}

/**
 * @brief Get item text of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

const char * tinyui_table_get_item_text(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return 0; }

    return tinyui_table_get_cell_text((tinyui_obj_t *)table, row, column);
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

int tinyui_table_set_cell_editable(tinyui_obj_t *table_obj, int row, int column, int editable, unsigned int text_max)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
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
        ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, tinyui_table_default_font());
    }
    return 0;
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

int tinyui_table_set_item_editable(tinyui_obj_t *table_obj, int row, int column, int editable, unsigned int text_max)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    return tinyui_table_set_cell_editable((tinyui_obj_t *)table, row, column, editable, text_max);
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

int tinyui_table_set_item_image(tinyui_obj_t *table_obj, int row, int column, int x, int y, struct tinyui_image_source *source, unsigned int mask_color)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || source == 0 || source->kind == TINYUI_IMAGE_SOURCE_EMPTY ||
        tinyui_image_source_get_image_tile(source) == 0 ||
        tinyui_image_source_get_mask_tile(source) == 0 ||
        mask_color > 0xFFFFFFU) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemImage(ld_table,
                        (uint8_t)row,
                        (uint8_t)column,
                        (int16_t)x,
                        (int16_t)y,
                        tinyui_image_source_get_image_tile(source),
                        tinyui_image_source_get_mask_tile(source),
                        (ldColor)mask_color);
    return 0;
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

int tinyui_table_set_item_button(tinyui_obj_t *table_obj, int row, int column, int x, int y, struct tinyui_image_source *release_source, unsigned int release_mask_color, struct tinyui_image_source *press_source, unsigned int press_mask_color, int checkable)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || release_source == 0 || press_source == 0 ||
        release_source->kind == TINYUI_IMAGE_SOURCE_EMPTY ||
        press_source->kind == TINYUI_IMAGE_SOURCE_EMPTY ||
        tinyui_image_source_get_image_tile(release_source) == 0 ||
        tinyui_image_source_get_mask_tile(release_source) == 0 ||
        tinyui_image_source_get_image_tile(press_source) == 0 ||
        tinyui_image_source_get_mask_tile(press_source) == 0 ||
        release_mask_color > 0xFFFFFFU || press_mask_color > 0xFFFFFFU) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemButton(ld_table,
                         (uint8_t)row,
                         (uint8_t)column,
                         (int16_t)x,
                         (int16_t)y,
                         tinyui_image_source_get_image_tile(release_source),
                         tinyui_image_source_get_mask_tile(release_source),
                         (ldColor)release_mask_color,
                         tinyui_image_source_get_image_tile(press_source),
                         tinyui_image_source_get_mask_tile(press_source),
                         (ldColor)press_mask_color,
                         checkable != 0);
    return 0;
}

/**
 * @brief Set excel type of table widget
 *
 * @return -1 on failure
 */

int tinyui_table_set_excel_type(tinyui_obj_t *table_obj)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }

    ldTableSetExcelType(ld_table, tinyui_table_default_font());
    return 0;
}

/**
 * @brief Set background color of table widget
 *
 * @param[in] table table
 * @param[in] bg_color Background color
 * @return 0 on success, -1 on failure
 */

int tinyui_table_set_background_color(tinyui_obj_t *table_obj, unsigned int bg_color)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    return tinyui_table_set_bg_color((tinyui_obj_t *)table, bg_color);
}

/**
 * @brief Set align grid of table widget
 *
 * @param[in] table table
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int tinyui_table_set_align_grid(tinyui_obj_t *table_obj, int enabled)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
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

int tinyui_table_set_item_width(tinyui_obj_t *table_obj, int column, int width)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || width <= 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        column < 0 || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemWidth(ld_table, (uint8_t)column, (int16_t)width);
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

int tinyui_table_set_item_height(tinyui_obj_t *table_obj, int row, int height)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || height <= 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || row >= ld_table->rowCount) {
        return -1;
    }

    ldTableSetItemHeight(ld_table, (uint8_t)row, (int16_t)height);
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

int tinyui_table_set_item_color(tinyui_obj_t *table_obj, int row, int column, unsigned int text_color, unsigned int bg_color)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || text_color > 0xFFFFFFU || bg_color > 0xFFFFFFU) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 || row >= ld_table->rowCount ||
        column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemColor(ld_table, (uint8_t)row, (uint8_t)column, (ldColor)text_color, (ldColor)bg_color);
    return 0;
}

/**
 * @brief Set bg color of table widget
 *
 * @param[in] table table
 * @param[in] bg_color Background color
 * @return -1 on failure
 */

int tinyui_table_set_bg_color(tinyui_obj_t *table_obj, unsigned int bg_color)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || bg_color > 0xFFFFFFU) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE) {
        return -1;
    }

    ldTableSetBackgroundColor(ld_table, (ldColor)bg_color);
    return 0;
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

int tinyui_table_set_item_static_text(tinyui_obj_t *table_obj, int row, int column, const char *text)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0 || text == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemStaticText(ld_table, (uint8_t)row, (uint8_t)column, (uint8_t *)text);
    return 0;
}

/**
 * @brief Set item font of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int tinyui_table_set_item_font(tinyui_obj_t *table_obj, int row, int column)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemFont(ld_table, (uint8_t)row, (uint8_t)column, tinyui_table_default_font());
    return 0;
}

/**
 * @brief Get align grid of table widget
 *
 * @param[in] table table
 * @return -1 on failure
 */

int tinyui_table_get_align_grid(const tinyui_obj_t *table_obj)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

unsigned int tinyui_table_get_background_color(const tinyui_obj_t *table_obj)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

void * tinyui_table_get_item(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return 0; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

int tinyui_table_set_item_align(tinyui_obj_t *table_obj, int row, int column, enum tinyui_align align)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;
    arm_2d_align_t ld_align;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ld_align = (arm_2d_align_t)tinyui_align_to_arm2d(align);
    ldTableSetItemAlign(ld_table, (uint8_t)row, (uint8_t)column, ld_align);
    return 0;
}

/**
 * @brief Get item align of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int tinyui_table_get_item_align(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;
    arm_2d_align_t ld_align;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ld_align = ldTableGetItemAlign(ld_table, (uint8_t)row, (uint8_t)column);
    switch (ld_align) {
    case ARM_2D_ALIGN_LEFT:
        return TINYUI_ALIGN_START;
    case ARM_2D_ALIGN_CENTRE:
        return TINYUI_ALIGN_CENTER;
    case ARM_2D_ALIGN_RIGHT:
        return TINYUI_ALIGN_END;
    default:
        return -1;
    }
}

/**
 * @brief Get item editable of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return -1 on failure
 */

int tinyui_table_get_item_editable(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    return ldTableGetItemEditable(ld_table, (uint8_t)row, (uint8_t)column) ? 1 : 0;
}

/**
 * @brief Get item font of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 */

void * tinyui_table_get_item_font(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return 0; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

int tinyui_table_get_item_height(const tinyui_obj_t *table_obj, int row)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

unsigned int tinyui_table_get_item_text_color(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

unsigned int tinyui_table_get_item_background_color(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

int tinyui_table_get_item_width(const tinyui_obj_t *table_obj, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table = (ldTable_t *)table->widget.ld_widget;

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

int tinyui_table_navigate(tinyui_obj_t *table_obj, tinyui_nav_dir_t dir)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;
    int ld_dir;
    enum tinyui_native_nav_dir native_dir;

    /* Public API uses tinyui_nav_dir_t; values match native nav dirs. */
    if (dir != TINYUI_NAV_LEFT &&
        dir != TINYUI_NAV_RIGHT &&
        dir != TINYUI_NAV_UP &&
        dir != TINYUI_NAV_DOWN) {
        return -1;
    }
    native_dir = (enum tinyui_native_nav_dir)dir;

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == NULL) {
        return -1;
    }

    ld_dir = tinyui_native_nav_dir_to_ld(native_dir);
    if (ld_dir < 0) {
        return -1;
    }

    ldTableNavigate(ld_table, (ldNavDir_t)ld_dir);
    return tinyui_table_sync_current_cell_local(table, NULL, NULL);
}

/**
 * @brief Get item region of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return Pointer to the object
 */

struct tinyui_table_region tinyui_table_get_item_region(const tinyui_obj_t *table_obj, int row, int column)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    ldTable_t *ld_table;
    struct tinyui_table_region region = {0};
    arm_2d_region_t native_region;

    if (table == 0) {
        return region;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || table->widget.kind != TINYUI_BACKEND_WIDGET_TABLE ||
        row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return region;
    }

    native_region = ldTableGetItemRegion(ld_table, (uint8_t)row, (uint8_t)column);
    region.x = native_region.tLocation.iX;
    region.y = native_region.tLocation.iY;
    region.width = native_region.tSize.iWidth;
    region.height = native_region.tSize.iHeight;
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

int tinyui_table_set_selected_cell(tinyui_obj_t *table_obj, int row, int column)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemSelect(ld_table, (uint8_t)row, (uint8_t)column, true);
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

int tinyui_table_set_item_select(tinyui_obj_t *table_obj, int row, int column, int selected)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    if (selected == 0) {
        return -1;
    }

    return tinyui_table_set_selected_cell((tinyui_obj_t *)table, row, column);
}

/**
 * @brief Set current cell of table widget
 *
 * @param[in] table table
 * @param[in] row Row index
 * @param[in] column Column index
 * @return 0 on success, -1 on failure
 */

int tinyui_table_set_current_cell(tinyui_obj_t *table_obj, int row, int column)
{
    struct tinyui_table *table = tinyui_table_as_table(table_obj);
    if (table == 0) { return -1; }

    ldTable_t *ld_table;

    if (table == 0) {
        return -1;
    }

    ld_table = (ldTable_t *)table->widget.ld_widget;
    if (ld_table == 0 || row < 0 || column < 0 ||
        row >= ld_table->rowCount || column >= ld_table->columnCount) {
        return -1;
    }

    ldTableSetItemSelect(ld_table, (uint8_t)row, (uint8_t)column, true);
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

int tinyui_table_get_current_row(const tinyui_obj_t *table_obj)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    int row;
    int column;

    if (table == 0) {
        return -1;
    }

    if (tinyui_table_sync_current_cell_local((struct tinyui_table *)table, &row, &column) == 0) {
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

int tinyui_table_get_current_column(const tinyui_obj_t *table_obj)
{
    const struct tinyui_table *table = tinyui_table_as_table_const(table_obj);
    if (table == 0) { return -1; }

    int row;
    int column;

    if (table == 0) {
        return -1;
    }

    if (tinyui_table_sync_current_cell_local((struct tinyui_table *)table, &row, &column) == 0) {
        return column;
    }
    return table->current_column;
}
