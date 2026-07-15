/*
 * TinyUI table unit tests — M3 Task 4 L3/L4 harness.
 *
 * Validates real ldTable_* mapping for cell text/edit, row×column bounds,
 * keyboard binding, navigation, and native getters. No fake success.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/ldMsg.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "internal.h"
#include "resource/image_source.h"
#include "widgets/keyboard.h"
#include "widgets/table.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void tinyui_table_test_fail_next_set_keyboard_binding(void);
void tinyui_table_test_reset_state(void);

static struct tinyui_widget g_disposed_backend_snapshot;
static int g_disposed_backend_valid = 0;
static const char *g_self_binary_path = 0;

void tinyui_test_capture_destroyed_widget_snapshot(const struct tinyui_widget *widget)
{
    if (widget == 0) {
        memset(&g_disposed_backend_snapshot, 0, sizeof(g_disposed_backend_snapshot));
        g_disposed_backend_valid = 0;
        return;
    }

    g_disposed_backend_snapshot = *widget;
    g_disposed_backend_valid = 1;
}

static const struct tinyui_widget *tinyui_table_test_last_disposed_backend(void)
{
    if (g_disposed_backend_valid == 0) {
        return 0;
    }
    return &g_disposed_backend_snapshot;
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    int status;

    assert(g_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", g_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len >= symbol_len &&
            strcmp(line + line_len - symbol_len, symbol) == 0) {
            assert(!"unexpected backend_ symbol still present in test binary");
        }
    }
    status = pclose(pipe);
    assert(status != -1);
}

static void bind_test_tiles(tinyui_image_source_t *source,
                            arm_2d_tile_t *img_tile,
                            arm_2d_tile_t *mask_tile)
{
    assert(source != 0);
    memset(source, 0, sizeof(*source));
    source->kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    if (img_tile != 0) {
        memcpy(source->_image_private, img_tile, sizeof(*img_tile));
    }
    if (mask_tile != 0) {
        memcpy(source->_mask_private, mask_tile, sizeof(*mask_tile));
    }
}

static tinyui_obj_t *create_table_3x3(tinyui_obj_t *root)
{
    tinyui_table_props_t props;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TABLE_FIELD_ROWS | TINYUI_TABLE_FIELD_COLUMNS;
    props.rows = 3;
    props.columns = 3;
    return tinyui_table_create_with_props(root, &props);
}

static ldTable_t *table_ld(tinyui_obj_t *table)
{
    struct tinyui_widget *backend = (struct tinyui_widget *)(void *)table;

    assert(table != 0);
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_TABLE);
    return (ldTable_t *)backend->ld_widget;
}

static void ensure_msg_queue(struct tinyui_widget *backend)
{
    struct tinyui_app *app_state;

    assert(backend != 0);
    app_state = backend->owner;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    if (app_state->ld_scene->ptMsgQueue == 0) {
        assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    }
}

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void test_table_create_and_ld_mapping(tinyui_obj_t *root)
{
    tinyui_obj_t *table = create_table_3x3(root);
    struct tinyui_widget *backend;
    ldBase_t *ld_base;
    ldTable_t *ld_table;
    tinyui_obj_t *default_table;

    assert(table != 0);
    backend = (struct tinyui_widget *)(void *)table;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_TABLE);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeTable);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table->rowCount == 3);
    assert(ld_table->columnCount == 3);
    assert(ld_table->itemSpace == 4);

    default_table = tinyui_table_create(root);
    assert(default_table != 0);
    ld_table = table_ld(default_table);
    assert(ld_table->rowCount == 1);
    assert(ld_table->columnCount == 1);
    assert(tinyui_table_create(0) == 0);
}

static void test_table_create_with_props_pushes_fields(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    struct tinyui_widget *kb_backend;
    tinyui_table_props_t props;
    tinyui_obj_t *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    unsigned int keyboard_binding = 0U;
    int cookie = 42;

    assert(keyboard != 0);
    kb_backend = (struct tinyui_widget *)(void *)keyboard;
    assert(kb_backend->ld_widget != 0);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TABLE_FIELD_ROWS
        | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_KEYBOARD_BINDING
        | TINYUI_TABLE_FIELD_STYLE_CLASS
        | TINYUI_TABLE_FIELD_USER_DATA
        | TINYUI_TABLE_FIELD_WIDTH
        | TINYUI_TABLE_FIELD_HEIGHT
        | TINYUI_TABLE_FIELD_BG_COLOR
        | TINYUI_TABLE_FIELD_TEXT_COLOR
        | TINYUI_TABLE_FIELD_PADDING;
    props.rows = 3;
    props.columns = 4;
    props.keyboard_binding = kb_backend->ld_name_id;
    props.style_class = "table-props";
    props.user_data = &cookie;
    props.width = 160;
    props.height = 90;
    props.bg_color = 0x102030U;
    props.text_color = 0x405060U;
    props.padding = 3;

    table = tinyui_table_create_with_props(root, &props);
    assert(table != 0);
    backend = (struct tinyui_widget *)(void *)table;
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    assert(ld_table->rowCount == 3);
    assert(ld_table->columnCount == 4);
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == kb_backend->ld_name_id);
    assert(ld_table->kbNameId == kb_backend->ld_name_id);
    assert(backend->width == 160);
    assert(backend->height == 90);
    assert(backend->user_data == &cookie);
    assert(backend->bg_color == 0x102030U);
    assert(backend->text_color == 0x405060U);
    assert(backend->padding == 3);
    assert(ld_table->itemSpace == 3);
    assert(backend->style_class == props.style_class);
    {
        ldTableItem_t *item = ldTableGetItem(ld_table, 0, 0);
        assert(item != 0);
        assert(item->textColor == (ldColor)tinyui_rgb_to_ld_color(0x405060U));
    }
}

static void test_table_create_with_props_unsupported_style_rolls_back(tinyui_obj_t *root)
{
    uint16_t child_count_before = 0;
    uint16_t child_count_after = 0;
    tinyui_table_props_t props;
    tinyui_obj_t *table;

    assert(tinyui_obj_get_child_count(root, &child_count_before) == TINYUI_OK);
    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TABLE_FIELD_ROWS | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_BORDER_COLOR | TINYUI_TABLE_FIELD_RADIUS;
    props.rows = 2;
    props.columns = 2;
    props.border_color = 0x708090U;
    props.radius = 5;
    table = tinyui_table_create_with_props(root, &props);
    assert(table == 0);
    assert(tinyui_obj_get_child_count(root, &child_count_after) == TINYUI_OK);
    assert(child_count_after == child_count_before);
}

static void test_table_create_with_props_keyboard_failure_rolls_back(tinyui_obj_t *root)
{
    ldBase_t *root_ld = (ldBase_t *)((struct tinyui_widget *)(void *)root)->ld_widget;
    ldBase_t *child_before = ldBaseGetChildList(root_ld);
    tinyui_table_props_t props;
    tinyui_obj_t *table;
    const struct tinyui_widget *disposed;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_TABLE_FIELD_ROWS
        | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_KEYBOARD_BINDING;
    props.rows = 2;
    props.columns = 2;
    props.keyboard_binding = 7U;

    tinyui_table_test_reset_state();
    tinyui_test_capture_destroyed_widget_snapshot(0);
    tinyui_table_test_fail_next_set_keyboard_binding();

    table = tinyui_table_create_with_props(root, &props);
    assert(table == 0);
    disposed = tinyui_table_test_last_disposed_backend();
    assert(disposed != 0);
    assert(disposed->kind == TINYUI_BACKEND_WIDGET_TABLE);
    assert(disposed->owner == 0);
    assert(disposed->ld_widget == 0);
    assert(ldBaseGetChildList(root_ld) == child_before);
}

static void test_table_cell_text_aliases_and_temp_buffer(tinyui_obj_t *root)
{
    tinyui_obj_t *table = create_table_3x3(root);
    ldTable_t *ld_table = table_ld(table);
    char temp[16];
    const char *static_literal = "STATIC";
    const char *got;

    assert(tinyui_table_set_item_text(table, 0, 0, "A") == 0);
    assert(strcmp(tinyui_table_get_item_text(table, 0, 0), "A") == 0);
    assert(strcmp(tinyui_table_get_cell_text(table, 0, 0), "A") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "A") == 0);

    assert(tinyui_table_set_cell_text(table, 1, 2, "CELL") == 0);
    assert(strcmp(tinyui_table_get_item_text(table, 1, 2), "CELL") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 1, 2), "CELL") == 0);

    /* set_item_text must copy: overwriting the caller buffer must not change cell. */
    memcpy(temp, "TEMP", 5);
    assert(tinyui_table_set_item_text(table, 2, 0, temp) == 0);
    memcpy(temp, "XXXX", 5);
    got = tinyui_table_get_item_text(table, 2, 0);
    assert(got != 0);
    assert(strcmp(got, "TEMP") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 2, 0), "TEMP") == 0);

    /* set_item_static_text borrows the pointer (LD contract). */
    assert(tinyui_table_set_item_static_text(table, 2, 1, static_literal) == 0);
    assert(tinyui_table_get_item_text(table, 2, 1) == static_literal);
    assert(ldTableGetItem(ld_table, 2, 1)->isStaticText == true);
    assert(ldTableGetItem(ld_table, 2, 1)->pText == (uint8_t *)static_literal);
}

