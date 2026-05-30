#include "picoui/app.h"
#include "picoui/list.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/misc/ldMsg.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static int list_selected_count = 0;
static int list_selected_index = -1;
static void *list_selected_user_data = 0;
static int native_list_clicked_count = 0;
static int native_list_clicked_index = -1;

static void on_list_selected(struct picoui_list *list, int index, void *user_data)
{
    list_selected_count++;
    list_selected_index = index;
    list_selected_user_data = user_data;
    assert(list != 0);
}

static bool on_native_list_clicked_probe(ld_scene_t *scene, ldMsg_t msg)
{
    (void)scene;
    native_list_clicked_count++;
    native_list_clicked_index = (int)msg.value;
    return false;
}

static uint64_t make_signal_value_xy(uint16_t x, uint16_t y)
{
    return ((uint64_t)x << 16) | (uint64_t)y;
}

static uint64_t make_hold_down_value_y(uint16_t offset_y)
{
    return (uint64_t)offset_y << 32;
}

static void reset_list_signal_counters(struct picoui_backend_widget *backend)
{
    list_selected_count = 0;
    list_selected_index = -1;
    list_selected_user_data = 0;
    native_list_clicked_count = 0;
    native_list_clicked_index = -1;
    backend->dispatch_count = 0;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
}

static void assert_list_selectable_state(struct picoui_list *list, int expected_enabled)
{
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(list->widget.enabled == expected_enabled);
    assert(ldBaseIsSelectable(ld_base) == (expected_enabled != 0));
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

static void test_enabled_contract_and_native_selected_bridge(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    int user_cookie = 23;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldList_t *ld_list;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "enabled_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "enabled_list");
    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    picoui_list_set_on_selected(list, on_list_selected, &user_cookie);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    assert(backend->owner != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);
    assert(ldMsgConnect(backend->ld_widget, SIGNAL_CLICKED_ITEM, on_native_list_clicked_probe) == true);

    assert_list_selectable_state(list, 1);
    reset_list_signal_counters(backend);

    assert(picoui_widget_set_enabled(&list->widget, 0) == 0);
    assert_list_selectable_state(list, 0);
    assert(ldListGetSelectItem(ld_list) == -1);
    ld_list->use_as__ldBase_t.isDirtyRegionUpdate = false;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(native_list_clicked_count == 0);
    assert(native_list_clicked_index == -1);
    assert(list_selected_count == 0);
    assert(list_selected_index == -1);
    assert(list_selected_user_data == 0);
    assert(picoui_list_get_selected_index(list) == -1);
    assert(ldListGetSelectItem(ld_list) == -1);
    assert(backend->value == -1);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_NONE);
    assert(backend->dispatch_count == 0);
    assert(ld_list->offset == 0);
    assert(ld_list->use_as__ldBase_t.isDirtyRegionUpdate == false);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_HOLD_DOWN,
                     make_hold_down_value_y(20)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ld_list->offset == 0);
    assert(ld_list->use_as__ldBase_t.isDirtyRegionUpdate == false);

    assert(picoui_widget_set_enabled(&list->widget, 1) == 0);
    assert_list_selectable_state(list, 1);
    reset_list_signal_counters(backend);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(native_list_clicked_count == 1);
    assert(native_list_clicked_index == 0);
    assert(list_selected_count == 1);
    assert(list_selected_index == 0);
    assert(list_selected_user_data == &user_cookie);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(backend->value == 0);
    assert(backend->last_signal == PICOUI_BACKEND_SIGNAL_VALUE_CHANGED);
    assert(backend->dispatch_count == 1);

    reset_list_signal_counters(backend);
    assert(picoui_widget_set_enabled(&list->widget, 1) == 0);
    assert_list_selectable_state(list, 1);
    ld_list->offset = 0;
    ld_list->_offset = 0;
    ld_list->isHoldMove = false;
    ld_list->isMoveReset = false;
    ldListSetSelectItem(ld_list, -1);
    list->selected_index = -1;
    backend->value = -1;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_HOLD_DOWN,
                     make_hold_down_value_y(20)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ld_list->isHoldMove == true);
    assert(ld_list->offset == 20);

    assert(picoui_widget_set_enabled(&list->widget, 0) == 0);
    assert_list_selectable_state(list, 0);
    ld_list->use_as__ldBase_t.isDirtyRegionUpdate = false;

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(native_list_clicked_count == 0);
    assert(list_selected_count == 0);
    assert(ld_list->isHoldMove == false);
    assert(ld_list->isMoveReset == false);
    assert(ld_list->offset == 0);
    assert(ld_list->_offset == 0);
    assert(ld_list->use_as__ldBase_t.isDirtyRegionUpdate == false);

    assert(picoui_widget_set_enabled(&list->widget, 1) == 0);
    assert_list_selectable_state(list, 1);
    reset_list_signal_counters(backend);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_PRESS,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_RELEASE,
                     make_signal_value_xy(10, 10)) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(native_list_clicked_count == 1);
    assert(native_list_clicked_index == 0);
    assert(list_selected_count == 1);
    assert(list_selected_index == 0);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(backend->value == 0);

    picoui_app_destroy(app);
}

static void test_list_widget_user_data_is_distinct_from_on_selected_cookie(struct picoui_window *win)
{
    int widget_cookie = 101;
    int callback_cookie = 202;
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_metadata_user_data");
    struct picoui_backend_widget *backend;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);

    assert(picoui_widget_set_user_data(&list->widget, &widget_cookie) == 0);
    assert(list->widget.user_data == &widget_cookie);
    assert(backend->user_data == &widget_cookie);
    assert(list->user_data == 0);

    picoui_list_set_on_selected(list, on_list_selected, &callback_cookie);

    assert(list->widget.user_data == &widget_cookie);
    assert(backend->user_data == &widget_cookie);
    assert(list->user_data == &callback_cookie);
    assert(list->widget.user_data != list->user_data);
    assert(backend->user_data != list->user_data);
}

static void test_list_style_class_and_user_data_are_metadata_only_contract(struct picoui_window *win)
{
    const char *style_class = "menu";
    int widget_cookie = 303;
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_metadata_style");
    struct picoui_backend_widget *backend;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);

    assert(picoui_widget_set_style_class(&list->widget, style_class) == 0);
    assert(picoui_widget_set_user_data(&list->widget, &widget_cookie) == 0);

    assert(list->widget.style_class != 0);
    assert(backend->style_class != 0);
    assert(list->widget.style_class == style_class);
    assert(backend->style_class == style_class);
    assert(list->widget.style_class == backend->style_class);
    assert(list->widget.user_data == &widget_cookie);
    assert(backend->user_data == &widget_cookie);
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
    test_list_widget_user_data_is_distinct_from_on_selected_cookie(win);
    test_list_style_class_and_user_data_are_metadata_only_contract(win);
    test_enabled_contract_and_native_selected_bridge(win);
    test_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
