#include "picoui/app.h"
#include "picoui/radial_menu.h"
#include "picoui/widget.h"
#include "picoui/window.h"

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

int main(void)
{
    test_radial_menu_navigation_and_selection_follow_backend_truth();
    test_radial_menu_rejects_items_beyond_native_capacity();
    test_radial_menu_create_with_default_index_defers_selection_until_items_exist();
    return 0;
}
