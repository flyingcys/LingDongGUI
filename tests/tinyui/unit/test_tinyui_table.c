#include "core/app.h"
#include "widgets/keyboard.h"
#include "widgets/table.h"
#include "widgets/window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/ldMsg.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"
#include "internal.h"
#include "tinyui_test_support.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

void tinyui_table_test_fail_next_set_keyboard_binding(void);
void tinyui_table_test_reset_state(void);
static struct tinyui_widget g_disposed_backend_snapshot;
static int g_disposed_backend_valid = 0;

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

static const char *test_self_binary_path = 0;
static char test_table_source_path[PATH_MAX];

static void init_table_source_path(void)
{
    const char *source = __FILE__;
    const char *suffix = "tests/tinyui/unit/test_tinyui_table.c";
    const char *match = strstr(source, suffix);
    size_t root_len;

    assert(match != 0);
    root_len = (size_t)(match - source);
    assert(root_len + strlen("tinyui/src/widgets/table.c") < sizeof(test_table_source_path));
    memcpy(test_table_source_path, source, root_len);
    snprintf(test_table_source_path + root_len,
             sizeof(test_table_source_path) - root_len,
             "tinyui/src/widgets/table.c");
}

static int test_source_has_symbol(const char *source_path, const char *symbol)
{
    char command[1024];

    assert(source_path != 0);
    assert(symbol != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(0 if sys.argv[2] in text else 1)\n"
             "PY",
             source_path,
             symbol);
    return system(command) == 0;
}

static void assert_archive_lacks_symbol(const char *archive_relpath, const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    int status;

    assert(test_self_binary_path != 0);
    assert(archive_relpath != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command),
             "cd \"$(dirname '%s')\" && nm \"%s\" 2>/dev/null",
             test_self_binary_path,
             archive_relpath);
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
            assert(!"unexpected symbol still present in archive");
        }
    }
    status = pclose(pipe);
    assert(status != -1);
}

static void assert_archive_lacks_member(const char *archive_relpath, const char *member)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    size_t member_len;
    int status;

    assert(test_self_binary_path != 0);
    assert(archive_relpath != 0);
    assert(member != 0);
    snprintf(command, sizeof(command),
             "cd \"$(dirname '%s')\" && nm \"%s\" 2>/dev/null",
             test_self_binary_path,
             archive_relpath);
    pipe = popen(command, "r");
    assert(pipe != 0);
    member_len = strlen(member);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len == member_len + 1 &&
            strncmp(line, member, member_len) == 0 &&
            line[member_len] == ':') {
            assert(!"unexpected archive member still present");
        }
    }
    status = pclose(pipe);
    assert(status != -1);
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    int status;

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
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
            assert(!"unexpected symbol still present in test binary");
        }
    }
    status = pclose(pipe);
    assert(status != -1);
}

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static arm_2d_tile_t make_table_rgb565_tile(uint16_t *buffer, int16_t width, int16_t height)
{
    arm_2d_tile_t tile = {0};

    tile.bIsRoot = true;
    tile.tInfo.tColourInfo.chScheme = ARM_2D_COLOUR_RGB565;
    tile.tRegion.tSize.iWidth = width;
    tile.tRegion.tSize.iHeight = height;
    tile.phwBuffer = buffer;
    return tile;
}

static void ensure_table_msg_queue(struct tinyui_app *app_state)
{
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
}

static void test_table_current_cell_matches_backend_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_truth", 3, 3);
    assert(table != 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(tinyui_table_set_current_cell(table, 1, 2) == 0);
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 2);

    ldTableSetItemSelect(ld_table, 2, 1, true);
    assert(tinyui_table_get_current_row(table) == 2);
    assert(tinyui_table_get_current_column(table) == 1);
    tinyui_app_destroy(app);
}

static void test_table_create_builds_direct_backend_mapping(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldTable_t *ld_table;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_direct_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_direct_mapping", 3, 3);
    assert(table != 0);

    backend = &table->widget;
    parent_backend = &win->widget;
    assert(backend->ld_widget != 0);
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_TABLE);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    assert(tinyui_app_lookup_host(backend->owner, backend->ld_name_id) == backend);
    assert(ld_table->itemSpace == 4);
    tinyui_app_destroy(app);
}

