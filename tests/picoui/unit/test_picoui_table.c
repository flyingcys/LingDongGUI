#include "picoui/app.h"
#include "picoui/keyboard.h"
#include "picoui/table.h"
#include "picoui/window.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/ldMsg.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>
#include <string.h>

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void ensure_table_msg_queue(struct picoui_backend_app_state *app_state)
{
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
}

static void test_table_current_cell_matches_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    table = picoui_table_create(win, "table_truth", 3, 3);
    assert(table != 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(picoui_table_set_current_cell(table, 1, 2) == 0);
    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 2);

    ldTableSetItemSelect(ld_table, 2, 1, true);
    assert(picoui_table_get_current_row(table) == 2);
    assert(picoui_table_get_current_column(table) == 1);
    picoui_app_destroy(app);
}

static void test_table_edit_commit_updates_model_and_visible_text(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    keyboard = picoui_keyboard_create(win, "keyboard_commit");
    table = picoui_table_create(win, "table_commit", 3, 3);
    assert(keyboard != 0);
    assert(table != 0);
    assert(picoui_table_set_keyboard_binding(table, 2U) == 0);
    assert(picoui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(picoui_table_set_cell_text(table, 0, 0, "before") == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 1);

    ldTableSetItemText(ld_table, 0, 0, (uint8_t *)"after");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(strcmp(picoui_table_get_cell_text(table, 0, 0), "after") == 0);
    assert(table->widget.last_edit_result == PICOUI_EDIT_RESULT_COMMIT);
    assert(table->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(backend->last_native_signal == SIGNAL_FINISHED);
    assert(backend->last_native_value == 0);
    assert(picoui_widget_is_editing_owner(&table->widget) == 0);
    picoui_app_destroy(app);
}

static void test_table_reuses_editable_cell_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    table = picoui_table_create(win, "table_contract", 3, 3);
    assert(table != 0);
    assert(picoui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_focus_owner(&table->widget) == 1);
    assert(picoui_widget_is_editing_owner(&table->widget) == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 0);
    assert(table->widget.last_edit_result == PICOUI_EDIT_RESULT_COMMIT);
    assert(backend->last_native_signal == SIGNAL_FINISHED);
    assert(backend->last_native_value == 0);
    picoui_app_destroy(app);
}

static void test_table_final_release_contract_covers_non_commit_exit_boundary(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldTable_t *ld_table;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "table_release_root");
    assert(win != 0);
    keyboard = picoui_keyboard_create(win, "table_release_keyboard");
    table = picoui_table_create(win, "table_release_ready", 2, 2);
    assert(keyboard != 0);
    assert(table != 0);
    assert(picoui_table_set_keyboard_binding(table, 9U) == 0);
    assert(picoui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(picoui_table_set_cell_text(table, 0, 0, "before") == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_TABLE);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_editing_owner(&table->widget) == 1);
    assert(picoui_widget_claim_focus(&keyboard->widget) == 0);
    assert(picoui_keyboard_exit(keyboard) == 0);

    assert(picoui_widget_is_editing_owner(&table->widget) == 0);
    assert(table->widget.last_edit_result == PICOUI_EDIT_RESULT_CANCEL);
    assert(table->widget.pending_edit_result == PICOUI_EDIT_RESULT_NONE);
    assert(backend->last_native_signal == SIGNAL_PRESS);
    assert(backend->last_native_value == make_signal_value_xy(10, 10));
    assert(strcmp(picoui_table_get_cell_text(table, 0, 0), "before") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "before") == 0);

    picoui_app_destroy(app);
}

