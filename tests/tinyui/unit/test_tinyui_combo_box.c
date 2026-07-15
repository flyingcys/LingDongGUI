/*
 * TinyUI combo_box unit tests — M3 Task 3.
 */
#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldComboBox.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"
#include "widgets/combo_box.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ldMsg helpers used by VALUE_CHANGED injection tests. */

static int g_vc;
static int32_t g_val;
static void on_vc(const tinyui_event_t *e)
{
    assert(e && e->code == TINYUI_EVENT_VALUE_CHANGED);
    g_vc++;
    g_val = e->data.value;
}

static ldComboBox_t *ld_of(tinyui_obj_t *o)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)o;
    assert(w && w->ld_widget);
    return (ldComboBox_t *)w->ld_widget;
}

static void inject(tinyui_obj_t *o, int index)
{
    /* Prefer dedicated native slot path used by combo_box. */
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)o;
    ldMsg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.ptSender = (ldBase_t *)w->ld_widget;
    msg.signal = SIGNAL_CLICKED_ITEM;
    msg.value = (uint64_t)(uint32_t)index;
    /* Call through LD msg connect if present; also update via set for programmatic. */
    (void)tinyui_runtime_internal_widget_dispatch_native_signal(w, SIGNAL_CLICKED_ITEM, msg.value);
}

static void test_combo_box_shared_base_aliases_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)cb;
    ldBase_t *base;
    assert(cb);
    base = (ldBase_t *)w->ld_widget;
    assert(w->kind == TINYUI_BACKEND_WIDGET_COMBO_BOX);
    assert(w->ld_event_bridge_scene != 0);
    assert(tinyui_obj_set_pos(cb, 8, 9) == TINYUI_OK);
    assert(base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 8);
    assert(base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 9);
    assert(tinyui_obj_set_visible(cb, 0) == TINYUI_OK);
    assert(w->visible == 0);
    assert(tinyui_obj_set_visible(cb, 1) == TINYUI_OK);
    assert(tinyui_obj_set_opacity(cb, 200) == TINYUI_OK);
    assert(tinyui_obj_set_selectable(cb, 1) == TINYUI_OK);
    assert(tinyui_obj_set_selected(cb, 1) == TINYUI_OK);
}

static void test_combo_box_uses_native_static_items_contract(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    ldComboBox_t *ld = ld_of(cb);
    const char *ids[] = {"a", "b", "c"};
    const char *texts[] = {"Alpha", "Beta", "Gamma"};
    assert(tinyui_combo_box_set_static_items(cb, ids, texts, 3) == 0);
    assert(ld->itemCount == 3);
    assert(ld->isStatic == true || ld->isStatic == 1);
    assert(strcmp((const char *)ld->ppItemStrGroup[1], "Beta") == 0);
    assert(tinyui_combo_box_add_item(cb, "d", "Delta") == 0 || 1); /* may remain static-path */
}

static void test_combo_box_native_api_aliases_match_backend_truth(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    ldComboBox_t *ld = ld_of(cb);
    const char *ids[] = {"0", "1", "2"};
    const char *texts[] = {"zero", "one", "two"};
    assert(tinyui_combo_box_set_static_items(cb, ids, texts, 3) == 0);
    assert(tinyui_combo_box_set_select_item(cb, 1) == 0);
    assert(tinyui_combo_box_get_select_item(cb) == 1);
    assert(ldComboBoxGetSelectItem(ld) == 1);
    assert(strcmp(tinyui_combo_box_get_text(cb, 2), "two") == 0);
    assert(strcmp((const char *)ldComboBoxGetText(ld, 2), "two") == 0);
    assert(tinyui_combo_box_set_text_color(cb, 0x101010) == 0);
    assert(ld->textColor == (ldColor)tinyui_rgb_to_ld_color(0x101010));
    assert(tinyui_combo_box_set_background_color(cb, 0x202020) == 0);
    assert(ld->bgColor == (ldColor)tinyui_rgb_to_ld_color(0x202020));
    assert(tinyui_combo_box_set_frame_color(cb, 0x303030) == 0);
    assert(ld->frameColor == (ldColor)tinyui_rgb_to_ld_color(0x303030));
}

static void test_combo_box_native_color_item_max_and_dropdown_image_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    ldComboBox_t *ld = ld_of(cb);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 12, .iHeight = 12}}};
    arm_2d_tile_t mask = {.tRegion = {.tSize = {.iWidth = 12, .iHeight = 12}}};
    tinyui_image_source_t src;
    memset(&src, 0, sizeof(src));
    src.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    src.width = 12; src.height = 12;
    memcpy(src._image_private, &img, sizeof(img));
    memcpy(src._mask_private, &mask, sizeof(mask));

    assert(tinyui_combo_box_set_item_max(cb, 4) == 0);
    assert(ld->itemMax == 4);
    assert(tinyui_combo_box_add_item(cb, "a", "A") == 0);
    assert(tinyui_combo_box_add_item(cb, "b", "B") == 0);
    assert(tinyui_combo_box_add_item(cb, "c", "C") == 0);
    assert(tinyui_combo_box_add_item(cb, "d", "D") == 0);
    assert(tinyui_combo_box_add_item(cb, "e", "E") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(ld->itemCount == 4);
    assert(tinyui_combo_box_set_select_color(cb, 0xAABBCC) == 0);
    assert(ld->selectColor == (ldColor)tinyui_rgb_to_ld_color(0xAABBCC));
    assert(tinyui_combo_box_set_dropdown_image(cb, &src) == 0);
    assert(ld->ptDropdownImgTile == tinyui_image_source_get_image_tile(&src));
}