static void test_table_row_column_bounds_reject(tinyui_obj_t *root)
{
    tinyui_obj_t *table = create_table_3x3(root);
    ldTable_t *ld_table = table_ld(table);
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    tinyui_image_source_t source;

    bind_test_tiles(&source, &img_tile, &mask_tile);

    assert(tinyui_table_set_item_text(table, -1, 0, "x") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_table_set_item_text(table, 0, -1, "x") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_table_set_item_text(table, 3, 0, "x") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_table_set_item_text(table, 0, 3, "x") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_table_set_cell_text(table, 9, 9, "x") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_table_get_item_text(table, 3, 0) == 0);
    assert(tinyui_table_get_cell_text(table, 0, 3) == 0);

    assert(tinyui_table_set_item_editable(table, 3, 0, 1, 8) == -1);
    assert(tinyui_table_set_cell_editable(table, 0, 3, 1, 8) == -1);
    assert(tinyui_table_set_item_color(table, 3, 0, 0x112233U, 0x445566U) == -1);
    assert(tinyui_table_set_item_align(table, 0, 3, TINYUI_ALIGN_CENTER) == -1);
    assert(tinyui_table_set_item_width(table, 3, 40) == -1);
    assert(tinyui_table_set_item_height(table, 3, 20) == -1);
    assert(tinyui_table_set_item_font(table, 3, 0) == -1);
    assert(tinyui_table_set_item_static_text(table, 3, 0, "s") == -1);
    assert(tinyui_table_set_item_image(table, 3, 0, 0, 0, &source, 0xFFFFFFU) == -1);
    assert(tinyui_table_set_item_button(table, 0, 3, 0, 0, &source, 0xFFFFFFU, &source, 0xFFFFFFU, 0) == -1);
    assert(tinyui_table_set_item_select(table, 3, 0, 1) == -1);
    assert(tinyui_table_set_current_cell(table, 3, 0) == -1);
    assert(tinyui_table_set_selected_cell(table, 0, 3) == -1);
    assert(tinyui_table_get_item(table, 3, 0) == 0);
    assert(tinyui_table_get_item_font(table, 3, 0) == 0);
    assert(tinyui_table_get_item_height(table, 3) == -1);
    assert(tinyui_table_get_item_width(table, 3) == -1);
    assert(tinyui_table_get_item_align(table, 3, 0) == -1);
    assert(tinyui_table_get_item_editable(table, 3, 0) == -1);

    /* OOB must not silently mutate any in-range cell. */
    assert(tinyui_table_set_item_text(table, 0, 0, "keep") == 0);
    assert(tinyui_table_set_item_text(table, 3, 0, "leak") == -1);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "keep") == 0);
}

