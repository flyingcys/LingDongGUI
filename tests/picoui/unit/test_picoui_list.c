#include "picoui/app.h"
#include "picoui/button.h"
#include "picoui/list.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldBase.h"
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

static struct picoui_backend_app_state *list_app_state(struct picoui_list *list)
{
    struct picoui_backend_widget *backend;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    assert(backend->owner != 0);
    assert(backend->owner->backend_app != 0);
    return (struct picoui_backend_app_state *)backend->owner->backend_app;
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
    assert(app_state->ld_scene->ptMsgQueue != 0
           || ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
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
    assert(backend->last_native_signal == SIGNAL_CLICKED_ITEM);
    assert(backend->last_native_value == 0);
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

static void test_selected_index_readback_matches_native_queue_after_preselected_state(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldList_t *ld_list;
    int user_cookie = 71;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_readback_truth_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_readback_truth");
    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_add_item(list, "item_display", "Display") == 0);
    picoui_list_set_on_selected(list, on_list_selected, &user_cookie);
    assert(picoui_list_set_selected_index(list, 1) == 0);
    assert(picoui_list_get_selected_index(list) == 1);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    app_state = (struct picoui_backend_app_state *)backend->owner->backend_app;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(app_state->ld_scene->ptMsgQueue != 0
           || ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);
    assert(ldMsgConnect(backend->ld_widget, SIGNAL_CLICKED_ITEM, on_native_list_clicked_probe) == true);

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
    {
        int backend_selected = picoui_backend_list_get_selected_index(list->widget.backend_widget);
        int public_selected = picoui_list_get_selected_index(list);
        assert(native_list_clicked_count == 1);
        assert(native_list_clicked_index == 0);
        assert(backend_selected == 0);
        assert(list->selected_index == 0);
        assert(public_selected == 0);
        assert(list_selected_count == 1);
        assert(list_selected_index == 0);
        assert(list_selected_user_data == &user_cookie);
        assert(backend->value == 0);
        assert(backend->dispatch_count == 1);
    }

    picoui_app_destroy(app);
}

static void test_selected_index_getter_resynchronizes_internal_and_backend_cache_from_native_truth(
    struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_sync_truth_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_sync_truth");
    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_set_selected_index(list, 0) == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    ldListSetSelectItem(ld_list, 1);
    list->selected_index = 0;
    backend->value = 0;

    assert(picoui_list_get_selected_index(list) == 1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);

    picoui_app_destroy(app);
}

static void test_hidden_or_disabled_list_releases_focus_and_rejects_native_selection(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    ldList_t *ld_list;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_focus_hidden_disabled_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_focus_hidden_disabled");
    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    app_state = list_app_state(list);
    assert(app_state->ld_scene != 0);
    assert(app_state->ld_scene->ptMsgQueue != 0
           || ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_widget_is_focus_owner(&list->widget) == 1);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);

