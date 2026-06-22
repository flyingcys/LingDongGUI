#include "app.h"
#include "radial_menu.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldRadialMenu.h"
#include "internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const char *test_repo_root_from_file(const char *file_path)
{
    const char *unit_segment = strstr(file_path, "/tests/tinyui/unit/");

    assert(unit_segment != 0);
    static char repo_root[1024];
    size_t root_len = (size_t)(unit_segment - file_path);

    assert(root_len < sizeof(repo_root));
    memcpy(repo_root, file_path, root_len);
    repo_root[root_len] = '\0';
    return repo_root;
}

static bool test_source_contains_definition(const char *relative_path, const char *symbol)
{
    char source_path[1400];
    char line[2048];
    FILE *fp;

    snprintf(source_path,
             sizeof(source_path),
             "%s/%s",
             test_repo_root_from_file(__FILE__),
             relative_path);
    fp = fopen(source_path, "r");
    assert(fp != 0);
    while (fgets(line, sizeof(line), fp) != 0) {
        if (strstr(line, symbol) != 0) {
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

static void assert_source_contains_definition(const char *relative_path, const char *symbol)
{
    assert(test_source_contains_definition(relative_path, symbol));
}

static void assert_source_lacks_definition(const char *relative_path, const char *symbol)
{
    assert(!test_source_contains_definition(relative_path, symbol));
}

static void test_radial_menu_internal_seam_names_are_tinyui_local(void)
{
    const char *widget_source = "tinyui/src/widgets/radial_menu.c";

    /* Phase C2: backend_* functions eliminated, replaced by inlined ld calls */
    assert_source_lacks_definition(widget_source, "static ldRadialMenu_t *tinyui_radial_menu_get_ld(");
    assert_source_lacks_definition(widget_source, "static int tinyui_radial_menu_backend_");
    assert_source_lacks_definition(widget_source, "static int tinyui_radial_menu_bind_host(");
    assert_source_lacks_definition(widget_source, "tinyui_radial_menu_create_with_backend_config");

    /* Remaining internal helpers */
    assert_source_contains_definition(widget_source, "static bool tinyui_radial_menu_native_slot(");
    assert_source_contains_definition(widget_source, "static int tinyui_radial_menu_props_are_valid(");
    assert_source_contains_definition(widget_source, "static struct tinyui_radial_menu *tinyui_radial_menu_create_internal(");

    /* C2 depose/rollback seam */
    assert_source_contains_definition(widget_source, "static void tinyui_radial_menu_ld_depose_cb(");
    assert_source_contains_definition(widget_source, "static void tinyui_radial_menu_rollback(");
}

static void radial_menu_on_selected(struct tinyui_radial_menu *radial_menu, int index, void *user_data)
{
    (void)radial_menu;
    (void)index;
    (void)user_data;
}

static void test_radial_menu_navigation_and_selection_follow_backend_truth(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_create((struct tinyui_widget *)win, "radial_menu");
    assert(radial_menu != 0);
    assert(tinyui_radial_menu_add_item(radial_menu, "weather") == 0);
    assert(tinyui_radial_menu_add_item(radial_menu, "note") == 0);
    assert(tinyui_radial_menu_add_item(radial_menu, "book") == 0);
    assert(tinyui_radial_menu_add_item(radial_menu, "chart") == 0);
    assert(tinyui_radial_menu_set_selected_index(radial_menu, 1) == 0);
    assert(tinyui_radial_menu_get_selected_index(radial_menu) == 1);
    assert(tinyui_radial_menu_offset_selection(radial_menu, 1) == 0);
    assert(tinyui_radial_menu_get_selected_index(radial_menu) == 2);

    tinyui_radial_menu_set_on_selected(radial_menu, radial_menu_on_selected, radial_menu);
    tinyui_app_destroy(app);
}

static void test_radial_menu_rejects_items_beyond_native_capacity(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;
    int index;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_create((struct tinyui_widget *)win, "radial_menu");
    assert(radial_menu != 0);
    for (index = 0; index < 5; ++index) {
        char id[16];

        snprintf(id, sizeof(id), "item_%d", index);
        assert(tinyui_radial_menu_add_item(radial_menu, id) == 0);
    }

    assert(tinyui_radial_menu_add_item(radial_menu, "overflow") == -1);
    assert(tinyui_radial_menu_set_selected_index(radial_menu, 4) == 0);
    assert(tinyui_radial_menu_set_selected_index(radial_menu, 5) == -1);

    tinyui_app_destroy(app);
}

static void test_radial_menu_create_with_default_index_defers_selection_until_items_exist(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;
    const struct tinyui_radial_menu_props props = {
        .id = "radial_menu",
        .default_index = 0,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_create_with_props((struct tinyui_widget *)win, &props);
    assert(radial_menu != 0);
    assert(tinyui_radial_menu_get_selected_index(radial_menu) == 0);
    assert(tinyui_radial_menu_add_item(radial_menu, "weather") == 0);
    assert(tinyui_radial_menu_get_selected_index(radial_menu) == 0);

    tinyui_app_destroy(app);
}

static void test_radial_menu_create_builds_direct_backend_mapping(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;
    ldRadialMenu_t *ld_radial_menu;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "radial_menu_direct_root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_create((struct tinyui_widget *)win, "radial_menu_direct");
    assert(radial_menu != 0);

    backend = &radial_menu->widget;
    parent_backend = &win->widget;
    assert(backend->ld_widget != 0);
    assert(parent_backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_RADIAL_MENU);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_radial_menu = (ldRadialMenu_t *)backend->ld_widget;
    assert(ld_radial_menu != 0);
    assert(((ldBase_t *)ld_radial_menu)->pInfo == backend);

    tinyui_app_destroy(app);
}

static void test_radial_menu_create_with_props_pushes_backend_geometry(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;
    struct tinyui_widget *backend;
    ldRadialMenu_t *ld_radial_menu;
    const struct tinyui_radial_menu_props props = {
        .id = "radial_menu",
        .width = 210,
        .height = 132,
        .x_axis = 96,
        .y_axis = 72,
        .item_max = 4,
        .default_index = 0,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_create_with_props((struct tinyui_widget *)win, &props);
    assert(radial_menu != 0);
    assert(tinyui_radial_menu_add_item(radial_menu, "weather") == 0);

    backend = &radial_menu->widget;
    assert(backend->ld_widget != 0);
    ld_radial_menu = (ldRadialMenu_t *)backend->ld_widget;
    assert(ld_radial_menu != 0);

    assert(ld_radial_menu->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == props.width);
    assert(ld_radial_menu->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == props.height);
    assert(ld_radial_menu->xAxis == props.x_axis);
    assert(ld_radial_menu->yAxis == props.y_axis);
    assert(ld_radial_menu->itemMax == props.item_max);
    assert(ld_radial_menu->selectItem == props.default_index);

    tinyui_app_destroy(app);
}

static void test_radial_menu_native_click_default_offset_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;
    struct tinyui_widget *backend;
    ldRadialMenu_t *ld_radial_menu;
    arm_2d_tile_t item_img = {
        .tRegion = {
            .tSize = { .iWidth = 20, .iHeight = 20 },
        },
    };
    arm_2d_tile_t item_mask = {
        .tRegion = {
            .tSize = { .iWidth = 20, .iHeight = 20 },
        },
    };
    struct tinyui_image_source item_source = {
        .img_tile = &item_img,
        .mask_tile = &item_mask,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_create((struct tinyui_widget *)win, "radial_menu_native");
    assert(radial_menu != 0);
    backend = &radial_menu->widget;
    assert(backend->ld_widget != 0);
    ld_radial_menu = (ldRadialMenu_t *)backend->ld_widget;
    assert(ld_radial_menu != 0);

    assert(tinyui_radial_menu_add_item_with_source(radial_menu, "weather", &item_source) == 0);
    assert(tinyui_radial_menu_add_item_with_source(radial_menu, "mail", &item_source) == 0);
    assert(tinyui_radial_menu_add_item_with_source(radial_menu, "book", &item_source) == 0);

    assert(tinyui_radial_menu_set_default_item(radial_menu, 1) == 0);
    assert(tinyui_radial_menu_click_item(radial_menu, 2) == 0);
    assert(tinyui_radial_menu_offset_item(radial_menu, -1) == 0);

    assert(ld_radial_menu->use_as__ldBase_t.itemCount == 3);
    assert(ld_radial_menu->ptItemInfoList[0].ptImgTile == &item_img);
    assert(ld_radial_menu->ptItemInfoList[0].ptMaskTile == &item_mask);
    assert(ld_radial_menu->selectItem == 1);
    assert(ld_radial_menu->targetItem == 2);
    assert(ld_radial_menu->_itemOffset == -1);

    assert(tinyui_radial_menu_add_item_with_source(0, "x", &item_source) == -1);
    assert(tinyui_radial_menu_set_default_item(0, 0) == -1);
    assert(tinyui_radial_menu_click_item(0, 0) == -1);
    assert(tinyui_radial_menu_offset_item(0, 1) == -1);

    tinyui_app_destroy(app);
}

static void test_radial_menu_init_and_alias_round_trip(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;
    struct tinyui_radial_menu *radial_menu;
    struct tinyui_widget *backend;
    ldRadialMenu_t *ld_radial_menu;
    arm_2d_tile_t item_img = {
        .tRegion = {
            .tSize = { .iWidth = 18, .iHeight = 18 },
        },
    };
    arm_2d_tile_t item_mask = {
        .tRegion = {
            .tSize = { .iWidth = 18, .iHeight = 18 },
        },
    };
    struct tinyui_image_source item_source = {
        .img_tile = &item_img,
        .mask_tile = &item_mask,
    };

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    radial_menu = tinyui_radial_menu_init((struct tinyui_widget *)win, "radial_menu_alias");
    assert(radial_menu != 0);
    backend = &radial_menu->widget;
    assert(backend->ld_widget != 0);
    ld_radial_menu = (ldRadialMenu_t *)backend->ld_widget;
    assert(ld_radial_menu != 0);

    assert(tinyui_radial_menu_add_item_with_image(radial_menu, "weather", &item_source) == 0);
    assert(tinyui_radial_menu_add_item_with_image(radial_menu, "mail", &item_source) == 0);
    assert(tinyui_radial_menu_add_item_with_image(radial_menu, "book", &item_source) == 0);
    assert(tinyui_radial_menu_set_default_item(radial_menu, 1) == 0);
    assert(tinyui_radial_menu_set_click_item(radial_menu, 2) == 0);
    assert(tinyui_radial_menu_offset_item(radial_menu, -1) == 0);

    assert(ld_radial_menu->use_as__ldBase_t.itemCount == 3);
    assert(ld_radial_menu->ptItemInfoList[0].ptImgTile == &item_img);
    assert(ld_radial_menu->ptItemInfoList[0].ptMaskTile == &item_mask);
    assert(ld_radial_menu->selectItem == 1);
    assert(ld_radial_menu->targetItem == 2);
    assert(ld_radial_menu->_itemOffset == -1);

    tinyui_app_destroy(app);
}

static void test_radial_menu_rejects_null_args(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win;

    app = tinyui_app_create();
    assert(app != 0);
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    assert(tinyui_radial_menu_create(0, "id") == 0);
    assert(tinyui_radial_menu_create((struct tinyui_widget *)win, 0) == 0);
    assert(tinyui_radial_menu_add_item(0, "item") == -1);
    assert(tinyui_radial_menu_set_selected_index(0, 0) == -1);
    assert(tinyui_radial_menu_get_selected_index(0) == -1);
    assert(tinyui_radial_menu_offset_selection(0, 1) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_radial_menu_internal_seam_names_are_tinyui_local();
    test_radial_menu_navigation_and_selection_follow_backend_truth();
    test_radial_menu_rejects_items_beyond_native_capacity();
    test_radial_menu_create_with_default_index_defers_selection_until_items_exist();
    test_radial_menu_create_builds_direct_backend_mapping();
    test_radial_menu_create_with_props_pushes_backend_geometry();
    test_radial_menu_native_click_default_offset_round_trip();
    test_radial_menu_init_and_alias_round_trip();
    test_radial_menu_rejects_null_args();
    return 0;
}
