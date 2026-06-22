#include "app.h"
#include "button.h"
#include "list.h"
#include "widget.h"
#include "window.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static const char *test_self_binary_path = 0;

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        size_t symbol_len = strlen(symbol);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        if (line_len >= symbol_len &&
            strcmp(line + line_len - symbol_len, symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static void assert_source_file_lacks_symbol(const char *relative_path, const char *symbol)
{
    char full_path[1024];
    char binary_dir[1024];
    char *last_slash;
    FILE *source;
    char line[512];
    size_t symbol_len;

    assert(relative_path != 0);
    assert(symbol != 0);
    source = fopen(relative_path, "r");
    if (source == 0) {
        assert(test_self_binary_path != 0);
        assert(strlen(test_self_binary_path) < sizeof(binary_dir));
        strcpy(binary_dir, test_self_binary_path);
        last_slash = strrchr(binary_dir, '/');
        assert(last_slash != 0);
        *last_slash = '\0';
        snprintf(full_path, sizeof(full_path), "%s/../../../%s", binary_dir, relative_path);
        source = fopen(full_path, "r");
    }
    assert(source != 0);
    symbol_len = strlen(symbol);
    while (fgets(line, sizeof(line), source) != 0) {
        if (strstr(line, symbol) != 0) {
            size_t prefix_len = (size_t)(strstr(line, symbol) - line);
            char prefix = prefix_len > 0 ? line[prefix_len - 1] : ' ';
            char suffix = line[prefix_len + symbol_len];
            int prefix_is_ident = (prefix == '_' ||
                                   (prefix >= '0' && prefix <= '9') ||
                                   (prefix >= 'A' && prefix <= 'Z') ||
                                   (prefix >= 'a' && prefix <= 'z'));
            int suffix_is_ident = (suffix == '_' ||
                                   (suffix >= '0' && suffix <= '9') ||
                                   (suffix >= 'A' && suffix <= 'Z') ||
                                   (suffix >= 'a' && suffix <= 'z'));
            assert(prefix_is_ident || suffix_is_ident || !"unexpected symbol still present in source");
        }
    }
    assert(fclose(source) == 0);
}

static void assert_source_file_has_symbol(const char *relative_path, const char *symbol)
{
    char full_path[1024];
    char binary_dir[1024];
    char *last_slash;
    FILE *source;
    char line[512];

    assert(relative_path != 0);
    assert(symbol != 0);
    source = fopen(relative_path, "r");
    if (source == 0) {
        assert(test_self_binary_path != 0);
        assert(strlen(test_self_binary_path) < sizeof(binary_dir));
        strcpy(binary_dir, test_self_binary_path);
        last_slash = strrchr(binary_dir, '/');
        assert(last_slash != 0);
        *last_slash = '\0';
        snprintf(full_path, sizeof(full_path), "%s/../../../%s", binary_dir, relative_path);
        source = fopen(full_path, "r");
    }
    assert(source != 0);
    while (fgets(line, sizeof(line), source) != 0) {
        if (strstr(line, symbol) != 0) {
            assert(fclose(source) == 0);
            return;
        }
    }
    assert(fclose(source) == 0);
    assert(!"expected symbol missing from source");
}

static int list_selected_count = 0;
static int list_selected_index = -1;
static void *list_selected_user_data = 0;
static int native_list_clicked_count = 0;
static int native_list_clicked_index = -1;

static void on_list_selected(struct tinyui_list *list, int index, void *user_data)
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

static void reset_list_signal_counters(struct tinyui_widget *backend)
{
    (void)backend;
    list_selected_count = 0;
    list_selected_index = -1;
    list_selected_user_data = 0;
    native_list_clicked_count = 0;
    native_list_clicked_index = -1;
}

static struct tinyui_app *list_app_state(struct tinyui_list *list)
{
    struct tinyui_widget *backend;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);
    assert(backend->owner != 0);
    assert(backend->owner->ld_scene != 0);
    return backend->owner;
}

static void assert_list_selectable_state(struct tinyui_list *list, int expected_enabled)
{
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(list->widget.enabled == expected_enabled);
    assert(ldBaseIsSelectable(ld_base) == (expected_enabled != 0));
}

static void test_create_and_props(struct tinyui_window *win)
{
    int user_cookie = 42;
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list_props props = {
        .id = "settings",
        .style_class = "menu",
        .user_data = &user_cookie,
    };
    struct tinyui_list *list = tinyui_list_create(parent, "list");
    struct tinyui_list *list_with_props = tinyui_list_create_with_props(parent, &props);
    struct tinyui_widget *backend;
    struct tinyui_widget *parent_backend;

    assert(list != 0);
    assert(list_with_props != 0);
    backend = &list->widget;
    parent_backend = &win->widget;
    assert(backend != 0);
    assert(parent_backend != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_LIST);
    assert(backend->owner == parent_backend->owner);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)parent_backend->ld_widget));
    assert(ldBaseGetParent((ldBase_t *)backend->ld_widget) == (ldBase_t *)parent_backend->ld_widget);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_widget != 0);
    assert(((ldBase_t *)backend->ld_widget)->pInfo == backend);
}

