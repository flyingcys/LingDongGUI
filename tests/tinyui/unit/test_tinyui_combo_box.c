#include "app.h"
#include "combo_box.h"
#include "window.h"
#include "../../../src/gui/ldComboBox.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static const char *test_self_binary_path = 0;
static const char *test_combo_box_source_path =
    "/Users/cys/embedded/LingDongGUI/tinyui/src/widgets/combo_box.c";

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];

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
    assert(pclose(pipe) == 0);
}

static void assert_source_lacks_function_definition(const char *source_path, const char *symbol)
{
    char command[1024];

    assert(source_path != 0);
    assert(symbol != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import re\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "symbol = sys.argv[2]\n"
             "pattern = re.compile(r'(^|\\n)\\s*(?:static\\s+)?[A-Za-z_][A-Za-z0-9_\\s\\*]*\\b' + re.escape(symbol) + r'\\s*\\(', re.MULTILINE)\n"
             "raise SystemExit(1 if pattern.search(text) else 0)\n"
             "PY",
             source_path,
             symbol);
    assert(system(command) == 0);
}

static unsigned int encode_rgb_to_ld_color(unsigned int rgb)
{
    unsigned int red = (rgb >> 16) & 0xFFU;
    unsigned int green = (rgb >> 8) & 0xFFU;
    unsigned int blue = rgb & 0xFFU;

    return ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
}

static int combo_selected_count = 0;
static int combo_selected_index = -1;
static void *combo_selected_user_data = 0;
static int native_combo_clicked_count = 0;
static int native_combo_clicked_index = -1;

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void on_combo_selected(struct tinyui_combo_box *combo_box, int index, void *user_data)
{
    combo_selected_count++;
    combo_selected_index = index;
    combo_selected_user_data = user_data;
    assert(combo_box != 0);
}

static bool on_native_combo_clicked_probe(ld_scene_t *scene, ldMsg_t msg)
{
    (void)scene;
    native_combo_clicked_count++;
    native_combo_clicked_index = (int)msg.value;
    return false;
}

static void test_combo_box_open_close_and_selected_item_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    int is_open = -1;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo");
    assert(combo_box != 0);
    assert(tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "display", "Display") == 0);
    assert(tinyui_combo_box_set_selected_index(combo_box, 0) == 0);

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct tinyui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(ldMsgConnect(backend->ld_widget, SIGNAL_CLICKED_ITEM, on_native_combo_clicked_probe) == true);
    native_combo_clicked_count = 0;
    native_combo_clicked_index = -1;

    assert(tinyui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 1);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_HOLD_DOWN,
                     make_signal_value_xy(10, 112)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 112)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(native_combo_clicked_count == 1);
    assert(native_combo_clicked_index == 2);
    assert(tinyui_combo_box_get_selected_index(combo_box) == 2);
    assert(((struct tinyui_backend_widget *)combo_box->widget.backend_widget)->value == 2);

    ld_combo_box->isExpand = false;
    assert(tinyui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);
    tinyui_app_destroy(app);
}

static void test_combo_box_reuses_selection_contract(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    struct tinyui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    int cookie = 11;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_contract");
    assert(combo_box != 0);
    assert(tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "display", "Display") == 0);
    tinyui_combo_box_set_on_selected(combo_box, on_combo_selected, &cookie);
    combo_selected_count = 0;
    combo_selected_index = -1;
    combo_selected_user_data = 0;

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct tinyui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(tinyui_combo_box_set_selected_index(combo_box, 0) == 0);
    assert(tinyui_widget_set_enabled(&combo_box->widget, 0) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 48)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(combo_selected_count == 0);
    assert(combo_selected_index == -1);
    assert(combo_selected_user_data == 0);
    assert(tinyui_combo_box_get_selected_index(combo_box) == 0);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 0);
    assert(backend->value == 0);
    assert(backend->last_signal == TINYUI_BACKEND_SIGNAL_NONE);
    assert(backend->dispatch_count == 0);
    tinyui_app_destroy(app);
}

static void test_combo_box_final_visual_and_selection_contract_is_release_ready(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    int is_open = -1;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "combo_release_root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_release_ready");
    assert(combo_box != 0);
    assert(tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "display", "Display") == 0);
    assert(tinyui_combo_box_set_selected_index(combo_box, 1) == 0);

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_COMBO_BOX);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(tinyui_combo_box_get_selected_index(combo_box) == 1);
    assert(backend->value == 1);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 1);
    assert(tinyui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    tinyui_app_destroy(app);
}

