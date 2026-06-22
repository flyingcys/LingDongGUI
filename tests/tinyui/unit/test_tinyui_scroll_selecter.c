#include "app.h"
#include "scroll_selecter.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldScrollSelecter.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int test_repo_root(char *buffer, size_t size)
{
    const char *source = __FILE__;
    const char *suffix = "/tests/tinyui/unit/test_tinyui_scroll_selecter.c";
    const char *match = strstr(source, suffix);
    size_t root_len;

    if (buffer == 0 || size == 0 || match == 0) {
        return -1;
    }

    root_len = (size_t)(match - source);
    if (root_len + 1 > size) {
        return -1;
    }

    memcpy(buffer, source, root_len);
    buffer[root_len] = '\0';
    return 0;
}

static int test_source_has_function_definition(const char *relative_path, const char *name)
{
    char repo_root[PATH_MAX];
    char path[PATH_MAX];
    char line[512];
    FILE *fp;

    if (test_repo_root(repo_root, sizeof(repo_root)) != 0) {
        return 0;
    }
    if (snprintf(path, sizeof(path), "%s/%s", repo_root, relative_path) < 0) {
        return 0;
    }

    fp = fopen(path, "r");
    if (fp == 0) {
        return 0;
    }

    while (fgets(line, sizeof(line), fp) != 0) {
        if (strstr(line, name) != 0 && strstr(line, "(") != 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

static void assert_source_lacks_function_definition(const char *relative_path, const char *name)
{
    assert(!test_source_has_function_definition(relative_path, name));
}

static unsigned int encode_rgb_to_ld_color(unsigned int rgb)
{
    unsigned int red = (rgb >> 16) & 0xFFU;
    unsigned int green = (rgb >> 8) & 0xFFU;
    unsigned int blue = rgb & 0xFFU;

    return ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
}

static void test_scroll_selecter_selected_item_matches_backend_truth(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldScrollSelecter_t *ld_scroll_selecter;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(tinyui_scroll_selecter_set_selected_index(scroll_selecter, 0) == 0);

    backend = &scroll_selecter->widget;
    parent_backend = &win->widget;
    assert(parent_backend->ld_widget != 0);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);
    assert(((ldBase_t *)ld_scroll_selecter)->pInfo == backend);

    assert(tinyui_scroll_selecter_set_selected_index(scroll_selecter, 1) == 0);
    ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, 2);
    assert(tinyui_scroll_selecter_get_selected_index(scroll_selecter) == 2);
    tinyui_app_destroy(app);
}

static void test_scroll_selecter_legacy_backend_constructor_is_disabled(struct tinyui_window *win)
{
    (void)win;
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_create_scroll_selecter") == 0);
}

static void test_scroll_selecter_internal_seams_are_tinyui_named(void)
{
    const char *widget_source = "tinyui/src/widgets/scroll_selecter.c";

    assert_source_lacks_function_definition(widget_source, "picoui_scroll_selecter_props_are_valid");
    assert_source_lacks_function_definition(widget_source, "picoui_scroll_selecter_rgb_to_ld_color");
    assert_source_lacks_function_definition(widget_source, "picoui_scroll_selecter_backend_from_widget");
    assert_source_lacks_function_definition(widget_source, "picoui_scroll_selecter_ld_from_backend");
    assert_source_lacks_function_definition(widget_source, "picoui_scroll_selecter_selected_text_from_public_state");

    assert(test_source_has_function_definition(widget_source, "tinyui_scroll_selecter_props_valid"));
    assert(test_source_has_function_definition(widget_source, "tinyui_scroll_selecter_ld_depose_cb"));
    assert(test_source_has_function_definition(widget_source, "tinyui_scroll_selecter_rollback"));
    assert(test_source_has_function_definition(widget_source, "tinyui_scroll_selecter_selected_text_from_public_state"));

    assert_source_lacks_function_definition(widget_source, "tinyui_scroll_selecter_rgb_to_ld_color");
    assert_source_lacks_function_definition(widget_source, "tinyui_scroll_selecter_get_ld");
    assert_source_lacks_function_definition(widget_source, "tinyui_scroll_selecter_backend_from_widget");
    assert_source_lacks_function_definition(widget_source, "tinyui_scroll_selecter_widget_is_valid");

    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_items");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_text_color");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_bg_color");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_indicator_color");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_bg_source");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_indicator_source");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_transparent");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_speed");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_select_text");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_selected_index");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_get_selected_index");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_get_selected_text");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_sync_selected_index");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_set_edit_mode");
    assert_source_lacks_function_definition(widget_source, "tinyui_backend_scroll_selecter_get_edit_mode");
}