static void test_table_native_item_image_button_and_excel_type_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *image_item;
    ldTableItem_t *button_item;
    struct picoui_image_source release_source = {
        .img_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
        .mask_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
    };
    struct picoui_image_source press_source = {
        .img_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
        .mask_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "table_native_root");
    assert(win != 0);
    table = picoui_table_create(win, "table_native_round_trip", 3, 3);
    assert(table != 0);

    assert(picoui_table_set_item_image(table, 0, 1, 6, 8, &release_source, 0x123456U) == 0);
    assert(picoui_table_set_item_button(table,
                                        1,
                                        2,
                                        4,
                                        5,
                                        &release_source,
                                        0x223344U,
                                        &press_source,
                                        0x556677U,
                                        1) == 0);
    assert(picoui_table_set_excel_type(table) == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    image_item = ldTableGetItem(ld_table, 0, 1);
    button_item = ldTableGetItem(ld_table, 1, 2);
    assert(image_item != 0);
    assert(button_item != 0);

    assert(image_item->tLocation.iX == 6);
    assert(image_item->tLocation.iY == 8);
    assert(image_item->ptPressImgTile == release_source.img_tile);
    assert(image_item->ptPressMaskTile == release_source.mask_tile);
    assert(image_item->releaseImgMaskColor == (ldColor)0x123456U);
    assert(image_item->ptReleaseImgTile == 0);
    assert(image_item->ptReleaseMaskTile == 0);

    assert(button_item->isButton == true);
    assert(button_item->isCheckable == true);
    assert(button_item->tLocation.iX == 4);
    assert(button_item->tLocation.iY == 5);
    assert(button_item->ptReleaseImgTile == release_source.img_tile);
    assert(button_item->ptReleaseMaskTile == release_source.mask_tile);
    assert(button_item->ptPressImgTile == press_source.img_tile);
    assert(button_item->ptPressMaskTile == press_source.mask_tile);
    assert(button_item->releaseImgMaskColor == (ldColor)0x223344U);
    assert(button_item->pressImgMaskColor == (ldColor)0x556677U);
    assert(ld_table->bgColor == __RGB(219, 219, 219));
    assert(ld_table->isAlignGrid == true);
    assert(ldTableGetItemWidth(ld_table, 0) == 35);
    assert(ldTableGetItemHeight(ld_table, 0) == 22);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 1, 0), "1") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 1), "A") == 0);
    assert(ldTableGetItemFont(ld_table, 1, 0) == (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    assert(ldTableGetItemEditable(ld_table, 1, 1) == true);

    assert(picoui_table_set_item_image(table, 0, 1, 0, 0, 0, 0xABCDEFU) == -1);
    assert(picoui_table_set_item_button(table, 1, 2, 0, 0, 0, 0, &press_source, 0x556677U, 0) == -1);

    assert(image_item->ptPressImgTile == release_source.img_tile);
    assert(button_item->ptReleaseImgTile == release_source.img_tile);
    assert(button_item->ptPressImgTile == press_source.img_tile);

    picoui_app_destroy(app);
}

static void test_table_native_size_align_color_font_region_and_navigation_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;
    struct picoui_table_region region;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "table_native_style_root");
    assert(win != 0);
    table = picoui_table_create(win, "table_native_style_round_trip", 3, 3);
    assert(table != 0);

    assert(picoui_table_set_item_width(table, 1, 88) == 0);
    assert(picoui_table_set_item_height(table, 2, 26) == 0);
    assert(picoui_table_set_item_color(table, 1, 1, 0x112233U, 0x445566U) == 0);
    assert(picoui_table_set_item_font(table, 1, 1) == 0);
    assert(picoui_table_set_item_align(table, 1, 1, PICOUI_ALIGN_CENTER) == 0);
    assert(picoui_table_set_current_cell(table, 1, 1) == 0);
    assert(picoui_table_navigate(table, PICOUI_NATIVE_NAV_RIGHT) == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldTableGetItemWidth(ld_table, 1) == 88);
    assert(ldTableGetItemHeight(ld_table, 2) == 26);
    assert(ldTableGetItemTextColor(ld_table, 1, 1) == (ldColor)0x112233U);
    assert(ldTableGetItemBackgroundColor(ld_table, 1, 1) == (ldColor)0x445566U);
    assert(ldTableGetItemFont(ld_table, 1, 1) == (arm_2d_font_t *)&ARM_2D_FONT_6x8);
    assert(ldTableGetItemAlign(ld_table, 1, 1) == ARM_2D_ALIGN_CENTRE);
    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 2);

    region = picoui_table_get_item_region(table, 1, 1);
    assert(region.width == 88);
    assert(region.height == ldTableGetItemHeight(ld_table, 1));
    assert(region.x == 76);
    assert(region.y == 42);

    assert(picoui_table_set_item_width(table, 1, 0) == -1);
    assert(picoui_table_set_item_height(table, 3, 20) == -1);
    assert(picoui_table_set_item_color(table, 1, 1, 0x1000000U, 0x445566U) == -1);
    assert(picoui_table_navigate(table, (enum picoui_native_nav_dir)99) == -1);

    assert(ldTableGetItemWidth(ld_table, 1) == 88);
    assert(ldTableGetItemHeight(ld_table, 2) == 26);
    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 2);

    picoui_app_destroy(app);
}

static void test_table_native_static_text_background_and_getters_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_backend_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *item;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "table_native_getters_root");
    assert(win != 0);
    table = picoui_table_create(win, "table_native_getters_round_trip", 3, 3);
    assert(table != 0);

    assert(picoui_table_set_bg_color(table, 0xA0B0C0U) == 0);
    assert(picoui_table_set_item_static_text(table, 0, 2, "HEAD") == 0);
    assert(picoui_table_set_item_align(table, 0, 2, PICOUI_ALIGN_END) == 0);
    assert(picoui_table_set_cell_editable(table, 2, 2, 1, 12) == 0);
    assert(picoui_table_set_selected_cell(table, 2, 2) == 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    item = ldTableGetItem(ld_table, 0, 2);
    assert(item != 0);

    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0xA0B0C0U);
    assert(item->isStaticText == true);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 2), "HEAD") == 0);
    assert(picoui_table_get_item_align(table, 0, 2) == PICOUI_ALIGN_END);
    assert(picoui_table_get_item_editable(table, 2, 2) == 1);
    assert(picoui_table_get_current_row(table) == 2);
    assert(picoui_table_get_current_column(table) == 2);

    assert(picoui_table_set_item_static_text(table, 0, 2, 0) == -1);
    assert(picoui_table_set_bg_color(table, 0x1000000U) == -1);
    assert(picoui_table_get_item_align(table, 9, 9) == -1);
    assert(picoui_table_get_item_editable(table, 9, 9) == -1);

    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0xA0B0C0U);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 2), "HEAD") == 0);

    picoui_app_destroy(app);
}

