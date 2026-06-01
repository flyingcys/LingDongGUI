#include "picoui/app.h"
#include "picoui/radial_menu.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldRadialMenu.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>

static void radial_menu_on_selected(struct picoui_radial_menu *radial_menu, int index, void *user_data)
{
    (void)radial_menu;
    (void)index;
    (void)user_data;
}

static void test_radial_menu_navigation_and_selection_follow_backend_truth(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_radial_menu *radial_menu;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    radial_menu = picoui_radial_menu_create((struct picoui_widget *)win, "radial_menu");
    assert(radial_menu != 0);
    assert(picoui_radial_menu_add_item(radial_menu, "weather") == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "note") == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "book") == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "chart") == 0);
    assert(picoui_radial_menu_set_selected_index(radial_menu, 1) == 0);
    assert(picoui_radial_menu_get_selected_index(radial_menu) == 1);
    assert(picoui_radial_menu_offset_selection(radial_menu, 1) == 0);
    assert(picoui_radial_menu_get_selected_index(radial_menu) == 2);

    picoui_radial_menu_set_on_selected(radial_menu, radial_menu_on_selected, radial_menu);
    picoui_app_destroy(app);
}

static void test_radial_menu_rejects_items_beyond_native_capacity(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_radial_menu *radial_menu;
    int index;

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    radial_menu = picoui_radial_menu_create((struct picoui_widget *)win, "radial_menu");
    assert(radial_menu != 0);
    for (index = 0; index < 5; ++index) {
        char id[16];

        snprintf(id, sizeof(id), "item_%d", index);
        assert(picoui_radial_menu_add_item(radial_menu, id) == 0);
    }

    assert(picoui_radial_menu_add_item(radial_menu, "overflow") == -1);
    assert(picoui_radial_menu_set_selected_index(radial_menu, 4) == 0);
    assert(picoui_radial_menu_set_selected_index(radial_menu, 5) == -1);

    picoui_app_destroy(app);
}

static void test_radial_menu_create_with_default_index_defers_selection_until_items_exist(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_radial_menu *radial_menu;
    const struct picoui_radial_menu_props props = {
        .id = "radial_menu",
        .default_index = 0,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    radial_menu = picoui_radial_menu_create_with_props((struct picoui_widget *)win, &props);
    assert(radial_menu != 0);
    assert(picoui_radial_menu_get_selected_index(radial_menu) == 0);
    assert(picoui_radial_menu_add_item(radial_menu, "weather") == 0);
    assert(picoui_radial_menu_get_selected_index(radial_menu) == 0);

    picoui_app_destroy(app);
}

static void test_radial_menu_create_with_props_pushes_backend_geometry(void)
{
    struct picoui_app *app;
    struct picoui_window *win;
    struct picoui_radial_menu *radial_menu;
    struct picoui_backend_widget *backend;
    ldRadialMenu_t *ld_radial_menu;
    const struct picoui_radial_menu_props props = {
        .id = "radial_menu",
        .width = 210,
        .height = 132,
        .x_axis = 96,
        .y_axis = 72,
        .item_max = 4,
        .default_index = 0,
    };

    app = picoui_app_create();
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    radial_menu = picoui_radial_menu_create_with_props((struct picoui_widget *)win, &props);
    assert(radial_menu != 0);
    assert(picoui_radial_menu_add_item(radial_menu, "weather") == 0);

    backend = (struct picoui_backend_widget *)radial_menu->widget.backend_widget;
    assert(backend != 0);
    ld_radial_menu = (ldRadialMenu_t *)backend->ld_widget;
    assert(ld_radial_menu != 0);

    assert(ld_radial_menu->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == props.width);
    assert(ld_radial_menu->use_as__ldBase_t.use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == props.height);
    assert(ld_radial_menu->xAxis == props.x_axis);
    assert(ld_radial_menu->yAxis == props.y_axis);
    assert(ld_radial_menu->itemMax == props.item_max);
    assert(ld_radial_menu->selectItem == props.default_index);

    picoui_app_destroy(app);
}

int main(void)
{
    test_radial_menu_navigation_and_selection_follow_backend_truth();
    test_radial_menu_rejects_items_beyond_native_capacity();
    test_radial_menu_create_with_default_index_defers_selection_until_items_exist();
    test_radial_menu_create_with_props_pushes_backend_geometry();
    return 0;
}