static void test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    int is_edit = -1;

    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_mode");
    assert(scroll_selecter != 0);

    backend = &scroll_selecter->widget;
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(tinyui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert(tinyui_scroll_selecter_set_edit_mode(scroll_selecter, 0) == 0);
    assert(tinyui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 0);
    assert(ld_scroll_selecter->isEdit == false);

    tinyui_app_destroy(app);
}

static void test_scroll_selecter_final_visual_and_edit_contract_is_release_ready(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    int is_edit = -1;

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_release_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_release_ready");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_SCROLL_SELECTER);
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(tinyui_scroll_selecter_set_selected_index(scroll_selecter, 2) == 0);
    assert(tinyui_scroll_selecter_get_selected_index(scroll_selecter) == 2);
    assert(backend->value == 2);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 2);

    assert(tinyui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert(tinyui_scroll_selecter_set_edit_mode(scroll_selecter, 0) == 0);
    assert(tinyui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 0);
    assert(ld_scroll_selecter->isEdit == false);
    assert(tinyui_scroll_selecter_set_edit_mode(scroll_selecter, 1) == 0);
    assert(tinyui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert(ld_scroll_selecter->isEdit == true);

    tinyui_app_destroy(app);
}

static void test_scroll_selecter_native_style_image_speed_and_select_text_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask_tile = {0};
    arm_2d_tile_t indicator_tile = {0};
    arm_2d_tile_t indicator_mask_tile = {0};
    struct tinyui_image_source bg_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask_tile,
    };
    struct tinyui_image_source indicator_source = {
        .img_tile = &indicator_tile,
        .mask_tile = &indicator_mask_tile,
    };
    struct tinyui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = &bg_mask_tile,
    };

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_native_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_native");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(tinyui_scroll_selecter_set_text_color(scroll_selecter, 0x445566U) == 0);
    assert(tinyui_scroll_selecter_set_bg_color(scroll_selecter, 0x112233U) == 0);
    assert(tinyui_scroll_selecter_set_indicator_color(scroll_selecter, 0x778899U) == 0);
    assert(tinyui_scroll_selecter_set_bg_source(scroll_selecter, &bg_source) == 0);
    assert(tinyui_scroll_selecter_set_indicator_source(scroll_selecter, &indicator_source) == 0);
    assert(tinyui_scroll_selecter_set_bg_source(scroll_selecter, &invalid_source) == -1);
    assert(tinyui_scroll_selecter_set_transparent(scroll_selecter, 1) == 0);
    assert(tinyui_scroll_selecter_set_speed(scroll_selecter, 3) == 0);
    assert(tinyui_scroll_selecter_set_select_text(scroll_selecter, "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_get_selected_index(scroll_selecter) == 1);

    assert((unsigned int)ld_scroll_selecter->textColor == encode_rgb_to_ld_color(0x445566U));
    assert((unsigned int)ld_scroll_selecter->bgColor == encode_rgb_to_ld_color(0x112233U));
    assert((unsigned int)ld_scroll_selecter->indicatorColor == encode_rgb_to_ld_color(0x778899U));
    assert(ld_scroll_selecter->ptImgTile == bg_source.img_tile);
    assert(ld_scroll_selecter->ptMaskTile == bg_source.mask_tile);
    assert(ld_scroll_selecter->ptIndicatorImgTile == indicator_source.img_tile);
    assert(ld_scroll_selecter->ptIndicatorMaskTile == indicator_source.mask_tile);
    assert(ld_scroll_selecter->isTransparent == true);
    assert(ld_scroll_selecter->moveOffset == 3);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 1);
    assert(ldScrollSelecterGetSelectText(ld_scroll_selecter) == (uint8_t *)"Bluetooth");

    tinyui_app_destroy(app);
}