    assert(picoui_widget_set_visible(&list->widget, 0) == 0);
    assert(picoui_widget_is_focus_owner(&list->widget) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    assert(picoui_widget_set_visible(&list->widget, 1) == 0);
    assert(picoui_widget_set_enabled(&list->widget, 0) == 0);
    assert(picoui_widget_is_focus_owner(&list->widget) == 0);
    ldListSetSelectItem(ld_list, 0);
    list->selected_index = 0;
    backend->value = 0;
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    assert(picoui_widget_set_enabled(&list->widget, 1) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    assert(picoui_widget_set_visible(&list->widget, 0) == 0);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    assert(picoui_widget_set_visible(&list->widget, 1) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    assert(picoui_widget_set_enabled(&list->widget, 0) == 0);
    ldMsgProcess(app_state->ld_scene);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    picoui_app_destroy(app);
}

static void test_selected_index_getter_clears_to_backend_unselected_truth(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_sync_unselected_truth_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_sync_unselected_truth");
    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_set_selected_index(list, 1) == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    ldListSetSelectItem(ld_list, -1);
    list->selected_index = 1;
    backend->value = 1;

    assert(picoui_list_get_selected_index(list) == -1);
    assert(list->selected_index == -1);
    assert(backend->value == -1);

    picoui_app_destroy(app);
}

static void test_list_item_marker_is_support_not_reject(struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_marker_support");
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_add_item(list, "item_display", "Display") == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(picoui_list_get_selected_index(list) == -1);
    assert(ldListGetSelectItem(ld_list) == -1);

    assert(picoui_list_set_selected_index(list, 2) == 0);
    assert(picoui_list_get_selected_index(list) == 2);
    assert(list->selected_index == 2);
    assert(backend->value == 2);
    assert(ldListGetSelectItem(ld_list) == 2);

    ldListSetSelectItem(ld_list, 1);
    list->selected_index = 2;
    backend->value = 2;
    assert(picoui_list_get_selected_index(list) == 1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);
    assert(ldListGetSelectItem(ld_list) == 1);
}

static void test_list_native_item_height_padding_margin_round_trip(struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_native_spacing");
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(picoui_list_set_item_height(list, 24) == 0);
    assert(picoui_list_set_padding_group(list, 1, 2, 3, 4) == 0);
    assert(picoui_list_set_margin_group(list, 5, 6, 7, 8) == 0);

    assert(ld_list->itemHeight == 24);
    assert(ld_list->padding.top == 1);
    assert(ld_list->padding.bottom == 2);
    assert(ld_list->padding.left == 3);
    assert(ld_list->padding.right == 4);
    assert(ld_list->margin.top == 5);
    assert(ld_list->margin.bottom == 6);
    assert(ld_list->margin.left == 7);
    assert(ld_list->margin.right == 8);

    assert(picoui_list_set_item_height(list, 0) == -1);
    assert(picoui_list_set_padding_group(list, -1, 2, 3, 4) == -1);
    assert(picoui_list_set_margin_group(list, 5, 6, 7, 256) == -1);

    assert(ld_list->itemHeight == 24);
    assert(ld_list->padding.top == 1);
    assert(ld_list->padding.bottom == 2);
    assert(ld_list->padding.left == 3);
    assert(ld_list->padding.right == 4);
    assert(ld_list->margin.top == 5);
    assert(ld_list->margin.bottom == 6);
    assert(ld_list->margin.left == 7);
    assert(ld_list->margin.right == 8);
}

static unsigned int test_list_rgb_to_ld_color(unsigned int rgb)
{
    return (unsigned int)__RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static void test_list_native_color_and_align_round_trip(struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_native_color_align");
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(picoui_list_set_text_color(list, 0x112233U) == 0);
    assert(picoui_list_set_bg_color(list, 0x445566U) == 0);
    assert(picoui_list_set_select_color(list, 0x778899U) == 0);
    assert(picoui_list_set_align(list, PICOUI_ALIGN_END) == 0);

    assert(ld_list->textColor == test_list_rgb_to_ld_color(0x112233U));
    assert(ld_list->bgColor == test_list_rgb_to_ld_color(0x445566U));
    assert(ld_list->selectColor == test_list_rgb_to_ld_color(0x778899U));
    assert(ld_list->tAlign == ARM_2D_ALIGN_RIGHT);

    assert(picoui_list_set_align(list, PICOUI_ALIGN_CENTER) == 0);
    assert(ld_list->tAlign == ARM_2D_ALIGN_CENTRE);

    assert(picoui_list_set_align(list, PICOUI_ALIGN_SPACE_BETWEEN) == -1);
    assert(ld_list->tAlign == ARM_2D_ALIGN_CENTRE);
}

static void test_list_native_item_widget_reparents_backend_tree(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_button *button;
    struct picoui_backend_widget *list_backend;
    struct picoui_backend_widget *button_backend;
    struct picoui_backend_widget *window_backend;
    ldList_t *ld_list;
    ldBase_t *ld_button;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_item_widget_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_item_widget");
    button = picoui_button_create(owned_win, "list_item_widget_button");
    assert(list != 0);
    assert(button != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    list_backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    button_backend = (struct picoui_backend_widget *)button->widget.backend_widget;
    window_backend = (struct picoui_backend_widget *)owned_win->widget.backend_widget;
    assert(list_backend != 0);
    assert(button_backend != 0);
    assert(window_backend != 0);
    ld_list = (ldList_t *)list_backend->ld_widget;
    ld_button = (ldBase_t *)button_backend->ld_widget;
    assert(ld_list != 0);
    assert(ld_button != 0);

    assert(button_backend->parent == window_backend);
    assert(ldBaseGetParent(ld_button) == (ldBase_t *)window_backend->ld_widget);

    assert(picoui_list_set_item_widget(list, 1, &button->widget) == 0);

    assert(button_backend->parent == list_backend);
    assert(button_backend->root == window_backend);
    assert(ldBaseGetParent(ld_button) == (ldBase_t *)ld_list);
    assert(button_backend->next_sibling == 0);
    assert(list_backend->first_child == button_backend);

    assert(picoui_list_set_item_widget(list, -1, &button->widget) == -1);
    assert(picoui_list_set_item_widget(list, 2, &button->widget) == -1);
    assert(picoui_list_set_item_widget(list, 1, 0) == -1);

    picoui_app_destroy(app);
}

static void test_repeated_native_clicked_item_same_index_is_noop_contract(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_backend_widget *backend;
    struct picoui_backend_app_state *app_state;
    int user_cookie = 61;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_repeat_click_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_repeat_click");
    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    picoui_list_set_on_selected(list, on_list_selected, &user_cookie);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    app_state = list_app_state(list);
    assert(app_state->ld_scene != 0);
    assert(app_state->ld_scene->ptMsgQueue != 0
           || ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);

    reset_list_signal_counters(backend);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(list_selected_count == 1);
    assert(list_selected_index == 0);
    assert(picoui_list_get_selected_index(list) == 0);

    reset_list_signal_counters(backend);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(list_selected_count == 0);
    assert(list_selected_index == -1);
    assert(picoui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
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

static void test_list_style_class_and_user_data_are_stable_widget_metadata_contract(
    struct picoui_window *win)
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

static void test_list_item_ids_are_picoui_data_not_backend_widget_identity(struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_item_contract");
    struct picoui_backend_widget *backend;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    assert(backend->ld_widget != 0);

    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    assert(list->item_count == 2);
    assert(backend->list_item_count == 2);

    assert(list->items[0].id != list->id);
    assert(list->items[0].id != backend->id);
    assert(list->items[0].id != (const char *)backend->ld_widget);
    assert(list->items[1].id != list->id);
    assert(list->items[1].id != backend->id);
    assert(list->items[1].id != (const char *)backend->ld_widget);
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
    test_list_style_class_and_user_data_are_stable_widget_metadata_contract(win);
    test_list_item_ids_are_picoui_data_not_backend_widget_identity(win);
    test_enabled_contract_and_native_selected_bridge(win);
    test_selected_index_readback_matches_native_queue_after_preselected_state(win);
    test_selected_index_getter_resynchronizes_internal_and_backend_cache_from_native_truth(win);
    test_selected_index_getter_clears_to_backend_unselected_truth(win);
    test_list_item_marker_is_support_not_reject(win);
    test_list_native_item_height_padding_margin_round_trip(win);
    test_list_native_color_and_align_round_trip(win);
    test_list_native_item_widget_reparents_backend_tree(win);
    test_hidden_or_disabled_list_releases_focus_and_rejects_native_selection(win);
    test_repeated_native_clicked_item_same_index_is_noop_contract(win);
    test_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
