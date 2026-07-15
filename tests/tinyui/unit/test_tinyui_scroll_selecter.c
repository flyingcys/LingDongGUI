/*
 * TinyUI scroll_selector unit tests — M3 Task 3.
 * CMake target still uses historical file name scroll_selecter.
 */
#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldScrollSelecter.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"
#include "widgets/scroll_selector.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static ldScrollSelecter_t *ld_of(tinyui_obj_t *o)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)o;
    assert(w && w->ld_widget);
    return (ldScrollSelecter_t *)w->ld_widget;
}

static void test_scroll_selector_init_and_native_base_aliases_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *ss = tinyui_scroll_selector_create(root);
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)ss;
    assert(ss);
    assert(w->kind == TINYUI_BACKEND_WIDGET_SCROLL_SELECTER);
    assert(w->ld_name_id != 0);
    assert(tinyui_obj_set_pos(ss, 3, 4) == TINYUI_OK);
    assert(tinyui_obj_set_visible(ss, 1) == TINYUI_OK);
}

static void test_scroll_selector_native_api_aliases_match_backend_truth(tinyui_obj_t *root)
{
    tinyui_obj_t *ss = tinyui_scroll_selector_create(root);
    ldScrollSelecter_t *ld = ld_of(ss);
    static const char *ids[] = {"0", "1", "2"};
    static const char *texts[] = {"Mon", "Tue", "Wed"};
    assert(tinyui_scroll_selector_set_items(ss, ids, texts, 3) == 0);
    assert(ld->itemCount == 3);
    assert(strcmp((const char *)ld->ppItemStrGroup[0], "Mon") == 0);
    assert(tinyui_scroll_selector_set_select_item_num(ss, 2) == 0);
    assert(tinyui_scroll_selector_get_select_item_num(ss) == 2);
    assert(ldScrollSelecterGetSelectItemNum(ld) == 2);
    assert(strcmp(tinyui_scroll_selector_get_select_text(ss), "Wed") == 0);
    assert(tinyui_scroll_selector_set_text_color(ss, 0x111111) == 0);
    assert(ld->textColor == (ldColor)tinyui_rgb_to_ld_color(0x111111));
    assert(tinyui_scroll_selector_set_background_color(ss, 0x222222) == 0);
    assert(ld->bgColor == (ldColor)tinyui_rgb_to_ld_color(0x222222));
}

static void test_scroll_selector_native_style_image_speed_and_select_text_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *ss = tinyui_scroll_selector_create(root);
    ldScrollSelecter_t *ld = ld_of(ss);
    static const char *ids[] = {"a", "b", "c"};
    static const char *texts[] = {"A", "B", "C"};
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 10, .iHeight = 10}}};
    arm_2d_tile_t mask = {.tRegion = {.tSize = {.iWidth = 10, .iHeight = 10}}};
    tinyui_image_source_t bg, ind;
    memset(&bg, 0, sizeof(bg));
    memset(&ind, 0, sizeof(ind));
    bg.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY; bg.width = 10; bg.height = 10;
    ind.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY; ind.width = 10; ind.height = 10;
    memcpy(bg._image_private, &img, sizeof(img));
    memcpy(bg._mask_private, &mask, sizeof(mask));
    memcpy(ind._image_private, &img, sizeof(img));
    memcpy(ind._mask_private, &mask, sizeof(mask));

    assert(tinyui_scroll_selector_set_items(ss, ids, texts, 3) == 0);
    assert(tinyui_scroll_selector_set_indicator_color(ss, 0xabcdef) == 0);
    assert(ld->indicatorColor == (ldColor)tinyui_rgb_to_ld_color(0xabcdef));
    assert(tinyui_scroll_selector_set_background_image(ss, &bg) == 0);
    assert(ld->ptImgTile == tinyui_image_source_get_image_tile(&bg));
    assert(tinyui_scroll_selector_set_indicator_image(ss, &ind) == 0);
    assert(ld->ptIndicatorImgTile == tinyui_image_source_get_image_tile(&ind));
    assert(tinyui_scroll_selector_set_speed(ss, 5) == 0);
    assert(ld->moveOffset == 5);
    assert(tinyui_scroll_selector_set_select_text(ss, "B") == 0);
    assert(tinyui_scroll_selector_get_select_item_num(ss) == 1);
    assert(tinyui_scroll_selector_set_transparent(ss, 1) == 0);
    assert(ld->isTransparent == true || ld->isTransparent == 1);
}