static void test_table_edit_commit_updates_model_and_visible_text(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_keyboard *keyboard;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldTable_t *ld_table;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "keyboard_commit");
    table = tinyui_table_create(win, "table_commit", 3, 3);
    assert(keyboard != 0);
    assert(table != 0);
    assert(tinyui_table_set_keyboard_binding(table, 2U) == 0);
    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(tinyui_table_set_cell_text(table, 0, 0, "before") == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    app_state = backend->owner;
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
    assert(tinyui_widget_is_editing_owner(&table->widget) == 1);

    ldTableSetItemText(ld_table, 0, 0, (uint8_t *)"after");
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(strcmp(tinyui_table_get_cell_text(table, 0, 0), "after") == 0);
    assert(table->widget.last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    assert(table->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(tinyui_widget_is_editing_owner(&table->widget) == 0);
    tinyui_app_destroy(app);
}

static void test_table_reuses_editable_cell_contract(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_contract", 3, 3);
    assert(table != 0);
    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    app_state = backend->owner;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_widget_is_focus_owner(&table->widget) == 1);
    assert(tinyui_widget_is_editing_owner(&table->widget) == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_widget_is_editing_owner(&table->widget) == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_FINISHED, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_widget_is_editing_owner(&table->widget) == 0);
    assert(table->widget.last_edit_result == TINYUI_EDIT_RESULT_COMMIT);
    tinyui_app_destroy(app);
}

static void test_table_double_press_blank_excel_cell_keeps_font_for_editing(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    arm_2d_tile_t draw_tile;
    static uint16_t draw_buffer[LD_CFG_SCREEN_WIDTH * LD_CFG_SCREEN_HEIGHT];

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_excel_click_root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "table_excel_keyboard");
    table = tinyui_table_create(win, "table_excel_click", 10, 6);
    assert(keyboard != 0);
    assert(table != 0);
    assert(tinyui_widget_set_pos(&table->widget, 780, 150) == 0);
    assert(tinyui_widget_set_size(&table->widget, 200, 100) == 0);
    assert(tinyui_table_set_item_space(table, 1) == 0);
    assert(tinyui_table_set_excel_type(table) == 0);
    assert(tinyui_table_set_keyboard_widget(table, keyboard) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    app_state = backend->owner;
    assert(app_state != 0);
    ensure_table_msg_queue(app_state);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(871, 241)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(871, 240)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(895, 246)) == true);
    ldMsgProcess(app_state->ld_scene);

    item = ldTableGetItem(ld_table, ld_table->currentRow, ld_table->currentColumn);
    assert(item != 0);
    assert(item->isEditable == true);
    assert(item->isEditing == true);
    assert(item->ptFont != 0);
    assert(((ldBase_t *)keyboard->widget.ld_widget)->isHidden == false);

    memset(draw_buffer, 0, sizeof(draw_buffer));
    draw_tile = make_table_rgb565_tile(draw_buffer, LD_CFG_SCREEN_WIDTH, LD_CFG_SCREEN_HEIGHT);
    ldTable_show(app_state->ld_scene, ld_table, &draw_tile, true);

    tinyui_app_destroy(app);
}

static void test_table_final_release_contract_covers_non_commit_exit_boundary(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_keyboard *keyboard;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldTable_t *ld_table;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_release_root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "table_release_keyboard");
    table = tinyui_table_create(win, "table_release_ready", 2, 2);
    assert(keyboard != 0);
    assert(table != 0);
    assert(tinyui_table_set_keyboard_binding(table, 9U) == 0);
    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 16) == 0);
    assert(tinyui_table_set_cell_text(table, 0, 0, "before") == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_TABLE);
    app_state = backend->owner;
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
    assert(tinyui_widget_is_editing_owner(&table->widget) == 1);
    assert(tinyui_widget_claim_focus(&keyboard->widget) == 0);
    assert(tinyui_keyboard_exit(keyboard) == 0);

    assert(tinyui_widget_is_editing_owner(&table->widget) == 0);
    assert(table->widget.last_edit_result == TINYUI_EDIT_RESULT_CANCEL);
    assert(table->widget.pending_edit_result == TINYUI_EDIT_RESULT_NONE);
    assert(strcmp(tinyui_table_get_cell_text(table, 0, 0), "before") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "before") == 0);

    tinyui_app_destroy(app);
}

