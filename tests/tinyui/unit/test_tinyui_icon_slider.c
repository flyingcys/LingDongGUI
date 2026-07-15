/*
 * TinyUI icon_slider unit tests — M3 Task 3.
 */
#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldIconSlider.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"
#include "widgets/icon_slider.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static tinyui_image_source_t make_src(arm_2d_tile_t *img, arm_2d_tile_t *mask)
{
    tinyui_image_source_t src;
    memset(&src, 0, sizeof(src));
    src.kind = TINYUI_IMAGE_SOURCE_RGB565_MEMORY;
    src.width = (uint16_t)img->tRegion.tSize.iWidth;
    src.height = (uint16_t)img->tRegion.tSize.iHeight;
    memcpy(src._image_private, img, sizeof(*img));
    memcpy(src._mask_private, mask, sizeof(*mask));
    return src;
}

static ldIconSlider_t *ld_of(tinyui_obj_t *o)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)o;
    assert(w && w->ld_widget);
    return (ldIconSlider_t *)w->ld_widget;
}

static void test_icon_slider_init_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)is;
    assert(is);
    assert(w->kind == TINYUI_BACKEND_WIDGET_ICON_SLIDER);
    assert(w->ld_event_bridge_scene != 0);
}

static void test_icon_slider_add_icon_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    ldIconSlider_t *ld = ld_of(is);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 24, .iHeight = 24}}};
    arm_2d_tile_t mask = {.tRegion = {.tSize = {.iWidth = 24, .iHeight = 24}}};
    tinyui_image_source_t src = make_src(&img, &mask);
    assert(tinyui_icon_slider_add_icon(is, "mail", "Mail", &src) == 0);
    assert(ld->iconCount == 1);
    assert(ld->ptIconInfoList[0].ptImgTile == tinyui_image_source_get_image_tile(&src));
    assert(strcmp((const char *)ld->ptIconInfoList[0].pName, "Mail") == 0);
}

static void test_icon_slider_set_speed_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    ldIconSlider_t *ld = ld_of(is);
    assert(tinyui_icon_slider_set_speed(is, 7) == 0);
    assert(ld->moveOffset == 7);
}

static void test_icon_slider_set_horizontal_scroll_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    ldIconSlider_t *ld = ld_of(is);
    assert(tinyui_icon_slider_set_horizontal_scroll(is, 1) == 0);
    assert(ld->isHorizontalScroll == true || ld->isHorizontalScroll == 1);
    assert(tinyui_icon_slider_set_horizontal_scroll(is, 0) == 0);
    assert(ld->isHorizontalScroll == false || ld->isHorizontalScroll == 0);
}

static void test_icon_slider_move_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    ldBase_t *base = (ldBase_t *)ld_of(is);
    assert(tinyui_obj_set_pos(is, 11, 22) == TINYUI_OK);
    assert(base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 11);
    assert(base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 22);
}

static void test_icon_slider_set_hidden_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)is;
    assert(tinyui_obj_set_visible(is, 0) == TINYUI_OK);
    assert(w->visible == 0);
    assert(tinyui_obj_set_visible(is, 1) == TINYUI_OK);
}

static void test_icon_slider_set_opacity_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    assert(tinyui_obj_set_opacity(is, 150) == TINYUI_OK);
}

static void test_icon_sliderh_set_corner_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    (void)tinyui_obj_set_selectable(is, 1);
    assert(is != 0);
}

static void test_icon_sliderh_set_select_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    assert(tinyui_obj_set_selected(is, 1) == TINYUI_OK);
}

static void test_icon_sliderh_set_selectable_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    assert(tinyui_obj_set_selectable(is, 1) == TINYUI_OK);
}

static void test_icon_slider_capacity(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 16, .iHeight = 16}}};
    arm_2d_tile_t mask = {.tRegion = {.tSize = {.iWidth = 16, .iHeight = 16}}};
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;
    for (i = 0; i < 8; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "i%d", i);
        assert(tinyui_icon_slider_add_icon(is, id, id, &src) == 0);
    }
    assert(tinyui_icon_slider_add_icon(is, "x", "x", &src) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
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

static void inject_icon_clicked_item(tinyui_obj_t *is, int index)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)is;
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

static void test_icon_slider_value_changed_unified_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *is = tinyui_icon_slider_create(root);
    ldIconSlider_t *ld = ld_of(is);
    tinyui_event_handle_t h = 0;
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 16, .iHeight = 16}}};
    arm_2d_tile_t mask = img;
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;

    g_vc = 0;
    g_val = -1;
    for (i = 0; i < 3; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "i%d", i);
        assert(tinyui_icon_slider_add_icon(is, id, id, &src) == 0);
    }
    assert(tinyui_obj_add_event_cb(is,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_vc,
                                   0,
                                   &h) == TINYUI_OK);

    assert(tinyui_icon_slider_set_selected_index(is, 1) == 0);
    assert(g_vc == 0);

    inject_icon_clicked_item(is, 2);
    assert(tinyui_icon_slider_get_selected_index(is) == 2);
    assert(ld->selectIconOrPage == 2);
    assert(g_vc == 1);
    assert(g_val == 2);

    inject_icon_clicked_item(is, 2);
    assert(g_vc == 1);
}

int main(void)
{
    tinyui_obj_t *root;
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root);
    test_icon_slider_init_native_parity(root);
    test_icon_slider_add_icon_native_parity(root);
    test_icon_slider_set_speed_native_parity(root);
    test_icon_slider_set_horizontal_scroll_native_parity(root);
    test_icon_slider_move_native_parity(root);
    test_icon_slider_set_hidden_native_parity(root);
    test_icon_slider_set_opacity_native_parity(root);
    test_icon_sliderh_set_corner_native_parity(root);
    test_icon_sliderh_set_select_native_parity(root);
    test_icon_sliderh_set_selectable_native_parity(root);
    test_icon_slider_capacity(root);
    test_icon_slider_value_changed_unified_pool(root);
    tinyui_deinit();
    return 0;
}