static void test_list_legacy_backend_constructor_is_disabled(struct tinyui_window *win)
{
    (void)win;
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_create_list") == 0);
}

static void test_items_selection_and_callback_contract(struct tinyui_window *win)
{
    int user_cookie = 7;
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list");

    assert(list != 0);
    assert(tinyui_list_get_selected_index(list) == -1);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_add_item(list, "item_display", "Display") == 0);
    assert(tinyui_list_set_selected_index(list, 2) == 0);
    assert(tinyui_list_get_selected_index(list) == 2);

    tinyui_list_set_on_selected(list, on_list_selected, &user_cookie);
}

static void test_enabled_contract_and_native_selected_bridge(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    int user_cookie = 23;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldList_t *ld_list;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "enabled_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "enabled_list");
    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    tinyui_list_set_on_selected(list, on_list_selected, &user_cookie);

    backend = &list->widget;
    assert(backend != 0);
    assert(backend->owner != 0);
    app_state = backend->owner;
    assert(app_state != 0);
    assert(app_state->ld_scene != 0);
    assert(app_state->ld_scene->ptMsgQueue != 0
           || ldMsgInit(&app_state->ld_scene->ptMsgQueue, 8) == true);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);
    assert(ldMsgConnect(backend->ld_widget, SIGNAL_CLICKED_ITEM, on_native_list_clicked_probe) == true);

    assert_list_selectable_state(list, 1);
    reset_list_signal_counters(backend);

    assert(tinyui_widget_set_enabled(&list->widget, 0) == 0);
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
    assert(tinyui_list_get_selected_index(list) == -1);
    assert(ldListGetSelectItem(ld_list) == -1);
    assert(backend->value == -1);
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

    assert(tinyui_widget_set_enabled(&list->widget, 1) == 0);
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
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(backend->value == 0);

    reset_list_signal_counters(backend);
    assert(tinyui_widget_set_enabled(&list->widget, 1) == 0);
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

    assert(tinyui_widget_set_enabled(&list->widget, 0) == 0);
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

    assert(tinyui_widget_set_enabled(&list->widget, 1) == 0);
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
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(backend->value == 0);

    tinyui_app_destroy(app);
}

static void test_selected_index_readback_matches_native_queue_after_preselected_state(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldList_t *ld_list;
    int user_cookie = 71;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_readback_truth_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_readback_truth");
    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_add_item(list, "item_display", "Display") == 0);
    tinyui_list_set_on_selected(list, on_list_selected, &user_cookie);
    assert(tinyui_list_set_selected_index(list, 1) == 0);
    assert(tinyui_list_get_selected_index(list) == 1);

    backend = &list->widget;
    assert(backend != 0);
    app_state = backend->owner;
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
        int backend_selected = tinyui_list_get_selected_index(&list->widget);
        int public_selected = tinyui_list_get_selected_index(list);
        assert(native_list_clicked_count == 1);
        assert(native_list_clicked_index == 0);
        assert(backend_selected == 0);
        assert(list->selected_index == 0);
        assert(public_selected == 0);
        assert(list_selected_count == 1);
        assert(list_selected_index == 0);
        assert(list_selected_user_data == &user_cookie);
        assert(backend->value == 0);
    }

    tinyui_app_destroy(app);
}

static void test_selected_index_getter_resynchronizes_internal_and_backend_cache_from_native_truth(
    struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_sync_truth_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_sync_truth");
    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_set_selected_index(list, 0) == 0);

    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    ldListSetSelectItem(ld_list, 1);
    list->selected_index = 0;
    backend->value = 0;

    assert(tinyui_list_get_selected_index(list) == 1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);

    tinyui_app_destroy(app);
}