static void test_table_r4_aliases_and_native_getters_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_table *table;
    struct picoui_keyboard *keyboard;
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *keyboard_backend;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    unsigned int keyboard_binding = 0;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "table_r4_alias_root");
    assert(win != 0);
    keyboard = picoui_keyboard_create(win, "table_r4_alias_keyboard");
    assert(keyboard != 0);
    table = picoui_table_init(win, "table_r4_alias", 3, 3);
    assert(table != 0);

    backend = (struct picoui_backend_widget *)table->widget.backend_widget;
    assert(backend != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    keyboard_backend = (struct picoui_backend_widget *)keyboard->widget.backend_widget;
    assert(keyboard_backend != 0);

    assert(picoui_table_set_keyboard(table, keyboard_backend->ld_name_id) == 0);
    assert(picoui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == keyboard_backend->ld_name_id);
    assert(ld_table->kbNameId == keyboard_backend->ld_name_id);

    assert(picoui_table_set_item_text(table, 1, 1, "CELL") == 0);
    assert(strcmp(picoui_table_get_item_text(table, 1, 1), "CELL") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 1, 1), "CELL") == 0);

    assert(picoui_table_set_item_editable(table, 1, 1, 1, 9) == 0);
    assert(picoui_table_get_item_editable(table, 1, 1) == 1);
    assert(ldTableGetItemEditable(ld_table, 1, 1) == true);

    assert(picoui_table_set_background_color(table, 0x102030U) == 0);
    assert(picoui_table_get_background_color(table) == 0x102030U);
    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0x102030U);

    assert(picoui_table_set_align_grid(table, 1) == 0);
    assert(picoui_table_get_align_grid(table) == 1);
    assert(ldTableGetAlignGrid(ld_table) == true);

    assert(picoui_table_set_item_width(table, 2, 66) == 0);
    assert(picoui_table_set_item_height(table, 1, 28) == 0);
    assert(picoui_table_set_item_color(table, 1, 1, 0xABCDEFU, 0x123456U) == 0);
    assert(picoui_table_set_item_font(table, 1, 1) == 0);
    assert(picoui_table_set_item_align(table, 1, 1, PICOUI_ALIGN_CENTER) == 0);
    assert(picoui_table_set_item_select(table, 1, 1, 1) == 0);

    item = (ldTableItem_t *)picoui_table_get_item(table, 1, 1);
    assert(item != 0);
    assert(item == ldTableGetItem(ld_table, 1, 1));
    assert(picoui_table_get_item_font(table, 1, 1) == ldTableGetItemFont(ld_table, 1, 1));
    assert(picoui_table_get_item_height(table, 1) == ldTableGetItemHeight(ld_table, 1));
    assert(picoui_table_get_item_width(table, 2) == ldTableGetItemWidth(ld_table, 2));
    assert(picoui_table_get_item_text_color(table, 1, 1) == (unsigned int)ldTableGetItemTextColor(ld_table, 1, 1));
    assert(picoui_table_get_item_background_color(table, 1, 1) ==
           (unsigned int)ldTableGetItemBackgroundColor(ld_table, 1, 1));
    assert(picoui_table_get_item_align(table, 1, 1) == PICOUI_ALIGN_CENTER);
    assert(picoui_table_get_current_row(table) == 1);
    assert(picoui_table_get_current_column(table) == 1);

    assert(picoui_tabel_show_keyboard(table) == 0);
    assert(picoui_widget_is_hidden(&keyboard->widget) == 0);

    assert(picoui_table_set_item_select(table, 1, 1, 0) == -1);
    assert(picoui_table_get_item(table, 9, 9) == 0);
    assert(picoui_table_get_item_font(table, 9, 9) == 0);
    assert(picoui_table_get_item_height(table, 9) == -1);
    assert(picoui_table_get_item_width(table, 9) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_table_current_cell_matches_backend_truth();
    test_table_edit_commit_updates_model_and_visible_text();
    test_table_reuses_editable_cell_contract();
    test_table_final_release_contract_covers_non_commit_exit_boundary();
    test_table_native_item_image_button_and_excel_type_round_trip();
    test_table_native_size_align_color_font_region_and_navigation_round_trip();
    test_table_native_static_text_background_and_getters_round_trip();
    test_table_r4_aliases_and_native_getters_round_trip();
    return 0;
}