static void test_table_native_item_image_button_and_excel_type_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *image_item;
    ldTableItem_t *button_item;
    struct tinyui_image_source release_source = {
        .img_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
        .mask_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
    };
    struct tinyui_image_source press_source = {
        .img_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
        .mask_tile = (arm_2d_tile_t *)&ARM_2D_FONT_6x8,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_native_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_native_round_trip", 3, 3);
    assert(table != 0);

    assert(tinyui_table_set_item_image(table, 0, 1, 6, 8, &release_source, 0x123456U) == 0);
    assert(tinyui_table_set_item_button(table,
                                        1,
                                        2,
                                        4,
                                        5,
                                        &release_source,
                                        0x223344U,
                                        &press_source,
                                        0x556677U,
                                        1) == 0);
    assert(tinyui_table_set_excel_type(table) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
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
    assert(ldTableGetItemFont(ld_table, 1, 0) == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(ldTableGetItemEditable(ld_table, 1, 1) == true);

    assert(tinyui_table_set_item_image(table, 0, 1, 0, 0, 0, 0xABCDEFU) == -1);
    assert(tinyui_table_set_item_button(table, 1, 2, 0, 0, 0, 0, &press_source, 0x556677U, 0) == -1);

    assert(image_item->ptPressImgTile == release_source.img_tile);
    assert(button_item->ptReleaseImgTile == release_source.img_tile);
    assert(button_item->ptPressImgTile == press_source.img_tile);

    tinyui_app_destroy(app);
}

static void test_table_native_size_align_color_font_region_and_navigation_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    struct tinyui_table_region region;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_native_style_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_native_style_round_trip", 3, 3);
    assert(table != 0);

    assert(tinyui_table_set_item_width(table, 1, 88) == 0);
    assert(tinyui_table_set_item_height(table, 2, 26) == 0);
    assert(tinyui_table_set_item_color(table, 1, 1, 0x112233U, 0x445566U) == 0);
    assert(tinyui_table_set_item_font(table, 1, 1) == 0);
    assert(tinyui_table_set_item_align(table, 1, 1, TINYUI_ALIGN_CENTER) == 0);
    assert(tinyui_table_set_current_cell(table, 1, 1) == 0);
    assert(tinyui_table_navigate(table, TINYUI_NATIVE_NAV_RIGHT) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(ldTableGetItemWidth(ld_table, 1) == 88);
    assert(ldTableGetItemHeight(ld_table, 2) == 26);
    assert(ldTableGetItemTextColor(ld_table, 1, 1) == (ldColor)0x112233U);
    assert(ldTableGetItemBackgroundColor(ld_table, 1, 1) == (ldColor)0x445566U);
    assert(ldTableGetItemFont(ld_table, 1, 1) == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(ldTableGetItemAlign(ld_table, 1, 1) == ARM_2D_ALIGN_CENTRE);
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 2);
    assert(ld_table->itemSpace == 4);

    region = tinyui_table_get_item_region(table, 1, 1);
    assert(region.width == 88);
    assert(region.height == ldTableGetItemHeight(ld_table, 1));
    assert(region.x == 76);
    assert(region.y == 42);

    assert(tinyui_table_set_item_width(table, 1, 0) == -1);
    assert(tinyui_table_set_item_height(table, 3, 20) == -1);
    assert(tinyui_table_set_item_color(table, 1, 1, 0x1000000U, 0x445566U) == -1);
    assert(tinyui_table_navigate(table, (enum tinyui_native_nav_dir)99) == -1);

    assert(ldTableGetItemWidth(ld_table, 1) == 88);
    assert(ldTableGetItemHeight(ld_table, 2) == 26);
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 2);

    tinyui_app_destroy(app);
}

static void test_table_item_space_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    ldTable_t *ld_table;
    struct tinyui_table_region region;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_item_space_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_item_space", 3, 3);
    assert(table != 0);
    ld_table = (ldTable_t *)table->widget.ld_widget;
    assert(ld_table != 0);

    assert(ld_table->itemSpace == 4);
    assert(tinyui_table_set_excel_type(table) == 0);
    assert(tinyui_table_set_item_width(table, 1, 88) == 0);
    assert(tinyui_table_set_item_space(table, 1) == 0);
    assert(ld_table->itemSpace == 1);

    region = tinyui_table_get_item_region(table, 1, 1);
    assert(region.width == 88);
    assert(region.height == ldTableGetItemHeight(ld_table, 1));
    assert(region.x == 37);
    assert(region.y == 24);

    assert(tinyui_table_set_item_space(table, 256U) == -1);
    assert(ld_table->itemSpace == 1);

    tinyui_app_destroy(app);
}

static void test_table_native_static_text_background_and_getters_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *item;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_native_getters_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_native_getters_round_trip", 3, 3);
    assert(table != 0);

    assert(tinyui_table_set_bg_color(table, 0xA0B0C0U) == 0);
    assert(tinyui_table_set_item_static_text(table, 0, 2, "HEAD") == 0);
    assert(tinyui_table_set_item_align(table, 0, 2, TINYUI_ALIGN_END) == 0);
    assert(tinyui_table_set_cell_editable(table, 2, 2, 1, 12) == 0);
    assert(tinyui_table_set_selected_cell(table, 2, 2) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    item = ldTableGetItem(ld_table, 0, 2);
    assert(item != 0);

    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0xA0B0C0U);
    assert(item->isStaticText == true);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 2), "HEAD") == 0);
    assert(tinyui_table_get_item_align(table, 0, 2) == TINYUI_ALIGN_END);
    assert(tinyui_table_get_item_editable(table, 2, 2) == 1);
    assert(tinyui_table_get_current_row(table) == 2);
    assert(tinyui_table_get_current_column(table) == 2);

    assert(tinyui_table_set_item_static_text(table, 0, 2, 0) == -1);
    assert(tinyui_table_set_bg_color(table, 0x1000000U) == -1);
    assert(tinyui_table_get_item_align(table, 9, 9) == -1);
    assert(tinyui_table_get_item_editable(table, 9, 9) == -1);

    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0xA0B0C0U);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 2), "HEAD") == 0);

    tinyui_app_destroy(app);
}

static void test_table_sync_current_cell_rejects_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    enum tinyui_backend_widget_kind saved_kind;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_corrupted_binding_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_corrupted_binding", 3, 3);
    assert(table != 0);

    assert(tinyui_table_set_current_cell(table, 1, 2) == 0);
    assert(table->current_row == 1);
    assert(table->current_column == 2);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    saved_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 2);
    assert(table->current_row == 1);
    assert(table->current_column == 2);

    backend->kind = saved_kind;
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 2);
    tinyui_app_destroy(app);
}