static void test_combo_box_native_color_item_max_and_dropdown_image_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    arm_2d_tile_t dropdown_tile = {0};
    arm_2d_tile_t dropdown_mask_tile = {0};
    struct tinyui_image_source dropdown_source = {
        .img_tile = &dropdown_tile,
        .mask_tile = &dropdown_mask_tile,
    };
    struct tinyui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = &dropdown_mask_tile,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "combo_native_root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_native");
    assert(combo_box != 0);
    assert(tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "display", "Display") == 0);

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(tinyui_combo_box_set_text_color(combo_box, 0x445566U) == 0);
    assert(tinyui_combo_box_set_bg_color(combo_box, 0x112233U) == 0);
    assert(tinyui_combo_box_set_frame_color(combo_box, 0x778899U) == 0);
    assert(tinyui_combo_box_set_select_color(combo_box, 0xAABBCCU) == 0);
    assert(tinyui_combo_box_set_item_max(combo_box, 5) == 0);
    assert(tinyui_combo_box_set_dropdown_source(combo_box, &dropdown_source) == 0);
    assert(tinyui_combo_box_set_dropdown_source(combo_box, &invalid_source) == -1);

    assert((unsigned int)ld_combo_box->textColor == encode_rgb_to_ld_color(0x445566U));
    assert((unsigned int)ld_combo_box->bgColor == encode_rgb_to_ld_color(0x112233U));
    assert((unsigned int)ld_combo_box->frameColor == encode_rgb_to_ld_color(0x778899U));
    assert((unsigned int)ld_combo_box->selectColor == encode_rgb_to_ld_color(0xAABBCCU));
    assert(ld_combo_box->itemMax == 5);
    assert(ld_combo_box->ptDropdownImgTile == dropdown_source.img_tile);
    assert(ld_combo_box->ptDropdownMaskTile == dropdown_source.mask_tile);

    tinyui_app_destroy(app);
}

static void test_combo_box_native_item_text_readback_matches_backend_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "combo_text_root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_text");
    assert(combo_box != 0);
    assert(tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "display", "Display") == 0);

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(tinyui_combo_box_get_text(combo_box, 0) == ldComboBoxGetText(ld_combo_box, 0));
    assert(tinyui_combo_box_get_text(combo_box, 1) == ldComboBoxGetText(ld_combo_box, 1));
    assert(tinyui_combo_box_get_text(combo_box, 2) == ldComboBoxGetText(ld_combo_box, 2));
    assert(tinyui_combo_box_get_text(combo_box, 3) == 0);

    tinyui_app_destroy(app);
}

static void test_combo_box_native_api_aliases_match_backend_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    const char *item_ids[] = {"wifi", "bluetooth", "display"};
    const char *texts[] = {"Wi-Fi", "Bluetooth", "Display"};
    arm_2d_tile_t dropdown_tile = {0};
    struct tinyui_image_source dropdown_source = {
        .img_tile = &dropdown_tile,
        .mask_tile = 0,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "combo_alias_root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_alias");
    assert(combo_box != 0);
    assert(tinyui_combo_box_set_static_items(combo_box, item_ids, texts, 3) == 0);
    assert(tinyui_combo_box_set_select_item(combo_box, 1) == 0);
    assert(tinyui_combo_box_get_select_item(combo_box) == 1);
    assert(tinyui_combo_box_set_background_color(combo_box, 0x010203U) == 0);
    assert(tinyui_combo_box_set_text_color(combo_box, 0x111213U) == 0);
    assert(tinyui_combo_box_set_frame_color(combo_box, 0x212223U) == 0);
    assert(tinyui_combo_box_set_dropdown_image(combo_box, &dropdown_source) == 0);

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 1);
    assert(ld_combo_box->itemCount == 3);
    assert(ld_combo_box->ptDropdownImgTile == dropdown_source.img_tile);
    assert(combo_box->widget.bg_color == 0x010203U);
    assert(combo_box->widget.text_color == 0x111213U);
    assert(combo_box->widget.border_color == 0x212223U);
    tinyui_app_destroy(app);
}

