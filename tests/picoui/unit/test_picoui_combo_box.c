#include "picoui/app.h"
#include "picoui/combo_box.h"
#include "picoui/window.h"
#include "../../../src/gui/ldComboBox.h"
#include "../../../src/misc/ldMsg.h"
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

static int combo_selected_count = 0;
static int combo_selected_index = -1;
static void *combo_selected_user_data = 0;
static int native_combo_clicked_count = 0;
static int native_combo_clicked_index = -1;

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static void on_combo_selected(struct picoui_combo_box *combo_box, int index, void *user_data)
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
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    int is_open = -1;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    assert(picoui_combo_box_set_selected_index(combo_box, 0) == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(ldMsgConnect(backend->ld_widget, SIGNAL_CLICKED_ITEM, on_native_combo_clicked_probe) == true);
    native_combo_clicked_count = 0;
    native_combo_clicked_index = -1;

    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue, backend->ld_widget, SIGNAL_PRESS, 0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
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
    assert(picoui_combo_box_get_selected_index(combo_box) == 2);
    assert(((struct picoui_backend_widget *)combo_box->widget.backend_widget)->value == 2);

    ld_combo_box->isExpand = false;
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);
    picoui_app_destroy(app);
}

static void test_combo_box_reuses_selection_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldComboBox_t *ld_combo_box;
    int cookie = 11;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_contract");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    picoui_combo_box_set_on_selected(combo_box, on_combo_selected, &cookie);
    combo_selected_count = 0;
    combo_selected_index = -1;
    combo_selected_user_data = 0;

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(picoui_combo_box_set_selected_index(combo_box, 0) == 0);
    assert(picoui_widget_set_enabled(&combo_box->widget, 0) == 0);
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
    assert(picoui_combo_box_get_selected_index(combo_box) == 0);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 0);
    assert(backend->value == 0);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(backend->dispatch_count == 0);
    picoui_app_destroy(app);
}

static void test_combo_box_final_visual_and_selection_contract_is_release_ready(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    int is_open = -1;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_release_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_release_ready");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);
    assert(picoui_combo_box_set_selected_index(combo_box, 1) == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_COMBO_BOX);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(picoui_combo_box_get_selected_index(combo_box) == 1);
    assert(backend->value == 1);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 1);
    assert(picoui_combo_box_is_open(combo_box, &is_open) == 0);
    assert(is_open == 0);

    picoui_app_destroy(app);
}

static void test_combo_box_native_color_item_max_and_dropdown_image_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    arm_2d_tile_t dropdown_tile = {0};
    arm_2d_tile_t dropdown_mask_tile = {0};
    struct picoui_image_source dropdown_source = {
        .img_tile = &dropdown_tile,
        .mask_tile = &dropdown_mask_tile,
    };
    struct picoui_image_source invalid_source = {
        .img_tile = 0,
        .mask_tile = &dropdown_mask_tile,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_native_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_native");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(picoui_combo_box_set_text_color(combo_box, 0x445566U) == 0);
    assert(picoui_combo_box_set_bg_color(combo_box, 0x112233U) == 0);
    assert(picoui_combo_box_set_frame_color(combo_box, 0x778899U) == 0);
    assert(picoui_combo_box_set_select_color(combo_box, 0xAABBCCU) == 0);
    assert(picoui_combo_box_set_item_max(combo_box, 5) == 0);
    assert(picoui_combo_box_set_dropdown_source(combo_box, &dropdown_source) == 0);
    assert(picoui_combo_box_set_dropdown_source(combo_box, &invalid_source) == -1);

    assert((unsigned int)ld_combo_box->textColor == encode_rgb_to_ld_color(0x445566U));
    assert((unsigned int)ld_combo_box->bgColor == encode_rgb_to_ld_color(0x112233U));
    assert((unsigned int)ld_combo_box->frameColor == encode_rgb_to_ld_color(0x778899U));
    assert((unsigned int)ld_combo_box->selectColor == encode_rgb_to_ld_color(0xAABBCCU));
    assert(ld_combo_box->itemMax == 5);
    assert(ld_combo_box->ptDropdownImgTile == dropdown_source.img_tile);
    assert(ld_combo_box->ptDropdownMaskTile == dropdown_source.mask_tile);

    picoui_app_destroy(app);
}

static void test_combo_box_native_item_text_readback_matches_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_text_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_text");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(picoui_combo_box_get_text(combo_box, 0) == ldComboBoxGetText(ld_combo_box, 0));
    assert(picoui_combo_box_get_text(combo_box, 1) == ldComboBoxGetText(ld_combo_box, 1));
    assert(picoui_combo_box_get_text(combo_box, 2) == ldComboBoxGetText(ld_combo_box, 2));
    assert(picoui_combo_box_get_text(combo_box, 3) == 0);

    picoui_app_destroy(app);
}