static void test_table_set_current_and_selected_cell_tolerate_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    enum tinyui_backend_widget_kind saved_kind;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_corrupted_setter_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_corrupted_setter", 3, 3);
    assert(table != 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    saved_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_table_set_current_cell(table, 1, 2) == 0);
    assert(table->current_row == 1);
    assert(table->current_column == 2);
    assert(ld_table->currentRow == 1);
    assert(ld_table->currentColumn == 2);

    assert(tinyui_table_set_selected_cell(table, 2, 1) == 0);
    assert(table->current_row == 2);
    assert(table->current_column == 1);
    assert(ld_table->currentRow == 2);
    assert(ld_table->currentColumn == 1);

    backend->kind = saved_kind;
    tinyui_app_destroy(app);
}

static void test_table_cell_text_and_editable_reject_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_cell_corrupt_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_cell_corrupt", 2, 2);
    assert(table != 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 8) == 0);
    assert(tinyui_table_set_cell_text(table, 0, 0, "before") == 0);
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_table_set_cell_text(table, 0, 0, "after") == -1);
    assert(tinyui_table_get_cell_text(table, 0, 0) == 0);
    assert(tinyui_table_set_cell_editable(table, 0, 0, 1, 8) == -1);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 0, 0), "before") == 0);
    assert(ldTableGetItemEditable(ld_table, 0, 0) == true);

    tinyui_app_destroy(app);
}

static void test_table_item_align_and_region_reject_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    struct tinyui_table_region region;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_item_meta_corrupt_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_item_meta_corrupt", 3, 3);
    assert(table != 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(tinyui_table_set_item_align(table, 1, 1, TINYUI_ALIGN_END) == 0);
    assert(tinyui_table_set_cell_editable(table, 1, 1, 1, 12) == 0);
    region = tinyui_table_get_item_region(table, 1, 1);
    assert(region.width > 0);
    assert(region.height > 0);

    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;
    assert(tinyui_table_set_item_align(table, 1, 1, TINYUI_ALIGN_CENTER) == -1);
    assert(tinyui_table_get_item_align(table, 1, 1) == -1);
    assert(tinyui_table_get_item_editable(table, 1, 1) == -1);
    region = tinyui_table_get_item_region(table, 1, 1);
    assert(region.x == 0);
    assert(region.y == 0);
    assert(region.width == 0);
    assert(region.height == 0);
    assert(ldTableGetItemAlign(ld_table, 1, 1) == ARM_2D_ALIGN_RIGHT);
    assert(ldTableGetItemEditable(ld_table, 1, 1) == true);

    tinyui_app_destroy(app);
}

static void test_table_style_setters_reject_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *item;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_style_corrupt_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_style_corrupt", 3, 3);
    assert(table != 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(tinyui_table_set_background_color(table, 0x102030U) == 0);
    assert(tinyui_table_set_item_width(table, 2, 66) == 0);
    assert(tinyui_table_set_item_height(table, 1, 28) == 0);
    assert(tinyui_table_set_item_color(table, 1, 1, 0xABCDEFU, 0x123456U) == 0);
    assert(tinyui_table_set_item_font(table, 1, 1) == 0);
    assert(tinyui_table_set_item_static_text(table, 0, 2, "HEAD") == 0);
    assert(tinyui_table_set_excel_type(table) == 0);

    item = ldTableGetItem(ld_table, 1, 1);
    assert(item != 0);
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_table_set_background_color(table, 0x556677U) == -1);
    assert(tinyui_table_set_item_width(table, 2, 77) == -1);
    assert(tinyui_table_set_item_height(table, 1, 30) == -1);
    assert(tinyui_table_set_item_color(table, 1, 1, 0x010203U, 0x040506U) == -1);
    assert(tinyui_table_set_item_font(table, 1, 1) == -1);
    assert(tinyui_table_set_item_static_text(table, 0, 2, "TAIL") == -1);
    assert(tinyui_table_set_excel_type(table) == -1);

    assert(ldTableGetBackgroundColor(ld_table) == __RGB(219,219,219));
    assert(ldTableGetItemWidth(ld_table, 2) == 71);
    assert(ldTableGetItemHeight(ld_table, 1) == 18);
    assert(ldTableGetItemTextColor(ld_table, 1, 1) == (ldColor)0xABCDEFU);
    assert(ldTableGetItemBackgroundColor(ld_table, 1, 1) == (ldColor)0x123456U);
    assert(ldTableGetItemFont(ld_table, 1, 1) == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(item->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);

    tinyui_app_destroy(app);
}

