/*
 * TinyUI radial_menu unit tests — M3 Task 3.
 */
#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldRadialMenu.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"
#include "widgets/radial_menu.h"
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

static ldRadialMenu_t *ld_of(tinyui_obj_t *o)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)o;
    assert(w && w->ld_widget);
    return (ldRadialMenu_t *)w->ld_widget;
}

static void test_radial_menu_init_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)rm;
    assert(rm);
    assert(w->kind == TINYUI_BACKEND_WIDGET_RADIAL_MENU);
}

static void test_radial_menu_add_item_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    ldRadialMenu_t *ld = ld_of(rm);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 20, .iHeight = 20}}};
    arm_2d_tile_t mask = {.tRegion = {.tSize = {.iWidth = 20, .iHeight = 20}}};
    tinyui_image_source_t src = make_src(&img, &mask);
    assert(tinyui_radial_menu_add_item_with_source(rm, "i0", &src) == 0);
    assert(ld->ptItemInfoList != 0);
}

static void test_radial_menu_set_default_item_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    ldRadialMenu_t *ld = ld_of(rm);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 16, .iHeight = 16}}};
    arm_2d_tile_t mask = img;
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;
    for (i = 0; i < 3; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "r%d", i);
        assert(tinyui_radial_menu_add_item_with_source(rm, id, &src) == 0);
    }
    assert(tinyui_radial_menu_set_default_item(rm, 1) == 0);
    assert(ld->selectItem == 1 || ld->targetItem == 1 || tinyui_radial_menu_get_selected_index(rm) == 1);
}

static void test_radial_menu_set_click_item_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 16, .iHeight = 16}}};
    arm_2d_tile_t mask = img;
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;
    for (i = 0; i < 3; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "c%d", i);
        assert(tinyui_radial_menu_add_item_with_source(rm, id, &src) == 0);
    }
    assert(tinyui_radial_menu_set_click_item(rm, 2) == 0);
}

static void test_radial_menu_set_offset_item_native_parity(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 16, .iHeight = 16}}};
    arm_2d_tile_t mask = img;
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;
    for (i = 0; i < 3; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "o%d", i);
        assert(tinyui_radial_menu_add_item_with_source(rm, id, &src) == 0);
    }
    assert(tinyui_radial_menu_offset_item(rm, 1) == 0);
}

static void test_radial_menu_capacity(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 12, .iHeight = 12}}};
    arm_2d_tile_t mask = img;
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;
    for (i = 0; i < 5; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "m%d", i);
        assert(tinyui_radial_menu_add_item_with_source(rm, id, &src) == 0);
    }
    assert(tinyui_radial_menu_add_item_with_source(rm, "x", &src) == -1);
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

static void inject_radial_clicked_item(tinyui_obj_t *rm, int index)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)rm;
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

static void test_radial_menu_value_changed_unified_pool(tinyui_obj_t *root)
{
    tinyui_obj_t *rm = tinyui_radial_menu_create(root);
    tinyui_event_handle_t h = 0;
    arm_2d_tile_t img = {.tRegion = {.tSize = {.iWidth = 12, .iHeight = 12}}};
    arm_2d_tile_t mask = img;
    tinyui_image_source_t src = make_src(&img, &mask);
    int i;

    g_vc = 0;
    g_val = -1;
    for (i = 0; i < 3; ++i) {
        char id[8];
        snprintf(id, sizeof(id), "r%d", i);
        assert(tinyui_radial_menu_add_item_with_source(rm, id, &src) == 0);
    }
    assert(tinyui_obj_add_event_cb(rm,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_vc,
                                   0,
                                   &h) == TINYUI_OK);

    assert(tinyui_radial_menu_set_selected_index(rm, 1) == 0);
    assert(g_vc == 0);

    inject_radial_clicked_item(rm, 2);
    assert(tinyui_radial_menu_get_selected_index(rm) == 2);
    assert(g_vc == 1);
    assert(g_val == 2);

    inject_radial_clicked_item(rm, 2);
    assert(g_vc == 1);
}

int main(void)
{
    tinyui_obj_t *root;
    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root);
    test_radial_menu_init_native_parity(root);
    test_radial_menu_add_item_native_parity(root);
    test_radial_menu_set_default_item_native_parity(root);
    test_radial_menu_set_click_item_native_parity(root);
    test_radial_menu_set_offset_item_native_parity(root);
    test_radial_menu_capacity(root);
    test_radial_menu_value_changed_unified_pool(root);
    tinyui_deinit();
    return 0;
}