static void test_hidden_or_disabled_list_releases_focus_and_rejects_native_selection(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    ldList_t *ld_list;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_focus_hidden_disabled_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_focus_hidden_disabled");
    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    backend = &list->widget;
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
    assert(tinyui_widget_is_focus_owner(&list->widget) == 1);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);

    assert(tinyui_widget_set_visible(&list->widget, 0) == 0);
    assert(tinyui_widget_is_focus_owner(&list->widget) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    assert(tinyui_widget_set_visible(&list->widget, 1) == 0);
    assert(tinyui_widget_set_enabled(&list->widget, 0) == 0);
    assert(tinyui_widget_is_focus_owner(&list->widget) == 0);
    ldListSetSelectItem(ld_list, 0);
    list->selected_index = 0;
    backend->value = 0;
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    assert(tinyui_widget_set_enabled(&list->widget, 1) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    assert(tinyui_widget_set_visible(&list->widget, 0) == 0);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    assert(tinyui_widget_set_visible(&list->widget, 1) == 0);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     1) == true);
    assert(tinyui_widget_set_enabled(&list->widget, 0) == 0);
    ldMsgProcess(app_state->ld_scene);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);
    assert(ldListGetSelectItem(ld_list) == 0);

    tinyui_app_destroy(app);
}

static void test_selected_index_getter_clears_to_backend_unselected_truth(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_sync_unselected_truth_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_sync_unselected_truth");
    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_set_selected_index(list, 1) == 0);

    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    ldListSetSelectItem(ld_list, -1);
    list->selected_index = 1;
    backend->value = 1;

    assert(tinyui_list_get_selected_index(list) == -1);
    assert(list->selected_index == -1);
    assert(backend->value == -1);

    tinyui_app_destroy(app);
}

static void test_list_item_marker_is_support_not_reject(struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_marker_support");
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_add_item(list, "item_display", "Display") == 0);

    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(tinyui_list_get_selected_index(list) == -1);
    assert(ldListGetSelectItem(ld_list) == -1);

    assert(tinyui_list_set_selected_index(list, 2) == 0);
    assert(tinyui_list_get_selected_index(list) == 2);
    assert(list->selected_index == 2);
    assert(backend->value == 2);
    assert(ldListGetSelectItem(ld_list) == 2);

    ldListSetSelectItem(ld_list, 1);
    list->selected_index = 2;
    backend->value = 2;
    assert(tinyui_list_get_selected_index(list) == 1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);
    assert(ldListGetSelectItem(ld_list) == 1);
}