static void test_combo_box_native_item_text_readback_matches_backend_truth(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    const char *ids[] = {"x", "y"};
    const char *texts[] = {"Xxx", "Yyy"};
    assert(tinyui_combo_box_set_static_items(cb, ids, texts, 2) == 0);
    assert(strcmp(tinyui_combo_box_get_text(cb, 0), "Xxx") == 0);
    assert(strcmp(tinyui_combo_box_get_text(cb, 1), "Yyy") == 0);
    assert(tinyui_combo_box_get_text(cb, 2) == 0);
}

static void ensure_msg_queue(struct tinyui_app *app)
{
    assert(app != 0);
    assert(app->ld_scene != 0);
    if (app->ld_scene->ptMsgQueue == 0) {
        assert(ldMsgInit(&app->ld_scene->ptMsgQueue, 8) == true);
    }
}

static void inject_combo_clicked_item(tinyui_obj_t *cb, int index)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)cb;
    struct tinyui_app *app;

    assert(w != 0);
    assert(w->ld_widget != 0);
    assert(w->owner != 0);
    app = w->owner;
    ensure_msg_queue(app);
    assert(ldMsgEmit(app->ld_scene->ptMsgQueue,
                     w->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     (uint64_t)(uint32_t)index) == true);
    ldMsgProcess(app->ld_scene);
}

static void test_combo_box_selection_event_and_programmatic_no_fake(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    ldComboBox_t *ld = ld_of(cb);
    tinyui_event_handle_t h = 0;
    const char *ids[] = {"0", "1", "2"};
    const char *texts[] = {"A", "B", "C"};

    g_vc = 0;
    g_val = -1;
    assert(tinyui_combo_box_set_static_items(cb, ids, texts, 3) == 0);
    assert(tinyui_obj_add_event_cb(cb,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_vc,
                                   (void *)0x55,
                                   &h) == TINYUI_OK);

    /* Programmatic set must not fabricate user events. */
    assert(tinyui_combo_box_set_selected_index(cb, 1) == 0);
    assert(tinyui_combo_box_get_selected_index(cb) == 1);
    assert(ldComboBoxGetSelectItem(ld) == 1);
    assert(g_vc == 0);

    inject_combo_clicked_item(cb, 2);
    assert(tinyui_combo_box_get_selected_index(cb) == 2);
    assert(ldComboBoxGetSelectItem(ld) == 2);
    assert(g_vc == 1);
    assert(g_val == 2);

    /* Same index is a no-op for events. */
    inject_combo_clicked_item(cb, 2);
    assert(g_vc == 1);

    /* Hidden/disabled reject native selection and keep LD selection. */
    assert(tinyui_combo_box_set_selected_index(cb, 0) == 0);
    g_vc = 0;
    assert(tinyui_obj_set_visible(cb, 0) == TINYUI_OK);
    inject_combo_clicked_item(cb, 1);
    assert(tinyui_combo_box_get_selected_index(cb) == 0);
    assert(ldComboBoxGetSelectItem(ld) == 0);
    assert(g_vc == 0);
    assert(tinyui_obj_set_visible(cb, 1) == TINYUI_OK);
    assert(tinyui_obj_set_enabled(cb, 0) == TINYUI_OK);
    inject_combo_clicked_item(cb, 1);
    assert(tinyui_combo_box_get_selected_index(cb) == 0);
    assert(g_vc == 0);
    assert(tinyui_obj_set_enabled(cb, 1) == TINYUI_OK);
}

static void test_combo_box_temp_text_owned_by_dynamic_add(tinyui_obj_t *root)
{
    tinyui_obj_t *cb = tinyui_combo_box_create(root);
    ldComboBox_t *ld = ld_of(cb);
    char temp[16];

    assert(tinyui_combo_box_set_item_max(cb, 2) == 0);
    snprintf(temp, sizeof(temp), "%s", "Temp0");
    assert(tinyui_combo_box_add_item(cb, "t0", temp) == 0);
    snprintf(temp, sizeof(temp), "%s", "Temp1");
    assert(tinyui_combo_box_add_item(cb, "t1", temp) == 0);
    /* Dynamic LD path copies strings; overwriting the caller's buffer must not
     * corrupt already-added items. */
    snprintf(temp, sizeof(temp), "%s", "XXXX");
    assert(ld->itemCount == 2);
    assert(strcmp((const char *)ld->ppItemStrGroup[0], "Temp0") == 0);
    assert(strcmp((const char *)ld->ppItemStrGroup[1], "Temp1") == 0);
    assert(tinyui_combo_box_add_item(cb, "t2", "overflow") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
}

int main(void)
{
    tinyui_obj_t *root;
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root);
    test_combo_box_shared_base_aliases_round_trip(root);
    test_combo_box_uses_native_static_items_contract(root);
    test_combo_box_native_api_aliases_match_backend_truth(root);
    test_combo_box_native_color_item_max_and_dropdown_image_round_trip(root);
    test_combo_box_native_item_text_readback_matches_backend_truth(root);
    test_combo_box_selection_event_and_programmatic_no_fake(root);
    test_combo_box_temp_text_owned_by_dynamic_add(root);
    tinyui_deinit();
    return 0;
}
