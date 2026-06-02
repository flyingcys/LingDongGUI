#include "picoui/app.h"
#include "picoui/scroll_selecter.h"
#include "picoui/window.h"
#include "../../../src/gui/ldScrollSelecter.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static unsigned int encode_rgb_to_ld_color(unsigned int rgb)
{
    unsigned int red = (rgb >> 16) & 0xFFU;
    unsigned int green = (rgb >> 8) & 0xFFU;
    unsigned int blue = rgb & 0xFFU;

    return ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
}

static void assert_scroll_selecter_backend_metadata(const struct picoui_backend_widget *backend,
                                                    unsigned int expected_identity)
{
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SCROLL_SELECTER);
    assert(backend->data_truth_policy == PICOUI_BACKEND_DATA_TRUTH_BACKEND_VALUE);
    assert(backend->data_model_identity != 0);
    if (expected_identity != 0) {
        assert(backend->data_model_identity == expected_identity);
    }
}

static void test_scroll_selecter_selected_item_matches_backend_truth(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    unsigned int data_model_identity;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll");
    assert(scroll_selecter != 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(picoui_scroll_selecter_set_selected_index(scroll_selecter, 0) == 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert_scroll_selecter_backend_metadata(backend, 0);
    data_model_identity = backend->data_model_identity;
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(picoui_scroll_selecter_set_selected_index(scroll_selecter, 1) == 0);
    assert_scroll_selecter_backend_metadata(backend, data_model_identity);
    ldScrollSelecterSetSelectItemNum(ld_scroll_selecter, 2);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 2);
    assert_scroll_selecter_backend_metadata(backend, data_model_identity);
    picoui_app_destroy(app);
}

static void test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    int is_edit = -1;
    unsigned int data_model_identity;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_mode");
    assert(scroll_selecter != 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert_scroll_selecter_backend_metadata(backend, 0);
    data_model_identity = backend->data_model_identity;
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert_scroll_selecter_backend_metadata(backend, data_model_identity);
    assert(picoui_scroll_selecter_set_edit_mode(scroll_selecter, 0) == 0);
    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 0);
    assert(ld_scroll_selecter->isEdit == false);
    assert_scroll_selecter_backend_metadata(backend, data_model_identity);

    picoui_app_destroy(app);
}

static void test_scroll_selecter_final_visual_and_edit_contract_is_release_ready(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    int is_edit = -1;

    assert(app != 0);
    win = picoui_window_create(app, "scroll_release_root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_release_ready");
    assert(scroll_selecter != 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_SCROLL_SELECTER);
    assert(backend->ld_widget != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(picoui_scroll_selecter_set_selected_index(scroll_selecter, 2) == 0);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 2);
    assert(backend->value == 2);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 2);

    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert(picoui_scroll_selecter_set_edit_mode(scroll_selecter, 0) == 0);
    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 0);
    assert(ld_scroll_selecter->isEdit == false);
    assert(picoui_scroll_selecter_set_edit_mode(scroll_selecter, 1) == 0);
    assert(picoui_scroll_selecter_get_edit_mode(scroll_selecter, &is_edit) == 0);
    assert(is_edit == 1);
    assert(ld_scroll_selecter->isEdit == true);

    picoui_app_destroy(app);
}

static void test_scroll_selecter_native_style_image_speed_and_select_text_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t bg_mask_tile = {0};
    arm_2d_tile_t indicator_tile = {0};
    arm_2d_tile_t indicator_mask_tile = {0};
    struct picoui_image_source bg_source = {
        .img_tile = &bg_tile,
        .mask_tile = &bg_mask_tile,
    };
    struct picoui_image_source indicator_source = {
        .img_tile = &indicator_tile,
        .mask_tile = &indicator_mask_tile,
    };
    struct picoui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = &bg_mask_tile,
    };

    assert(app != 0);
    win = picoui_window_create(app, "scroll_native_root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_native");
    assert(scroll_selecter != 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(picoui_scroll_selecter_set_text_color(scroll_selecter, 0x445566U) == 0);
    assert(picoui_scroll_selecter_set_bg_color(scroll_selecter, 0x112233U) == 0);
    assert(picoui_scroll_selecter_set_indicator_color(scroll_selecter, 0x778899U) == 0);
    assert(picoui_scroll_selecter_set_bg_source(scroll_selecter, &bg_source) == 0);
    assert(picoui_scroll_selecter_set_indicator_source(scroll_selecter, &indicator_source) == 0);
    assert(picoui_scroll_selecter_set_bg_source(scroll_selecter, &invalid_source) == -1);
    assert(picoui_scroll_selecter_set_transparent(scroll_selecter, 1) == 0);
    assert(picoui_scroll_selecter_set_speed(scroll_selecter, 3) == 0);
    assert(picoui_scroll_selecter_set_select_text(scroll_selecter, "Bluetooth") == 0);
    assert(picoui_scroll_selecter_get_selected_index(scroll_selecter) == 1);

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

    picoui_app_destroy(app);
}