static void test_list_native_item_height_padding_margin_round_trip(struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_native_spacing");
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(tinyui_list_set_item_height(list, 24) == 0);
    assert(tinyui_list_set_padding_group(list, 1, 2, 3, 4) == 0);
    assert(tinyui_list_set_margin_group(list, 5, 6, 7, 8) == 0);

    assert(ld_list->itemHeight == 24);
    assert(ld_list->padding.top == 1);
    assert(ld_list->padding.bottom == 2);
    assert(ld_list->padding.left == 3);
    assert(ld_list->padding.right == 4);
    assert(ld_list->margin.top == 5);
    assert(ld_list->margin.bottom == 6);
    assert(ld_list->margin.left == 7);
    assert(ld_list->margin.right == 8);

    assert(tinyui_list_set_item_height(list, 0) == -1);
    assert(tinyui_list_set_padding_group(list, -1, 2, 3, 4) == -1);
    assert(tinyui_list_set_margin_group(list, 5, 6, 7, 256) == -1);

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

static void test_list_native_color_and_align_round_trip(struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_native_color_align");
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(tinyui_list_set_text_color(list, 0x112233U) == 0);
    assert(tinyui_list_set_bg_color(list, 0x445566U) == 0);
    assert(tinyui_list_set_select_color(list, 0x778899U) == 0);
    assert(tinyui_list_set_align(list, TINYUI_ALIGN_END) == 0);

    assert(ld_list->textColor == test_list_rgb_to_ld_color(0x112233U));
    assert(ld_list->bgColor == test_list_rgb_to_ld_color(0x445566U));
    assert(ld_list->selectColor == test_list_rgb_to_ld_color(0x778899U));
    assert(ld_list->tAlign == ARM_2D_ALIGN_RIGHT);

    assert(tinyui_list_set_align(list, TINYUI_ALIGN_CENTER) == 0);
    assert(ld_list->tAlign == ARM_2D_ALIGN_CENTRE);

    assert(tinyui_list_set_align(list, TINYUI_ALIGN_SPACE_BETWEEN) == -1);
    assert(ld_list->tAlign == ARM_2D_ALIGN_CENTRE);
}

static void test_list_backend_moved_helpers_fail_closed_without_mutation(struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_backend_helper_guard");
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_set_item_height(list, 24) == 0);
    assert(tinyui_list_set_padding_group(list, 1, 2, 3, 4) == 0);
    assert(tinyui_list_set_margin_group(list, 5, 6, 7, 8) == 0);
    assert(tinyui_list_set_text_color(list, 0x112233U) == 0);
    assert(tinyui_list_set_bg_color(list, 0x445566U) == 0);
    assert(tinyui_list_set_select_color(list, 0x778899U) == 0);
    assert(tinyui_list_set_align(list, TINYUI_ALIGN_END) == 0);

    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(tinyui_list_set_selected_index(0, 0) == -1);
    assert(tinyui_list_get_selected_index(0) == -1);
    assert(tinyui_list_sync_selected_index(0, 0) == -1);

    assert(list->item_count == 2);
    assert(backend->list_item_count == 2);
    assert(ld_list->itemCount == 2);
    assert(ld_list->itemHeight == 24);
    assert(ld_list->padding.top == 1);
    assert(ld_list->padding.bottom == 2);
    assert(ld_list->padding.left == 3);
    assert(ld_list->padding.right == 4);
    assert(ld_list->margin.top == 5);
    assert(ld_list->margin.bottom == 6);
    assert(ld_list->margin.left == 7);
    assert(ld_list->margin.right == 8);
    assert(ld_list->textColor == test_list_rgb_to_ld_color(0x112233U));
    assert(ld_list->bgColor == test_list_rgb_to_ld_color(0x445566U));
    assert(ld_list->selectColor == test_list_rgb_to_ld_color(0x778899U));
    assert(ld_list->tAlign == ARM_2D_ALIGN_RIGHT);
    assert(tinyui_list_set_selected_index_ld(backend, 1) == 0);
    assert(tinyui_list_get_selected_index_ld(backend) == 1);
    assert(list->selected_index == -1);
    assert(tinyui_list_sync_selected_index(list, 0) == 0);
    assert(list->selected_index == 1);
    assert(backend->value == 1);
    assert(tinyui_list_set_selected_index_ld(backend, -1) == -1);
    assert(tinyui_list_set_selected_index_ld(backend, TINYUI_BACKEND_LIST_MAX_ITEMS) == -1);
    assert(tinyui_list_get_selected_index_ld(backend) == 1);
}

static void test_list_legacy_backend_helper_symbols_are_removed(struct tinyui_window *win)
{
    (void)win;
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_list_ld_widget");
    /* Phase C2: private color/align/backend/get_ld helpers folded into core helpers
     * (tinyui_rgb_to_ld_color / tinyui_align_to_arm2d) and direct ld_widget casts. */
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_list_rgb_to_ld_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_list_map_align");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_list_backend");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_list_get_ld");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_items");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_item_height");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_padding_group");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_margin_group");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_text_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_bg_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_select_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_align");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "tinyui_backend_list_set_item_widget");
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_item_height") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_padding_group") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_margin_group") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_text_color") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_bg_color") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_select_color") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_align") == 0);
    assert(dlsym(RTLD_DEFAULT, "tinyui_backend_list_set_item_widget") == 0);
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_items");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_item_height");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_padding_group");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_margin_group");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_text_color");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_bg_color");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_select_color");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_align");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_item_widget");
    assert_self_binary_lacks_symbol("tinyui_backend_list_set_selected_index");
    assert_self_binary_lacks_symbol("tinyui_backend_list_get_selected_index");
    assert_self_binary_lacks_symbol("tinyui_backend_list_sync_selected_index");
}

static void test_list_backend_selected_index_sync_rejects_corrupted_binding_without_cache_pollution(
    struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_sync_binding_guard");
    struct tinyui_widget *backend;
    enum tinyui_backend_widget_kind original_kind;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_set_selected_index(list, 1) == 0);

    backend = &list->widget;
    assert(backend != 0);
    assert(backend->value == 1);

    original_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_GRAPH;

    assert(tinyui_list_sync_selected_index(list, 0) == -1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);

    backend->kind = original_kind;
}