static void test_scroll_selecter_selected_text_readback_matches_backend_truth(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_text_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_text");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(tinyui_scroll_selecter_set_selected_index(scroll_selecter, 2) == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(tinyui_scroll_selecter_get_selected_text(scroll_selecter) == ldScrollSelecterGetSelectText(ld_scroll_selecter));
    assert(strcmp(tinyui_scroll_selecter_get_selected_text(scroll_selecter), "Display") == 0);

    tinyui_app_destroy(app);
}

static void test_scroll_selecter_native_api_aliases_match_backend_truth(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    const char *item_ids[] = {"wifi", "bluetooth", "display"};
    const char *texts[] = {"Wi-Fi", "Bluetooth", "Display"};
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t indicator_tile = {0};
    struct tinyui_image_source bg_source = {.img_tile = &bg_tile, .mask_tile = 0};
    struct tinyui_image_source indicator_source = {.img_tile = &indicator_tile, .mask_tile = 0};

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_alias_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_alias");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_set_items(scroll_selecter, item_ids, texts, 3) == 0);
    assert(tinyui_scroll_selecter_set_select_item_num(scroll_selecter, 1) == 0);
    assert(tinyui_scroll_selecter_get_select_item_num(scroll_selecter) == 1);
    assert(tinyui_scroll_selecter_set_background_color(scroll_selecter, 0x010203U) == 0);
    assert(tinyui_scroll_selecter_set_text_color(scroll_selecter, 0x111213U) == 0);
    assert(tinyui_scroll_selecter_set_background_image(scroll_selecter, &bg_source) == 0);
    assert(tinyui_scroll_selecter_set_indicator_image(scroll_selecter, &indicator_source) == 0);
    assert(strcmp(tinyui_scroll_selecter_get_select_text(scroll_selecter), "Bluetooth") == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 1);
    assert(ld_scroll_selecter->itemCount == 3);
    assert(ld_scroll_selecter->ptImgTile == bg_source.img_tile);
    assert(ld_scroll_selecter->ptIndicatorImgTile == indicator_source.img_tile);
    assert(scroll_selecter->widget.bg_color == 0x010203U);
    assert(scroll_selecter->widget.text_color == 0x111213U);
    tinyui_app_destroy(app);
}

static void test_scroll_selecter_init_and_native_base_aliases_round_trip(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_base_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_base");
    assert(scroll_selecter != 0);
    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(tinyui_widget_set_pos(&scroll_selecter->widget, 21, 43) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 21);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 43);
    tinyui_app_destroy(app);
}

static void test_scroll_selecter_sync_selected_index_round_trip(struct tinyui_window *win)
{
    struct tinyui_scroll_selecter *ss = tinyui_scroll_selecter_create(win, "ss_sync");
    const char *ids[] = {"opt_1", "opt_2", "opt_3"};
    const unsigned char *texts[] = {(const unsigned char *)"One", (const unsigned char *)"Two", (const unsigned char *)"Three"};
    int selected;

    assert(ss != 0);
    assert(tinyui_scroll_selecter_set_items(ss, ids, texts, 3) == 0);
    assert(tinyui_scroll_selecter_set_selected_index(ss, 2) == 0);
    selected = tinyui_scroll_selecter_get_selected_index(ss);
    assert(selected == 2);
}

static void test_scroll_selecter_get_selected_text_round_trip(struct tinyui_window *win)
{
    struct tinyui_scroll_selecter *ss = tinyui_scroll_selecter_create(win, "ss_get_text");
    const char *ids[] = {"opt_x"};
    const unsigned char *texts[] = {(const unsigned char *)"OptionX"};

    assert(ss != 0);
    assert(tinyui_scroll_selecter_set_items(ss, ids, texts, 1) == 0);
    assert(tinyui_scroll_selecter_set_selected_index(ss, 0) == 0);

    const char *sel_text = tinyui_scroll_selecter_get_selected_text(ss);
    assert(sel_text != 0);
}