static void test_scroll_selecter_selected_text_readback_matches_backend_truth(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;

    assert(app != 0);
    win = picoui_window_create(app, "scroll_text_root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_text");
    assert(scroll_selecter != 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "wifi", "Wi-Fi") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "bluetooth", "Bluetooth") == 0);
    assert(picoui_scroll_selecter_add_item(scroll_selecter, "display", "Display") == 0);
    assert(picoui_scroll_selecter_set_selected_index(scroll_selecter, 2) == 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);

    assert(picoui_scroll_selecter_get_selected_text(scroll_selecter) == ldScrollSelecterGetSelectText(ld_scroll_selecter));
    assert(strcmp(picoui_scroll_selecter_get_selected_text(scroll_selecter), "Display") == 0);

    picoui_app_destroy(app);
}

static void test_scroll_selecter_native_api_aliases_match_backend_truth(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldScrollSelecter_t *ld_scroll_selecter;
    const char *item_ids[] = {"wifi", "bluetooth", "display"};
    const char *texts[] = {"Wi-Fi", "Bluetooth", "Display"};
    arm_2d_tile_t bg_tile = {0};
    arm_2d_tile_t indicator_tile = {0};
    struct picoui_image_source bg_source = {.img_tile = &bg_tile, .mask_tile = 0};
    struct picoui_image_source indicator_source = {.img_tile = &indicator_tile, .mask_tile = 0};

    assert(app != 0);
    win = picoui_window_create(app, "scroll_alias_root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_alias");
    assert(scroll_selecter != 0);
    assert(picoui_scroll_selecter_set_items(scroll_selecter, item_ids, texts, 3) == 0);
    assert(picoui_scroll_selecter_set_select_item_num(scroll_selecter, 1) == 0);
    assert(picoui_scroll_selecter_get_select_item_num(scroll_selecter) == 1);
    assert(picoui_scroll_selecter_set_background_color(scroll_selecter, 0x010203U) == 0);
    assert(picoui_scroll_selecter_set_text_color(scroll_selecter, 0x111213U) == 0);
    assert(picoui_scroll_selecter_set_background_image(scroll_selecter, &bg_source) == 0);
    assert(picoui_scroll_selecter_set_indicator_image(scroll_selecter, &indicator_source) == 0);
    assert(strcmp(picoui_scroll_selecter_get_select_text(scroll_selecter), "Bluetooth") == 0);

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_scroll_selecter = (ldScrollSelecter_t *)backend->ld_widget;
    assert(ld_scroll_selecter != 0);
    assert(ldScrollSelecterGetSelectItemNum(ld_scroll_selecter) == 1);
    assert(ld_scroll_selecter->itemCount == 3);
    assert(ld_scroll_selecter->ptImgTile == bg_source.img_tile);
    assert(ld_scroll_selecter->ptIndicatorImgTile == indicator_source.img_tile);
    assert(scroll_selecter->widget.bg_color == 0x010203U);
    assert(scroll_selecter->widget.text_color == 0x111213U);
    picoui_app_destroy(app);
}

static void test_scroll_selecter_init_and_native_base_aliases_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(app != 0);
    win = picoui_window_create(app, "scroll_base_root");
    assert(win != 0);
    scroll_selecter = picoui_scroll_selecter_create(win, "scroll_base");
    assert(scroll_selecter != 0);
    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(picoui_widget_set_pos(&scroll_selecter->widget, 21, 43) == 0);
    assert(ld_base->tRegion.tLocation.iX == 21);
    assert(ld_base->tRegion.tLocation.iY == 43);
    picoui_app_destroy(app);
}

int main(void)
{
    test_scroll_selecter_selected_item_matches_backend_truth();
    test_scroll_selecter_edit_mode_and_navigation_mode_are_distinct();
    test_scroll_selecter_final_visual_and_edit_contract_is_release_ready();
    test_scroll_selecter_native_style_image_speed_and_select_text_round_trip();
    test_scroll_selecter_selected_text_readback_matches_backend_truth();
    test_scroll_selecter_native_api_aliases_match_backend_truth();
    test_scroll_selecter_init_and_native_base_aliases_round_trip();
    return 0;
}