static void test_table_image_and_button_reject_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    ldTable_t *ld_table;
    ldTableItem_t *image_item;
    ldTableItem_t *button_item;
    arm_2d_tile_t release_tile = {0};
    arm_2d_tile_t press_tile = {0};
    arm_2d_tile_t release_mask_tile = {0};
    arm_2d_tile_t press_mask_tile = {0};
    struct tinyui_image_source image_src = { .img_tile = &release_tile, .mask_tile = &release_mask_tile };
    struct tinyui_image_source release_src = { .img_tile = &release_tile, .mask_tile = &release_mask_tile };
    struct tinyui_image_source press_src = { .img_tile = &press_tile, .mask_tile = &press_mask_tile };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_image_button_corrupt_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_image_button_corrupt", 2, 2);
    assert(table != 0);

    assert(tinyui_table_set_item_image(table, 0, 0, 4, 4, &image_src, 0xFFFFFFU) == 0);
    assert(tinyui_table_set_item_button(table, 0, 1, 2, 2,
                                        &release_src, 0xAAAAAAU,
                                        &press_src, 0xBBBBBBU,
                                        1) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    image_item = ldTableGetItem(ld_table, 0, 0);
    button_item = ldTableGetItem(ld_table, 0, 1);
    assert(image_item != 0);
    assert(button_item != 0);
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_table_set_item_image(table, 0, 0, 6, 6, &image_src, 0x123456U) == -1);
    assert(tinyui_table_set_item_button(table, 0, 1, 3, 3,
                                        &release_src, 0x111111U,
                                        &press_src, 0x222222U,
                                        0) == -1);

    assert(image_item->tLocation.iX == 4);
    assert(image_item->tLocation.iY == 4);
    assert(image_item->ptPressImgTile == image_src.img_tile);
    assert(image_item->ptPressMaskTile == image_src.mask_tile);
    assert(image_item->releaseImgMaskColor == (ldColor)0xFFFFFFU);

    assert(button_item->tLocation.iX == 2);
    assert(button_item->tLocation.iY == 2);
    assert(button_item->ptReleaseImgTile == release_src.img_tile);
    assert(button_item->ptReleaseMaskTile == release_src.mask_tile);
    assert(button_item->ptPressImgTile == press_src.img_tile);
    assert(button_item->ptPressMaskTile == press_src.mask_tile);
    assert(button_item->releaseImgMaskColor == (ldColor)0xAAAAAAU);
    assert(button_item->pressImgMaskColor == (ldColor)0xBBBBBBU);
    assert(button_item->isButton == true);
    assert(button_item->isCheckable == true);

    tinyui_app_destroy(app);
}

static void test_table_get_keyboard_binding_rejects_corrupted_backend_binding(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    struct tinyui_widget *keyboard_backend;
    enum tinyui_backend_widget_kind saved_kind;
    unsigned int keyboard_binding = 0;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_keyboard_corrupted_root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "table_keyboard_corrupted_keyboard");
    assert(keyboard != 0);
    table = tinyui_table_create(win, "table_keyboard_corrupted", 3, 3);
    assert(table != 0);

    keyboard_backend = &keyboard->widget;
    assert(keyboard_backend->ld_widget != 0);
    assert(tinyui_table_set_keyboard_binding(table, keyboard_backend->ld_name_id) == 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    saved_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == keyboard_backend->ld_name_id);
    assert(table->keyboard_binding == keyboard_backend->ld_name_id);

    backend->kind = saved_kind;
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == keyboard_backend->ld_name_id);
    tinyui_app_destroy(app);
}

static void test_table_set_keyboard_widget_uses_native_id(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_keyboard *keyboard;
    ldTable_t *ld_table;
    unsigned int keyboard_binding = 0;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_keyboard_widget_root");
    assert(win != 0);
    table = tinyui_table_create(win, "table_keyboard_widget", 3, 3);
    keyboard = tinyui_keyboard_create(win, "table_keyboard_native");
    assert(table != 0);
    assert(keyboard != 0);
    assert(table->widget.ld_widget != 0);
    ld_table = (ldTable_t *)table->widget.ld_widget;
    assert(ld_table != 0);

    assert(tinyui_table_set_keyboard_widget(table, keyboard) == 0);
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == keyboard->widget.ld_name_id);
    assert(table->keyboard_binding == keyboard->widget.ld_name_id);
    assert(ld_table->kbNameId == keyboard->widget.ld_name_id);

    assert(tinyui_table_set_keyboard_widget(0, keyboard) == -1);
    assert(tinyui_table_set_keyboard_widget(table, 0) == -1);
    tinyui_app_destroy(app);
}

static void test_table_set_keyboard_widget_rejects_cross_owner(void)
{
    struct tinyui_app *app_a;
    struct tinyui_app *app_b;
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_table *table;
    struct tinyui_keyboard *keyboard_a;
    struct tinyui_keyboard *keyboard_b;
    ldTable_t *ld_table;

    app_a = tinyui_app_create();
    app_b = tinyui_app_create();
    assert(app_a != 0);
    assert(app_b != 0);
    win_a = tinyui_window_create(app_a, "table_owner_a");
    win_b = tinyui_window_create(app_b, "table_owner_b");
    assert(win_a != 0);
    assert(win_b != 0);

    table = tinyui_table_create(win_a, "table_cross_owner", 3, 3);
    keyboard_a = tinyui_keyboard_create(win_a, "table_keyboard_same_owner");
    keyboard_b = tinyui_keyboard_create(win_b, "table_keyboard_other_owner");
    assert(table != 0);
    assert(keyboard_a != 0);
    assert(keyboard_b != 0);

    ld_table = (ldTable_t *)table->widget.ld_widget;
    assert(ld_table != 0);
    assert(tinyui_table_set_keyboard_widget(table, keyboard_a) == 0);
    assert(ld_table->kbNameId == keyboard_a->widget.ld_name_id);
    assert(table->keyboard_binding == keyboard_a->widget.ld_name_id);

    assert(tinyui_table_set_keyboard_widget(table, keyboard_b) == -1);
    assert(ld_table->kbNameId == keyboard_a->widget.ld_name_id);
    assert(table->keyboard_binding == keyboard_a->widget.ld_name_id);

    tinyui_app_destroy(app_b);
    tinyui_app_destroy(app_a);
}

