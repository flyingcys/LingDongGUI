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

#include "table_basic/table_basic.h"
#include "tinyui.h"

#include <string.h>

tinyui_result_t tinyui_demo_table_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *hint;
    tinyui_obj_t *table;
    tinyui_table_props_t table_props;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    result = tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(screen,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(screen, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }
    if (tinyui_window_set_padding(screen, 24, 24, 24, 24) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    memset(&table_props, 0, sizeof(table_props));
    /* border/radius 无 LD 全局通道，不得写入 props 假成功；padding→itemSpace。 */
    table_props.fields = TINYUI_TABLE_FIELD_ROWS | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_WIDTH | TINYUI_TABLE_FIELD_HEIGHT
        | TINYUI_TABLE_FIELD_BG_COLOR | TINYUI_TABLE_FIELD_TEXT_COLOR
        | TINYUI_TABLE_FIELD_PADDING;
    table_props.rows = 3;
    table_props.columns = 3;
    table_props.width = 220;
    table_props.height = 120;
    table_props.bg_color = 0xD8EBD0U;
    table_props.text_color = 0x203020U;
    table_props.padding = 6;

    title = tinyui_label_create(screen);
    hint = tinyui_label_create(screen);
    table = tinyui_table_create_with_props(screen, &table_props);
    if (title == NULL || hint == NULL || table == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Table") != 0
        || tinyui_label_set_text(
               hint, "Editable cell reuses the shared R1 commit boundary.") != 0
        || tinyui_table_set_cell_text(table, 0, 0, "A1") != 0
        || tinyui_table_set_cell_text(table, 0, 1, "B1") != 0
        || tinyui_table_set_cell_text(table, 0, 2, "C1") != 0
        || tinyui_table_set_cell_text(table, 1, 0, "A2") != 0
        || tinyui_table_set_cell_text(table, 1, 1, "Edit") != 0
        || tinyui_table_set_cell_text(table, 1, 2, "C2") != 0
        || tinyui_table_set_cell_text(table, 2, 0, "A3") != 0
        || tinyui_table_set_cell_text(table, 2, 1, "B3") != 0
        || tinyui_table_set_cell_text(table, 2, 2, "C3") != 0
        || tinyui_table_set_cell_editable(table, 1, 1, 1, 16) != 0
        || tinyui_table_set_current_cell(table, 1, 1) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