static void test_combo_box_shared_base_aliases_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    ldBase_t *ld_base;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "combo_base_root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_base");
    assert(combo_box != 0);
    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_widget_set_pos(&combo_box->widget, 12, 34) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 12);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 34);
    assert(tinyui_widget_set_visible(&combo_box->widget, 0) == 0);
    assert(tinyui_widget_set_opacity(&combo_box->widget, 77) == 0);
    assert(tinyui_widget_set_selectable(&combo_box->widget, 1) == 0);
    assert(tinyui_widget_set_selected(&combo_box->widget, 1) == 0);
    assert(tinyui_widget_set_corner(&combo_box->widget, 1) == 0);

    assert(ld_base->isHidden == true);
    assert(ld_base->opacity == 77);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelected == true);
    assert(ld_base->isCorner == true);
    tinyui_app_destroy(app);
}

static void test_combo_box_uses_native_static_items_contract(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_combo_box *combo_box;
    struct tinyui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "combo_static_root");
    assert(win != 0);
    combo_box = tinyui_combo_box_create(win, "combo_static");
    assert(combo_box != 0);
    assert(tinyui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_combo_box_add_item(combo_box, "display", "Display") == 0);

    backend = (struct tinyui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(ld_combo_box->isStatic == true);
    assert(ld_combo_box->itemCount == 3);
    assert(ld_combo_box->ppItemStrGroup == (uint8_t **)combo_box->backend_item_texts);
    assert(ldComboBoxGetText(ld_combo_box, 0) == (uint8_t *)combo_box->backend_item_texts[0]);
    assert(ldComboBoxGetText(ld_combo_box, 1) == (uint8_t *)combo_box->backend_item_texts[1]);
    assert(ldComboBoxGetText(ld_combo_box, 2) == (uint8_t *)combo_box->backend_item_texts[2]);

    tinyui_app_destroy(app);
}

static void test_combo_box_sync_selected_index_round_trip(struct tinyui_window *win)
{
    struct tinyui_combo_box *cb = tinyui_combo_box_create(win, "cb_sync");
    const char *ids[] = {"item_a", "item_b", "item_c"};
    const char *texts[] = {"A", "B", "C"};
    int selected;

    assert(cb != 0);
    assert(tinyui_combo_box_set_static_items(cb, ids, texts, 3) == 0);
    assert(tinyui_combo_box_set_selected_index(cb, 1) == 0);
    selected = tinyui_combo_box_get_selected_index(cb);
    assert(selected == 1);
}

static void test_combo_box_get_open_round_trip(struct tinyui_window *win)
{
    struct tinyui_combo_box *cb = tinyui_combo_box_create(win, "cb_open");
    int is_open = -1;

    assert(cb != 0);
    assert(tinyui_combo_box_is_open(cb, &is_open) == 0);
    assert(is_open == 0);
}

static void test_combo_box_internal_helpers_no_longer_use_tinyui_backend_prefix(void)
{
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_rgb_to_ld_color");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_get_ld");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_native_slot");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_combo_box_create_backend_local");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_combo_box_props_are_valid");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_combo_box_dispose_partial");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_items");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_text_color");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_bg_color");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_frame_color");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_select_color");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_item_max");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_dropdown_source");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_set_selected_index");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_get_selected_index");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_get_text");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_sync_selected_index");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_bind_host");
    assert_source_lacks_function_definition(test_combo_box_source_path, "tinyui_backend_combo_box_get_open");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_items");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_text_color");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_bg_color");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_frame_color");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_select_color");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_item_max");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_dropdown_source");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_set_selected_index");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_get_selected_index");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_get_text");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_sync_selected_index");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_bind_host");
    assert_self_binary_lacks_symbol("tinyui_backend_combo_box_get_open");
}

int main(void)
{
    Dl_info self_info;

    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    test_combo_box_open_close_and_selected_item_truth();
    test_combo_box_reuses_selection_contract();
    test_combo_box_final_visual_and_selection_contract_is_release_ready();
    test_combo_box_native_color_item_max_and_dropdown_image_round_trip();
    test_combo_box_native_item_text_readback_matches_backend_truth();
    test_combo_box_native_api_aliases_match_backend_truth();
    test_combo_box_shared_base_aliases_round_trip();
    test_combo_box_uses_native_static_items_contract();

    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    test_combo_box_sync_selected_index_round_trip(win);
    test_combo_box_get_open_round_trip(win);
    test_combo_box_internal_helpers_no_longer_use_tinyui_backend_prefix();
    tinyui_app_destroy(app);
    return 0;
}