static void test_table_r4_aliases_and_native_getters_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    struct tinyui_keyboard *keyboard;
    struct tinyui_widget *backend;
    struct tinyui_widget *keyboard_backend;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    unsigned int keyboard_binding = 0;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_r4_alias_root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "table_r4_alias_keyboard");
    assert(keyboard != 0);
    table = tinyui_table_create(win, "table_r4_alias", 3, 3);
    assert(table != 0);

    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);
    keyboard_backend = &keyboard->widget;
    assert(keyboard_backend->ld_widget != 0);

    assert(tinyui_table_set_keyboard(table, keyboard_backend->ld_name_id) == 0);
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == keyboard_backend->ld_name_id);
    assert(ld_table->kbNameId == keyboard_backend->ld_name_id);

    assert(tinyui_table_set_item_text(table, 1, 1, "CELL") == 0);
    assert(strcmp(tinyui_table_get_item_text(table, 1, 1), "CELL") == 0);
    assert(strcmp((const char *)ldTableGetItemText(ld_table, 1, 1), "CELL") == 0);

    assert(tinyui_table_set_item_editable(table, 1, 1, 1, 9) == 0);
    assert(tinyui_table_get_item_editable(table, 1, 1) == 1);
    assert(ldTableGetItemEditable(ld_table, 1, 1) == true);

    assert(tinyui_table_set_background_color(table, 0x102030U) == 0);
    assert(tinyui_table_get_background_color(table) == (unsigned int)ldTableGetBackgroundColor(ld_table));
    assert(ldTableGetBackgroundColor(ld_table) == (ldColor)0x102030U);

    assert(tinyui_table_set_align_grid(table, 1) == 0);
    assert(tinyui_table_get_align_grid(table) == 1);
    assert(ldTableGetAlignGrid(ld_table) == true);

    assert(tinyui_table_set_item_width(table, 2, 66) == 0);
    assert(tinyui_table_set_item_height(table, 1, 28) == 0);
    assert(tinyui_table_set_item_color(table, 1, 1, 0xABCDEFU, 0x123456U) == 0);
    assert(tinyui_table_set_item_font(table, 1, 1) == 0);
    assert(tinyui_table_set_item_align(table, 1, 1, TINYUI_ALIGN_CENTER) == 0);
    assert(tinyui_table_set_item_select(table, 1, 1, 1) == 0);

    item = (ldTableItem_t *)tinyui_table_get_item(table, 1, 1);
    assert(item != 0);
    assert(item == ldTableGetItem(ld_table, 1, 1));
    assert(tinyui_table_get_item_font(table, 1, 1) == ldTableGetItemFont(ld_table, 1, 1));
    assert(tinyui_table_get_item_height(table, 1) == ldTableGetItemHeight(ld_table, 1));
    assert(tinyui_table_get_item_width(table, 2) == ldTableGetItemWidth(ld_table, 2));
    assert(tinyui_table_get_item_text_color(table, 1, 1) == (unsigned int)ldTableGetItemTextColor(ld_table, 1, 1));
    assert(tinyui_table_get_item_background_color(table, 1, 1) ==
           (unsigned int)ldTableGetItemBackgroundColor(ld_table, 1, 1));
    assert(tinyui_table_get_item_align(table, 1, 1) == TINYUI_ALIGN_CENTER);
    assert(tinyui_table_get_current_row(table) == 1);
    assert(tinyui_table_get_current_column(table) == 1);

    assert(tinyui_table_show_keyboard(table) == 0);
    assert(tinyui_tabel_show_keyboard(table) == 0);
    assert(tinyui_widget_is_hidden(&keyboard->widget) == 0);

    assert(tinyui_table_set_item_select(table, 1, 1, 0) == -1);
    assert(tinyui_table_get_item(table, 9, 9) == 0);
    assert(tinyui_table_get_item_font(table, 9, 9) == 0);
    assert(tinyui_table_get_item_height(table, 9) == -1);
    assert(tinyui_table_get_item_width(table, 9) == -1);

    tinyui_app_destroy(app);
}