static void test_list_rejects_corrupted_backend_binding_without_mutating_widget_metadata(
    struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_binding_guard_style");
    struct tinyui_widget *backend;
    ldList_t *ld_list;
    enum tinyui_backend_widget_kind original_kind;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(tinyui_list_set_text_color(list, 0x112233U) == 0);
    assert(tinyui_list_set_bg_color(list, 0x445566U) == 0);
    assert(tinyui_list_set_select_color(list, 0x778899U) == 0);
    assert(tinyui_list_set_align(list, TINYUI_ALIGN_END) == 0);

    original_kind = backend->kind;
    backend->kind = TINYUI_BACKEND_WIDGET_GRAPH;

    assert(tinyui_list_set_text_color(list, 0x010203U) == -1);
    assert(tinyui_list_set_bg_color(list, 0x040506U) == -1);
    assert(tinyui_list_set_select_color(list, 0x070809U) == -1);
    assert(tinyui_list_set_align(list, TINYUI_ALIGN_START) == -1);

    assert(list->widget.text_color == 0x112233U);
    assert(list->widget.bg_color == 0x445566U);
    assert(list->widget.border_color == 0x778899U);
    assert(ld_list->textColor == test_list_rgb_to_ld_color(0x112233U));
    assert(ld_list->bgColor == test_list_rgb_to_ld_color(0x445566U));
    assert(ld_list->selectColor == test_list_rgb_to_ld_color(0x778899U));
    assert(ld_list->tAlign == ARM_2D_ALIGN_RIGHT);

    backend->kind = original_kind;
}

static void test_list_native_item_widget_reparents_backend_tree(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_list *nested_list;
    struct tinyui_button *button;
    struct tinyui_widget *list_backend;
    struct tinyui_widget *nested_list_backend;
    struct tinyui_widget *button_backend;
    struct tinyui_widget *window_backend;
    ldList_t *ld_list;
    ldList_t *ld_nested_list;
    ldBase_t *ld_button;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_item_widget_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_item_widget");
    nested_list = tinyui_list_create(parent, "list_item_widget_nested_list");
    button = tinyui_button_create((struct tinyui_widget *)nested_list, "list_item_widget_button");
    assert(list != 0);
    assert(nested_list != 0);
    assert(button != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    list_backend = &list->widget;
    nested_list_backend = &nested_list->widget;
    button_backend = &button->widget;
    window_backend = &owned_win->widget;
    assert(list_backend != 0);
    assert(nested_list_backend != 0);
    assert(button_backend != 0);
    assert(window_backend != 0);
    ld_list = (ldList_t *)list_backend->ld_widget;
    ld_nested_list = (ldList_t *)nested_list_backend->ld_widget;
    ld_button = (ldBase_t *)button_backend->ld_widget;
    assert(ld_list != 0);
    assert(ld_nested_list != 0);
    assert(ld_button != 0);

    assert(ldBaseGetParent((ldBase_t *)ld_nested_list) == (ldBase_t *)window_backend->ld_widget);
    assert(nested_list_backend->owner == app);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)nested_list_backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)window_backend->ld_widget));
    assert(ldBaseGetParent(ld_button) == (ldBase_t *)ld_nested_list);
    assert(button_backend->owner == app);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)button_backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)window_backend->ld_widget));

    assert(tinyui_list_set_item_widget(list, 1, &nested_list->widget) == 0);

    assert(ldBaseGetParent((ldBase_t *)ld_nested_list) == (ldBase_t *)ld_list);
    assert(nested_list_backend->owner == app);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)nested_list_backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)window_backend->ld_widget));
    assert(ldBaseGetParent(ld_button) == (ldBase_t *)ld_nested_list);
    assert(button_backend->owner == app);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)button_backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)window_backend->ld_widget));

    assert(tinyui_list_set_item_widget(list, -1, &nested_list->widget) == -1);
    assert(tinyui_list_set_item_widget(list, 2, &nested_list->widget) == -1);
    assert(tinyui_list_set_item_widget(list, 1, 0) == -1);

    tinyui_app_destroy(app);
}