static void test_table_r4_aliases_and_native_getters_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *table = create_table_3x3(root);
    struct tinyui_widget *kb_backend;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    unsigned int keyboard_binding = 0U;
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    arm_2d_tile_t press_tile = {0};
    arm_2d_tile_t press_mask_tile = {0};
    tinyui_image_source_t image_source;
    tinyui_image_source_t release_source;
    tinyui_image_source_t press_source;
    struct tinyui_table_region region;

    assert(keyboard != 0);
    assert(table != 0);
    kb_backend = (struct tinyui_widget *)(void *)keyboard;
    backend = (struct tinyui_widget *)(void *)table;
    ld_table = table_ld(table);
    assert(kb_backend->ld_widget != 0);

    /* create dims */
    assert(ld_table->rowCount == 3);
    assert(ld_table->columnCount == 3);

    /* keyboard binding aliases */
    assert(tinyui_table_set_keyboard(table, kb_backend->ld_name_id) == 0);
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == kb_backend->ld_name_id);
    assert(ld_table->kbNameId == kb_backend->ld_name_id);
    assert(tinyui_table_set_keyboard_widget(table, keyboard) == 0);
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == kb_backend->ld_name_id);

    /* item text / cell aliases */
    assert(tinyui_table_set_item_text(table, 1, 1, "CELL") == 0);
    assert(strcmp(tinyui_table_get_item_text(table, 1, 1), "CELL") == 0);
    assert(strcmp(tinyui_table_get_cell_text(table, 1, 1), "CELL") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 1, 1), "CELL") == 0);

    /* editable */
    assert(tinyui_table_set_item_editable(table, 1, 1, 1, 12) == 0);
    assert(tinyui_table_get_item_editable(table, 1, 1) == 1);
    assert(ldTableGetItemEditable(ld_table, 1, 1) == true);
    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 8) == 0);
    assert(tinyui_table_get_item_editable(table, 0, 0) == 1);

    /* background / excel / align_grid */
    assert(tinyui_table_set_background_color(table, 0x102030U) == 0);
    assert(tinyui_table_get_background_color(table)
           == (unsigned int)ldTableGetBackgroundColor(ld_table));
    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0x102030U);
    assert(tinyui_table_set_bg_color(table, 0x203040U) == 0);
    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0x203040U);
    assert(tinyui_table_set_excel_type(table) == 0);
    assert(tinyui_table_set_align_grid(table, 1) == 0);
    assert(tinyui_table_get_align_grid(table) == 1);
    assert(ldTableGetAlignGrid(ld_table) == true);
    assert(tinyui_table_set_align_grid(table, 0) == 0);
    assert(tinyui_table_get_align_grid(table) == 0);

    /* width / height / color / font / align / select */
    assert(tinyui_table_set_item_width(table, 2, 66) == 0);
    assert(tinyui_table_get_item_width(table, 2) == ldTableGetItemWidth(ld_table, 2));
    assert(ldTableGetItemWidth(ld_table, 2) == 66);
    assert(tinyui_table_set_item_height(table, 1, 28) == 0);
    assert(tinyui_table_get_item_height(table, 1) == ldTableGetItemHeight(ld_table, 1));
    assert(ldTableGetItemHeight(ld_table, 1) == 28);
    assert(tinyui_table_set_item_color(table, 1, 1, 0xABCDEFU, 0x123456U) == 0);
    assert(tinyui_table_get_item_text_color(table, 1, 1)
           == (unsigned int)ldTableGetItemTextColor(ld_table, 1, 1));
    assert(tinyui_table_get_item_background_color(table, 1, 1)
           == (unsigned int)ldTableGetItemBackgroundColor(ld_table, 1, 1));
    assert(ldTableGetItemTextColor(ld_table, 1, 1) == (ldColor)0xABCDEFU);
    assert(ldTableGetItemBackgroundColor(ld_table, 1, 1) == (ldColor)0x123456U);
    assert(tinyui_table_set_item_font(table, 1, 1) == 0);
    assert(tinyui_table_get_item_font(table, 1, 1) == ldTableGetItemFont(ld_table, 1, 1));
    assert(tinyui_table_get_item_font(table, 1, 1) != 0);
    assert(tinyui_table_set_item_align(table, 1, 1, TINYUI_ALIGN_CENTER) == 0);
    assert(tinyui_table_get_item_align(table, 1, 1) == TINYUI_ALIGN_CENTER);
    assert(ldTableGetItemAlign(ld_table, 1, 1) == ARM_2D_ALIGN_CENTRE);
    assert(tinyui_table_set_item_select(table, 1, 1, 1) == 0);
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 1);
    assert(ld_table->currentRow == 1);
    assert(ld_table->currentColumn == 1);

    item = (ldTableItem_t *)tinyui_table_get_item(table, 1, 1);
    assert(item != 0);
    assert(item == ldTableGetItem(ld_table, 1, 1));

    region = tinyui_table_get_item_region(table, 1, 1);
    assert(region.width == ldTableGetItemWidth(ld_table, 1));
    assert(region.height == ldTableGetItemHeight(ld_table, 1));

    /* image / button */
    bind_test_tiles(&image_source, &img_tile, &mask_tile);
    bind_test_tiles(&release_source, &img_tile, &mask_tile);
    bind_test_tiles(&press_source, &press_tile, &press_mask_tile);
    assert(tinyui_table_set_item_image(table, 0, 1, 3, 4, &image_source, 0xFFFFFFU) == 0);
    item = ldTableGetItem(ld_table, 0, 1);
    assert(item != 0);
    assert(item->ptPressImgTile == tinyui_image_source_get_image_tile(&image_source));
    assert(item->isButton == false);
    assert(tinyui_table_set_item_button(table,
                                        0,
                                        2,
                                        1,
                                        2,
                                        &release_source,
                                        0xFFFFFFU,
                                        &press_source,
                                        0x0F0F0FU,
                                        1) == 0);
    item = ldTableGetItem(ld_table, 0, 2);
    assert(item != 0);
    assert(item->isButton == true);
    assert(item->isCheckable == true);
    assert(item->ptReleaseImgTile == tinyui_image_source_get_image_tile(&release_source));
    assert(item->ptPressImgTile == tinyui_image_source_get_image_tile(&press_source));

    /* navigate */
    assert(tinyui_table_set_current_cell(table, 1, 1) == 0);
    assert(tinyui_table_navigate(table, TINYUI_NAV_RIGHT) == 0);
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 2);
    assert(ld_table->currentRow == 1);
    assert(ld_table->currentColumn == 2);
    assert(tinyui_table_navigate(table, TINYUI_NAV_DOWN) == 0);
    assert(tinyui_table_get_current_row(table) == 2);
    assert(tinyui_table_get_current_column(table) == 2);

    /* show_keyboard on editable current cell */
    assert(tinyui_table_set_current_cell(table, 1, 1) == 0);
    assert(tinyui_table_set_item_editable(table, 1, 1, 1, 12) == 0);
    assert(tinyui_table_show_keyboard(table) == 0);
    assert(tinyui_runtime_internal_widget_is_hidden(kb_backend) == 0);

    /* item space */
    assert(tinyui_table_set_item_space(table, 6U) == 0);
    assert(ld_table->itemSpace == 6);

    /* negative cases retained in r4 */
    assert(tinyui_table_set_item_select(table, 1, 1, 0) == -1);
    assert(tinyui_table_get_item(table, 9, 9) == 0);
    assert(tinyui_table_get_item_font(table, 9, 9) == 0);
    assert(tinyui_table_get_item_height(table, 9) == -1);
    assert(tinyui_table_get_item_width(table, 9) == -1);
    (void)backend;
}