static void test_table_create_with_props_applies_keyboard_and_size_contract(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_keyboard *keyboard;
    struct tinyui_table *table;
    struct tinyui_widget *backend;
    struct tinyui_widget *keyboard_backend;
    ldTable_t *ld_table;
    unsigned int keyboard_binding = 0;
    const struct tinyui_table_props props = {
        .id = "table_props_contract",
        .keyboard_binding = 0,
        .rows = 3,
        .columns = 3,
        .style_class = "table-props",
        .user_data = (void *)0x1234,
        .width = 144,
        .height = 72,
        .bg_color = 0x102030U,
        .text_color = 0x405060U,
        .border_color = 0x708090U,
        .radius = 6,
        .padding = 4,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_props_root");
    assert(win != 0);
    keyboard = tinyui_keyboard_create(win, "table_props_keyboard");
    assert(keyboard != 0);

    keyboard_backend = &keyboard->widget;
    assert(keyboard_backend->ld_widget != 0);

    {
        struct tinyui_table_props bound_props = props;
        bound_props.keyboard_binding = keyboard_backend->ld_name_id;
        table = tinyui_table_create_with_props(win, &bound_props);
    }

    assert(table != 0);
    backend = &table->widget;
    assert(backend->ld_widget != 0);
    ld_table = (ldTable_t *)backend->ld_widget;
    assert(ld_table != 0);

    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == keyboard_backend->ld_name_id);
    assert(table->keyboard_binding == keyboard_backend->ld_name_id);
    assert(ld_table->kbNameId == keyboard_backend->ld_name_id);
    assert(table->widget.width == 144);
    assert(table->widget.height == 72);
    assert(table->widget.user_data == (void *)0x1234);
    assert(table->widget.bg_color == 0x102030U);
    assert(table->widget.text_color == 0x405060U);
    assert(table->widget.border_color == 0x708090U);
    assert(table->widget.radius == 6);
    assert(table->widget.padding == 4);
    assert(table->widget.style_class == props.style_class);
    assert(tinyui_table_set_current_cell(table, 2, 1) == 0);
    assert(tinyui_table_get_current_row(table) == 2);
    assert(tinyui_table_get_current_column(table) == 1);

    tinyui_app_destroy(app);
}

static void test_table_set_excel_type_round_trip(struct tinyui_window *win)
{
    struct tinyui_table *table = tinyui_table_create(win, "table_excel", 3, 4);
    assert(table != 0);
    assert(tinyui_table_set_excel_type(table) == 0);
}

static void test_table_set_item_image_round_trip(struct tinyui_window *win)
{
    struct tinyui_table *table = tinyui_table_create(win, "table_img", 2, 2);
    arm_2d_tile_t img_tile = {0};
    arm_2d_tile_t mask_tile = {0};
    struct tinyui_image_source src = { .img_tile = &img_tile, .mask_tile = &mask_tile };

    assert(table != 0);
    assert(tinyui_table_set_cell_text(table, 0, 0, "cell") == 0);
    assert(tinyui_table_set_item_image(table, 0, 0, 4, 4, &src, 0xFFFFFFU) == 0);
}

static void test_table_set_item_button_round_trip(struct tinyui_window *win)
{
    struct tinyui_table *table = tinyui_table_create(win, "table_btn", 2, 2);
    arm_2d_tile_t rel_tile = {0};
    arm_2d_tile_t press_tile = {0};
    arm_2d_tile_t rel_mask_tile = {0};
    arm_2d_tile_t press_mask_tile = {0};
    struct tinyui_image_source rel_src = { .img_tile = &rel_tile, .mask_tile = &rel_mask_tile };
    struct tinyui_image_source press_src = { .img_tile = &press_tile, .mask_tile = &press_mask_tile };

    assert(table != 0);
    assert(tinyui_table_set_cell_text(table, 0, 1, "btn_cell") == 0);
    assert(tinyui_table_set_item_button(table, 0, 1, 2, 2,
                                         &rel_src, 0xFFFFFFU,
                                         &press_src, 0xFFFFFFU,
                                         0) == 0);
}

static void test_table_item_image_rejects_null_source(struct tinyui_window *win)
{
    struct tinyui_table *table = tinyui_table_create(win, "table_null_src", 2, 2);
    assert(table != 0);
    assert(tinyui_table_set_cell_text(table, 0, 0, "cell") == 0);
    assert(tinyui_table_set_item_image(table, 0, 0, 4, 4, 0, 0xFFFFFFU) == -1);
}

static void test_table_create_with_props_keyboard_failure_rolls_back_attached_child(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    ldBase_t *win_ld;
    ldBase_t *tail_ld;
    ldBase_t *next_before_ld = 0;
    struct tinyui_table *probe;
    struct tinyui_table *table;
    const struct tinyui_widget *disposed_backend;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_props_fail_root");
    assert(win != 0);

    win_ld = (ldBase_t *)win->widget.ld_widget;
    tail_ld = ldBaseGetChildList(win_ld);
    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    tinyui_table_test_reset_state();
    tinyui_test_capture_destroyed_widget_snapshot(0);
    probe = tinyui_table_create(win, "table_props_fail_probe", 2, 2);
    assert(probe != 0);
    assert(tinyui_widget_destroy(&probe->widget) == 0);

    tinyui_table_test_fail_next_set_keyboard_binding();
    table = tinyui_table_create_with_props(
        win,
        &(struct tinyui_table_props){
            .id = "table_props_fail_keyboard",
            .rows = 2,
            .columns = 2,
            .keyboard_binding = 7U,
        });

    assert(table == 0);
    disposed_backend = tinyui_table_test_last_disposed_backend();
    assert(disposed_backend != 0);
    assert(disposed_backend->kind == TINYUI_BACKEND_WIDGET_TABLE);
    assert(disposed_backend->owner == 0);
    assert(disposed_backend->ld_event_bridge_scene == 0);
    assert(disposed_backend->ld_event_bridge_sender == 0);
    assert(disposed_backend->ld_event_bridge_next == 0);
    assert(disposed_backend->ld_widget == 0);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }

    tinyui_app_destroy(app);
}