static void test_list_set_item_widget_rejects_corrupted_binding_without_reparenting(
    struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_list *nested_list;
    struct tinyui_widget *list_backend;
    struct tinyui_widget *nested_list_backend;
    struct tinyui_widget *window_backend;
    ldList_t *ld_list;
    ldList_t *ld_nested_list;
    enum tinyui_backend_widget_kind original_kind;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_item_widget_binding_guard_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_item_widget_binding_guard");
    nested_list = tinyui_list_create(parent, "list_item_widget_binding_guard_nested");
    assert(list != 0);
    assert(nested_list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);

    list_backend = &list->widget;
    nested_list_backend = &nested_list->widget;
    window_backend = &owned_win->widget;
    assert(list_backend != 0);
    assert(nested_list_backend != 0);
    assert(window_backend != 0);
    ld_list = (ldList_t *)list_backend->ld_widget;
    ld_nested_list = (ldList_t *)nested_list_backend->ld_widget;
    assert(ld_list != 0);
    assert(ld_nested_list != 0);

    original_kind = list_backend->kind;
    list_backend->kind = TINYUI_BACKEND_WIDGET_GRAPH;

    assert(tinyui_list_set_item_widget(list, 0, &nested_list->widget) == -1);
    assert(ldBaseGetParent((ldBase_t *)ld_nested_list) == (ldBase_t *)window_backend->ld_widget);
    assert(nested_list_backend->owner == app);
    assert((ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)nested_list_backend->ld_widget) == (ldBase_t *)ldBaseGetRootNode((arm_2d_control_node_t *)window_backend->ld_widget));
    assert(ldBaseGetChildList((ldBase_t *)ld_list) == 0);
    assert(ld_list->itemCount == 1);

    list_backend->kind = original_kind;
    tinyui_app_destroy(app);
}

static void test_repeated_native_clicked_item_same_index_is_noop_contract(struct tinyui_window *win)
{
    struct tinyui_app *app;
    struct tinyui_window *owned_win;
    struct tinyui_widget *parent;
    struct tinyui_list *list;
    struct tinyui_widget *backend;
    struct tinyui_app *app_state;
    int user_cookie = 61;

    (void)win;
    app = tinyui_app_create();
    owned_win = tinyui_window_create(app, "list_repeat_click_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct tinyui_widget *)owned_win;
    list = tinyui_list_create(parent, "list_repeat_click");
    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    tinyui_list_set_on_selected(list, on_list_selected, &user_cookie);

    backend = &list->widget;
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
    assert(tinyui_list_get_selected_index(list) == 0);

    reset_list_signal_counters(backend);
    assert(ldMsgEmit(app_state->ld_scene->ptMsgQueue,
                     backend->ld_widget,
                     SIGNAL_CLICKED_ITEM,
                     0) == true);
    ldMsgProcess(app_state->ld_scene);
    assert(list_selected_count == 0);
    assert(list_selected_index == -1);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(list->selected_index == 0);
    assert(backend->value == 0);

    tinyui_app_destroy(app);
}

static void test_list_selected_index_getter_rejects_corrupted_native_truth_without_cache_pollution(
    struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_selected_binding_guard");
    struct tinyui_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(tinyui_list_set_selected_index(list, 1) == 0);

    backend = &list->widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    ldListSetSelectItem(ld_list, 9);
    assert(tinyui_list_get_selected_index(list) == 1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);
    assert(ldListGetSelectItem(ld_list) == 9);
}

static void test_list_widget_user_data_is_distinct_from_on_selected_cookie(struct tinyui_window *win)
{
    int widget_cookie = 101;
    int callback_cookie = 202;
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_metadata_user_data");
    struct tinyui_widget *backend;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);

    assert(tinyui_widget_set_user_data(&list->widget, &widget_cookie) == 0);
    assert(list->widget.user_data == &widget_cookie);
    assert(backend->user_data == &widget_cookie);
    assert(list->user_data == 0);

    tinyui_list_set_on_selected(list, on_list_selected, &callback_cookie);

    assert(list->widget.user_data == &widget_cookie);
    assert(backend->user_data == &widget_cookie);
    assert(list->user_data == &callback_cookie);
    assert(list->widget.user_data != list->user_data);
    assert(backend->user_data != list->user_data);
}

static void test_list_style_class_and_user_data_are_stable_widget_metadata_contract(
    struct tinyui_window *win)
{
    const char *style_class = "menu";
    int widget_cookie = 303;
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_metadata_style");
    struct tinyui_widget *backend;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);

    assert(tinyui_widget_set_style_class(&list->widget, style_class) == 0);
    assert(tinyui_widget_set_user_data(&list->widget, &widget_cookie) == 0);

    assert(list->widget.style_class != 0);
    assert(backend->style_class != 0);
    assert(list->widget.style_class == style_class);
    assert(backend->style_class == style_class);
    assert(list->widget.style_class == backend->style_class);
    assert(list->widget.user_data == &widget_cookie);
    assert(backend->user_data == &widget_cookie);
}