static void test_scroll_selecter_set_items_resets_native_and_public_selection_together(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    const char *replacement_ids[] = {"opt_a", "opt_b"};
    const char *replacement_texts[] = {"Alpha", "Beta"};

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_reset_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_reset");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(tinyui_scroll_selecter_set_selected_index(scroll_selecter, 2) == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 2);

    assert(tinyui_scroll_selecter_set_items(scroll_selecter, replacement_ids, replacement_texts, 2) == 0);
    assert(scroll_selecter->item_count == 2);
    assert(scroll_selecter->selected_index == 0);
    assert(tinyui_scroll_selecter_get_selected_index(scroll_selecter) == 0);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 0);
    assert(tinyui_scroll_selecter_get_selected_text(scroll_selecter) != 0);
    assert(strcmp(tinyui_scroll_selecter_get_selected_text(scroll_selecter), "Alpha") == 0);

    tinyui_app_destroy(app);
}

static void test_scroll_selecter_corrupted_backend_binding_preserves_public_selection_state(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    const char *replacement_ids[] = {"opt_a", "opt_b"};
    const char *replacement_texts[] = {"Alpha", "Beta"};
    enum tinyui_backend_widget_kind saved_kind;

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_corrupt_selection_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_corrupt_selection");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(tinyui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(tinyui_scroll_selecter_set_selected_index(scroll_selecter, 1) == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    saved_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;
    ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, 2);

    assert(tinyui_scroll_selecter_set_items(scroll_selecter, replacement_ids, replacement_texts, 2) == -1);
    assert(scroll_selecter->item_count == 3);
    assert(scroll_selecter->selected_index == 1);
    assert(tinyui_scroll_selecter_get_selected_index(scroll_selecter) == 1);
    assert(tinyui_scroll_selecter_get_selected_text(scroll_selecter) != 0);
    assert(strcmp(tinyui_scroll_selecter_get_selected_text(scroll_selecter), "Bluetooth") == 0);

    backend->kind = saved_kind;
    tinyui_app_destroy(app);
}

static void test_scroll_selecter_corrupted_backend_binding_rejects_edit_mutation(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    enum tinyui_backend_widget_kind saved_kind;
    int is_edit = -1;

    assert(app != 0);
    win = tinyui_window_create(app, "scroll_corrupt_edit_root");
    assert(win != 0);
    scroll_selecter = tinyui_scroll_selecter_create(win, "scroll_corrupt_edit");
    assert(scroll_selecter != 0);
    assert(tinyui_scroll_selecter_set_edit_mode(scroll_selecter, 0) == 0);

    backend = &scroll_selecter->widget;
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);
    assert(ld_scroll_selecter->isEdit == false);

    saved_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_LABEL;

    assert(tinyui_scroll_selecter_set_edit_mode(scroll_selecter, 1) == -1);
    assert(scroll_selecter->edit_mode == 0);
    assert(ld_scroll_selecter->isEdit == false);
    assert(tinyui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 0);

    backend->kind = saved_kind;
    tinyui_app_destroy(app);
}

int main(void)
{
    test_scroll_selecter_internal_seams_are_tinyui_named();
    test_scroll_selecter_selected_item_matches_backend_truth();
    test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct();
    test_scroll_selecter_final_visual_and_edit_contract_is_release_ready();
    test_scroll_selecter_native_style_image_speed_and_select_text_round_trip();
    test_scroll_selecter_selected_text_readback_matches_backend_truth();
    test_scroll_selecter_native_api_aliases_match_backend_truth();
    test_scroll_selecter_init_and_native_base_aliases_round_trip();

    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");
    test_scroll_selecter_legacy_backend_constructor_is_disabled(win);
    test_scroll_selecter_sync_selected_index_round_trip(win);
    test_scroll_selecter_get_selected_text_round_trip(win);
    tinyui_app_destroy(app);
    test_scroll_selecter_set_items_resets_native_and_public_selection_together();
    test_scroll_selecter_corrupted_backend_binding_preserves_public_selection_state();
    test_scroll_selecter_corrupted_backend_binding_rejects_edit_mutation();
    return 0;
}