static void test_table_create_with_props_accepts_keyboard_binding_sentinel_default(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_table *table;
    unsigned int keyboard_binding = 123U;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "table_keyboard_sentinel_root");
    assert(win != 0);

    table = tinyui_table_create_with_props(
        win,
        &(struct tinyui_table_props){
            .id = "table_keyboard_sentinel",
            .rows = 2,
            .columns = 2,
            .keyboard_binding = 0U,
        });

    assert(table != 0);
    assert(tinyui_table_get_keyboard_binding(table, &keyboard_binding) == 0);
    assert(keyboard_binding == 0U);

    tinyui_app_destroy(app);
}

static void test_table_props_source_no_longer_uses_has_keyboard_binding(void)
{
    assert(tinyui_test_source_contains("tinyui/include/widgets/table.h", "has_keyboard_binding") == 0);
    assert(tinyui_test_source_contains("tinyui/src/widgets/table.c", "props->has_keyboard_binding") == 0);
}

static void test_table_legacy_bind_host_symbol_is_removed(void)
{
    assert_self_binary_lacks_symbol("tinyui_backend_table_bind_host");
}

static void test_table_navigate_and_sync_current_cell_backend_symbols_are_no_longer_public(void)
{
    assert_archive_lacks_member("../../libtinyui_core.a", "backend_table.c.o");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_keyboard_binding");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_get_keyboard_binding");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_cell_text");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_get_cell_text");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_cell_editable");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_align");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_get_item_align");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_get_item_editable");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_get_item_region");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_excel_type");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_width");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_height");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_color");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_bg_color");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_static_text");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_font");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_image");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_item_button");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_selected_cell");
    assert_archive_lacks_symbol("../../libtinyui_core.a",
                                "tinyui_backend_table_set_current_cell");
    assert_self_binary_lacks_symbol("tinyui_backend_table_navigate");
    assert_self_binary_lacks_symbol("tinyui_backend_table_sync_current_cell");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_cell_text");
    assert_self_binary_lacks_symbol("tinyui_backend_table_get_cell_text");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_cell_editable");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_align");
    assert_self_binary_lacks_symbol("tinyui_backend_table_get_item_align");
    assert_self_binary_lacks_symbol("tinyui_backend_table_get_item_editable");
    assert_self_binary_lacks_symbol("tinyui_backend_table_get_item_region");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_excel_type");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_width");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_height");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_color");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_bg_color");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_static_text");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_font");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_image");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_item_button");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_selected_cell");
    assert_self_binary_lacks_symbol("tinyui_backend_table_set_current_cell");
    test_table_create_with_props_accepts_keyboard_binding_sentinel_default();
    test_table_props_source_no_longer_uses_has_keyboard_binding();
}

static void test_table_create_uses_shared_leaf_helper(void)
{
    assert(test_source_has_symbol(test_table_source_path, "tinyui_widget_create_leaf"));
}

int main(int argc, char **argv)
{
    (void)argc;
    test_self_binary_path = argv[0];
    init_table_source_path();
    test_table_create_builds_direct_backend_mapping();
    test_table_current_cell_matches_backend_truth();
    test_table_edit_commit_updates_model_and_visible_text();
    test_table_reuses_editable_cell_contract();
    test_table_double_press_blank_excel_cell_keeps_font_for_editing();
    test_table_final_release_contract_covers_non_commit_exit_boundary();
    test_table_native_item_image_button_and_excel_type_round_trip();
    test_table_native_size_align_color_font_region_and_navigation_round_trip();
    test_table_item_space_round_trip();
    test_table_native_static_text_background_and_getters_round_trip();
    test_table_sync_current_cell_rejects_corrupted_backend_binding();
    test_table_set_current_and_selected_cell_tolerate_corrupted_backend_binding();
    test_table_cell_text_and_editable_reject_corrupted_backend_binding();
    test_table_item_align_and_region_reject_corrupted_backend_binding();
    test_table_style_setters_reject_corrupted_backend_binding();
    test_table_image_and_button_reject_corrupted_backend_binding();
    test_table_get_keyboard_binding_rejects_corrupted_backend_binding();
    test_table_set_keyboard_widget_uses_native_id();
    test_table_set_keyboard_widget_rejects_cross_owner();
    test_table_r4_aliases_and_native_getters_round_trip();
    test_table_create_with_props_applies_keyboard_and_size_contract();
    test_table_create_with_props_keyboard_failure_rolls_back_attached_child();
    test_table_legacy_bind_host_symbol_is_removed();
    test_table_navigate_and_sync_current_cell_backend_symbols_are_no_longer_public();
    test_table_create_uses_shared_leaf_helper();

    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    test_table_set_excel_type_round_trip(win);
    test_table_set_item_image_round_trip(win);
    test_table_set_item_button_round_trip(win);
    test_table_item_image_rejects_null_source(win);
    tinyui_app_destroy(app);
    return 0;
}