static void test_list_item_ids_are_tinyui_data_not_backend_widget_identity(struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list_item_contract");
    struct tinyui_widget *backend;

    assert(list != 0);
    backend = &list->widget;
    assert(backend != 0);
    assert(backend->ld_widget != 0);

    assert(tinyui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(tinyui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    assert(list->item_count == 2);
    assert(backend->list_item_count == 2);

    assert(list->items[0].id != list->id);
    assert(list->items[0].id != list->id);
    assert(list->items[0].id != (const char *)backend->ld_widget);
    assert(list->items[1].id != list->id);
    assert(list->items[1].id != list->id);
    assert(list->items[1].id != (const char *)backend->ld_widget);
}

static void test_rejects_invalid_inputs(struct tinyui_window *win)
{
    struct tinyui_widget *parent = (struct tinyui_widget *)win;
    struct tinyui_list *list = tinyui_list_create(parent, "list");
    int i;

    assert(list != 0);
    assert(tinyui_list_create(0, "list") == 0);
    assert(tinyui_list_create_with_props(0, &(struct tinyui_list_props){.id = "list"}) == 0);
    assert(tinyui_list_create(parent, 0) == 0);
    assert(tinyui_list_create_with_props(parent, 0) == 0);
    assert(tinyui_list_create_with_props(parent, &(struct tinyui_list_props){0}) == 0);
    assert(tinyui_list_add_item(0, "missing_list", "Missing list") == -1);
    assert(tinyui_list_add_item(list, 0, "Missing id") == -1);
    assert(tinyui_list_add_item(list, "missing_text", 0) == -1);
    assert(tinyui_list_set_selected_index(0, 0) == -1);
    assert(tinyui_list_set_selected_index(list, 0) == -1);
    assert(tinyui_list_get_selected_index(0) == -1);

    for (i = 0; i < 16; ++i) {
        assert(tinyui_list_add_item(list, "item", "Item") == 0);
    }
    assert(tinyui_list_set_selected_index(list, 15) == 0);
    assert(tinyui_list_set_selected_index(list, 16) == -1);
    assert(tinyui_list_set_selected_index(list, -1) == -1);
    assert(tinyui_list_add_item(list, "overflow", "Overflow") == -1);
}

int main(int argc, char **argv)
{
    (void)argc;
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win = tinyui_window_create(app, "root");

    test_self_binary_path = argv[0];
    assert(app != 0);
    assert(win != 0);

    test_create_and_props(win);
    test_list_legacy_backend_constructor_is_disabled(win);
    test_items_selection_and_callback_contract(win);
    test_list_widget_user_data_is_distinct_from_on_selected_cookie(win);
    test_list_style_class_and_user_data_are_stable_widget_metadata_contract(win);
    test_list_item_ids_are_tinyui_data_not_backend_widget_identity(win);
    test_enabled_contract_and_native_selected_bridge(win);
    test_selected_index_readback_matches_native_queue_after_preselected_state(win);
    test_selected_index_getter_resynchronizes_internal_and_backend_cache_from_native_truth(win);
    test_selected_index_getter_clears_to_backend_unselected_truth(win);
    test_list_item_marker_is_support_not_reject(win);
    test_list_native_item_height_padding_margin_round_trip(win);
    test_list_native_color_and_align_round_trip(win);
    test_list_backend_moved_helpers_fail_closed_without_mutation(win);
    test_list_legacy_backend_helper_symbols_are_removed(win);
    test_list_backend_selected_index_sync_rejects_corrupted_binding_without_cache_pollution(win);
    test_list_rejects_corrupted_backend_binding_without_mutating_widget_metadata(win);
    test_list_native_item_widget_reparents_backend_tree(win);
    test_list_set_item_widget_rejects_corrupted_binding_without_reparenting(win);
    test_hidden_or_disabled_list_releases_focus_and_rejects_native_selection(win);
    test_repeated_native_clicked_item_same_index_is_noop_contract(win);
    test_list_selected_index_getter_rejects_corrupted_native_truth_without_cache_pollution(win);
    test_rejects_invalid_inputs(win);

    tinyui_app_destroy(app);
    return 0;
}
