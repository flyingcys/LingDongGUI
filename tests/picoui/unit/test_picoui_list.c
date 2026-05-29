#include "picoui/app.h"
#include "picoui/list.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <assert.h>

static void on_list_selected(struct picoui_list *list, int index, void *user_data)
{
    (void)list;
    (void)index;
    (void)user_data;
}

static void test_create_and_props(struct picoui_window *win)
{
    int user_cookie = 42;
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list_props props = {
        .id = "settings",
        .style_class = "menu",
        .user_data = &user_cookie,
    };
    struct picoui_list *list = picoui_list_create(parent, "list");
    struct picoui_list *list_with_props = picoui_list_create_with_props(parent, &props);

    assert(list != 0);
    assert(list_with_props != 0);
}

static void test_items_selection_and_callback_contract(struct picoui_window *win)
{
    int user_cookie = 7;
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list");

    assert(list != 0);
    assert(picoui_list_get_selected_index(list) == -1);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_add_item(list, "item_display", "Display") == 0);
    assert(picoui_list_set_selected_index(list, 2) == 0);
    assert(picoui_list_get_selected_index(list) == 2);

    picoui_list_set_on_selected(list, on_list_selected, &user_cookie);
}

static void test_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list");
    int i;

    assert(list != 0);
    assert(picoui_list_create(0, "list") == 0);
    assert(picoui_list_create_with_props(0, &(struct picoui_list_props){.id = "list"}) == 0);
    assert(picoui_list_create(parent, 0) == 0);
    assert(picoui_list_create_with_props(parent, 0) == 0);
    assert(picoui_list_create_with_props(parent, &(struct picoui_list_props){0}) == 0);
    assert(picoui_list_add_item(0, "missing_list", "Missing list") == -1);
    assert(picoui_list_add_item(list, 0, "Missing id") == -1);
    assert(picoui_list_add_item(list, "missing_text", 0) == -1);
    assert(picoui_list_set_selected_index(0, 0) == -1);
    assert(picoui_list_set_selected_index(list, 0) == -1);
    assert(picoui_list_get_selected_index(0) == -1);

    for (i = 0; i < 16; ++i) {
        assert(picoui_list_add_item(list, "item", "Item") == 0);
    }
    assert(picoui_list_set_selected_index(list, 15) == 0);
    assert(picoui_list_set_selected_index(list, 16) == -1);
    assert(picoui_list_set_selected_index(list, -1) == -1);
    assert(picoui_list_add_item(list, "overflow", "Overflow") == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");

    assert(app != 0);
    assert(win != 0);

    test_create_and_props(win);
    test_items_selection_and_callback_contract(win);
    test_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