static void test_table_edit_commit_path(tinyui_obj_t *root)
{
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    tinyui_obj_t *table = create_table_3x3(root);
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldTable_t *ld_table;

    assert(keyboard != 0);
    assert(table != 0);
    assert(tinyui_table_set_keyboard_widget(table, keyboard) == 0);
    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(tinyui_table_set_cell_text(table, 0, 0, "before") == 0);
    assert(tinyui_table_set_current_cell(table, 0, 0) == 0);

    backend = (struct tinyui_widget *)(void *)table;
    ensure_msg_queue(backend);
    app_state = backend->owner;
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_runtime_internal_widget_is_focus_owner(backend) == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_runtime_internal_widget_is_editing_owner(backend) == 1);

    ldTableSetItemText(ld_table, 0, 0, (uint8_t *)"after");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_FINISHED,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);

    assert(strcmp(tinyui_table_get_cell_text(table, 0, 0), "after") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "after") == 0);
    assert(backend->last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    assert(tinyui_runtime_internal_widget_is_editing_owner(backend) == 0);
}

static void test_table_rejects_null_and_invalid(tinyui_obj_t *root)
{
    tinyui_obj_t *table = create_table_3x3(root);
    tinyui_obj_t *keyboard = tinyui_keyboard_create(root);
    unsigned int binding = 1U;
    tinyui_image_source_t empty_source;

    assert(table != 0);
    assert(keyboard != 0);
    memset(&empty_source, 0, sizeof(empty_source));

    assert(tinyui_table_set_item_text(0, 0, 0, "x") == -1);
    assert(tinyui_table_set_item_text(table, 0, 0, 0) == -1);
    assert(tinyui_table_get_item_text(0, 0, 0) == 0);
    assert(tinyui_table_set_keyboard_binding(0, 1U) == -1);
    assert(tinyui_table_set_keyboard_binding(table, 0U) == -1);
    assert(tinyui_table_get_keyboard_binding(0, &binding) == -1);
    assert(tinyui_table_get_keyboard_binding(table, 0) == -1);
    assert(tinyui_table_set_keyboard_widget(table, 0) == -1);
    assert(tinyui_table_set_keyboard_widget(0, keyboard) == -1);
    assert(tinyui_table_set_item_image(table, 0, 0, 0, 0, 0, 0xFFFFFFU) == -1);
    assert(tinyui_table_set_item_image(table, 0, 0, 0, 0, &empty_source, 0xFFFFFFU) == -1);
    assert(tinyui_table_set_item_width(table, 0, 0) == -1);
    assert(tinyui_table_set_item_height(table, 0, -1) == -1);
    assert(tinyui_table_set_item_color(table, 0, 0, 0x1000000U, 0U) == -1);
    assert(tinyui_table_set_bg_color(table, 0x1000000U) == -1);
    assert(tinyui_table_set_item_space(table, 300U) == -1);
    assert(tinyui_table_navigate(table, TINYUI_NAV_ENTER) == -1);
    assert(tinyui_table_show_keyboard(0) == -1);
    assert(tinyui_table_set_excel_type(0) == -1);
    assert(tinyui_table_set_align_grid(0, 1) == -1);

    {
        tinyui_table_props_t bad_dims;

        memset(&bad_dims, 0, sizeof(bad_dims));
        bad_dims.fields = TINYUI_TABLE_FIELD_ROWS | TINYUI_TABLE_FIELD_COLUMNS;
        bad_dims.rows = 0;
        bad_dims.columns = 3;
        assert(tinyui_table_create_with_props(root, &bad_dims) == 0);
        bad_dims.rows = 3;
        bad_dims.columns = 0;
        assert(tinyui_table_create_with_props(root, &bad_dims) == 0);
        bad_dims.rows = 256;
        bad_dims.columns = 3;
        assert(tinyui_table_create_with_props(root, &bad_dims) == 0);
    }
}

static void test_table_no_backend_prefix_symbols(void)
{
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_cell_text");
    assert_self_binary_lacks_symbol("tinyui_backend_table_get_cell_text");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_keyboard_binding");
    assert_self_binary_lacks_symbol("tinyui_backend_table_navigate");
    assert_self_binary_lacks_symbol("tinyui_backend_table_sync_current_cell");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_image");
    assert_self_binary_lacks_symbol("tinyui_backend_table_bind_host");
}

int main(int argc, char **argv)
{
    tinyui_obj_t *root;

    g_self_binary_path = (argc > 0) ? argv[0] : 0;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_table_create_and_ld_mapping(root);
    test_table_create_with_props_pushes_fields(root);
    test_table_create_with_props_unsupported_style_rolls_back(root);
    test_table_create_with_props_keyboard_failure_rolls_back(root);
    test_table_cell_text_aliases_and_temp_buffer(root);
    test_table_row_column_bounds_reject(root);
    test_table_r4_aliases_and_native_getters_round_trip(root);
    test_table_edit_commit_path(root);
    test_table_rejects_null_and_invalid(root);
    test_table_no_backend_prefix_symbols();

    tinyui_deinit();
    return 0;
}