static void test_combo_box_native_api_aliases_match_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;
    const char *item_ids[] = {"wifi", "bluetooth", "display"};
    const char *texts[] = {"Wi-Fi", "Bluetooth", "Display"};
    arm_2d_tile_t dropdown_tile = {0};
    struct picoui_image_source dropdown_source = {
        .img_tile = &dropdown_tile,
        .mask_tile = 0,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_alias_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_alias");
    assert(combo_box != 0);
    assert(picoui_combo_box_set_static_items(combo_box, item_ids, texts, 3) == 0);
    assert(picoui_combo_box_set_select_item(combo_box, 1) == 0);
    assert(picoui_combo_box_get_select_item(combo_box) == 1);
    assert(picoui_combo_box_set_background_color(combo_box, 0x010203U) == 0);
    assert(picoui_combo_box_set_text_color(combo_box, 0x111213U) == 0);
    assert(picoui_combo_box_set_frame_color(combo_box, 0x212223U) == 0);
    assert(picoui_combo_box_set_dropdown_image(combo_box, &dropdown_source) == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);
    assert(ldComboBoxGetSelectItem(ld_combo_box) == 1);
    assert(ld_combo_box->itemCount == 3);
    assert(ld_combo_box->ptDropdownImgTile == dropdown_source.img_tile);
    assert(combo_box->widget.bg_color == 0x010203U);
    assert(combo_box->widget.text_color == 0x111213U);
    assert(combo_box->widget.border_color == 0x212223U);
    picoui_app_destroy(app);
}

static void test_combo_box_shared_base_aliases_round_trip(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_base_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_base");
    assert(combo_box != 0);
    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);

    assert(picoui_widget_set_pos(&combo_box->widget, 12, 34) == 0);
    assert(picoui_widget_set_visible(&combo_box->widget, 0) == 0);
    assert(picoui_widget_set_opacity(&combo_box->widget, 77) == 0);
    assert(picoui_widget_set_selectable(&combo_box->widget, 1) == 0);
    assert(picoui_widget_set_selected(&combo_box->widget, 1) == 0);
    assert(picoui_widget_set_corner(&combo_box->widget, 1) == 0);

    assert(ld_base->tRegion.tLocation.iX == 12);
    assert(ld_base->tRegion.tLocation.iY == 34);
    assert(ld_base->isHidden == false);
    assert(ld_base->opa == 77);
    assert(ld_base->isSelectable == true);
    assert(ld_base->isSelect == true);
    assert(ld_base->isCorner == true);
    picoui_app_destroy(app);
}

static void test_combo_box_uses_native_static_items_contract(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_combo_box *combo_box;
    struct picoui_backend_widget *backend;
    ldComboBox_t *ld_combo_box;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "combo_static_root");
    assert(win != 0);
    combo_box = picoui_combo_box_create(win, "combo_static");
    assert(combo_box != 0);
    assert(picoui_combo_box_add_item(combo_box, "wifi", "Wi-Fi") == 0);
    assert(picoui_combo_box_add_item(combo_box, "bluetooth", "Bluetooth") == 0);
    assert(picoui_combo_box_add_item(combo_box, "display", "Display") == 0);

    backend = (struct picoui_backend_widget *)combo_box->widget.backend_widget;
    assert(backend != 0);
    ld_combo_box = (ldComboBox_t *)backend->ld_widget;
    assert(ld_combo_box != 0);

    assert(ld_combo_box->isStatic == true);
    assert(ld_combo_box->itemCount == 3);
    assert(ld_combo_box->ppItemStrGroup == (uint8_t **)combo_box->backend_item_texts);
    assert(ldComboBoxGetText(ld_combo_box, 0) == (uint8_t *)combo_box->backend_item_texts[0]);
    assert(ldComboBoxGetText(ld_combo_box, 1) == (uint8_t *)combo_box->backend_item_texts[1]);
    assert(ldComboBoxGetText(ld_combo_box, 2) == (uint8_t *)combo_box->backend_item_texts[2]);

    picoui_app_destroy(app);
}

int main(void)
{
    test_combo_box_open_close_and_selected_item_truth();
    test_combo_box_reuses_selection_contract();
    test_combo_box_final_visual_and_selection_contract_is_release_ready();
    test_combo_box_native_color_item_max_and_dropdown_image_round_trip();
    test_combo_box_native_item_text_readback_matches_backend_truth();
    test_combo_box_native_api_aliases_match_backend_truth();
    test_combo_box_shared_base_aliases_round_trip();
    test_combo_box_uses_native_static_items_contract();
    return 0;
}