static void test_scroll_selector_edit_mode_and_navigation_mode_are_distinct(tinyui_obj_t *root)
{
    tinyui_obj_t *ss = tinyui_scroll_selector_create(root);
    ldScrollSelecter_t *ld = ld_of(ss);
    static const char *ids[] = {"1", "2"};
    static const char *texts[] = {"1", "2"};
    int is_edit = -1;
    assert(tinyui_scroll_selector_set_items(ss, ids, texts, 2) == 0);
    assert(tinyui_scroll_selector_set_edit_mode(ss, 1) == 0);
    assert(ld->isEdit == true || ld->isEdit == 1);
    assert(tinyui_scroll_selector_get_edit_mode(ss, &is_edit) == 0);
    assert(is_edit == 1);
    assert(tinyui_scroll_selector_set_edit_mode(ss, 0) == 0);
    assert(ld->isEdit == false || ld->isEdit == 0);
}

static void test_scroll_selector_capacity(tinyui_obj_t *root)
{
    tinyui_obj_t *ss = tinyui_scroll_selector_create(root);
    ldScrollSelecter_t *ld = ld_of(ss);
    static const char *ids[TINYUI_LIST_MAX_ITEMS + 1];
    static const char *texts[TINYUI_LIST_MAX_ITEMS + 1];
    static char idbuf[TINYUI_LIST_MAX_ITEMS + 1][8];
    static char buf[TINYUI_LIST_MAX_ITEMS + 1][8];
    int i;
    for (i = 0; i < TINYUI_LIST_MAX_ITEMS + 1; ++i) {
        snprintf(idbuf[i], sizeof(idbuf[i]), "i%d", i);
        snprintf(buf[i], sizeof(buf[i]), "t%d", i);
        ids[i] = idbuf[i];
        texts[i] = buf[i];
    }
    assert(tinyui_scroll_selector_set_items(ss, ids, texts, TINYUI_LIST_MAX_ITEMS) == 0);
    assert(ld->itemCount == TINYUI_LIST_MAX_ITEMS);
    assert(tinyui_scroll_selector_set_items(ss, ids, texts, TINYUI_LIST_MAX_ITEMS + 1) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(ld->itemCount == TINYUI_LIST_MAX_ITEMS);
    assert(tinyui_scroll_selector_add_item(ss, "overflow", "x") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(ld->itemCount == TINYUI_LIST_MAX_ITEMS);
}

static int g_vc;
static int32_t g_val;

static void on_vc(const tinyui_event_t *e)
{
    assert(e != 0);
    assert(e->code == TINYUI_EVENT_VALUE_CHANGED);
    g_vc += 1;
    g_val = e->data.value;
}

static void ensure_msg_queue(struct tinyui_app *app)
{
    assert(app != 0);
    assert(app->ld_scene != 0);
    if (app->ld_scene->ptMsgQueue == 0) {
        assert(ldMsgInit(&app->ld_scene->ptMsgQueue, 8) == true);
    }
}

static void inject_scroll_value_changed(tinyui_obj_t *ss, int index)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)ss;
    struct tinyui_app *app;

    assert(w != 0);
    assert(w->ld_widget != 0);
    assert(w->owner != 0);
    app = w->owner;
    ensure_msg_queue(app);
    assert(ldMsgEmit(app->ld_scene->ptMsgQueue,
                     w->ld_widget,
                     SIGNAL_VALUE_CHANGED,
                     (uint64_t)(uint32_t)index) == true);
    ldMsgProcess(app->ld_scene);
}

static void test_scroll_selector_value_changed_unified_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *ss = tinyui_scroll_selector_create(root);
    ldScrollSelecter_t *ld = ld_of(ss);
    tinyui_event_handle_t h = 0;
    static const char *ids[] = {"0", "1", "2"};
    static const char *texts[] = {"Mon", "Tue", "Wed"};

    g_vc = 0;
    g_val = -1;
    assert(tinyui_scroll_selector_set_items(ss, ids, texts, 3) == 0);
    assert(tinyui_obj_add_event_cb(ss,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_vc,
                                   0,
                                   &h) == TINYUI_OK);

    assert(tinyui_scroll_selector_set_selected_index(ss, 1) == 0);
    assert(g_vc == 0);
    assert(ldScrollSelecterGetSelectItemNum(ld) == 1);

    inject_scroll_value_changed(ss, 2);
    assert(tinyui_scroll_selector_get_selected_index(ss) == 2);
    assert(ldScrollSelecterGetSelectItemNum(ld) == 2);
    assert(g_vc == 1);
    assert(g_val == 2);

    inject_scroll_value_changed(ss, 2);
    assert(g_vc == 1);
}

int main(void)
{
    tinyui_obj_t *root;
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root);
    test_scroll_selector_init_and_native_base_aliases_round_trip(root);
    test_scroll_selector_native_api_aliases_match_backend_truth(root);
    test_scroll_selector_native_style_image_speed_and_select_text_round_trip(root);
    test_scroll_selector_edit_mode_and_navigation_mode_are_distinct(root);
    test_scroll_selector_capacity(root);
    test_scroll_selector_value_changed_unified_pool(root);
    tinyui_deinit();
    return 0;
}
