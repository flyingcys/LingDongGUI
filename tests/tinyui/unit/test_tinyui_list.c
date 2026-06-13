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

int tinyui_list_set_items(void *backend_widget,
                          const char *const *item_ids,
                          const unsigned char *const *items,
                          int item_count);
int tinyui_list_set_item_height(void *backend_widget, int item_height);
int tinyui_list_set_padding_group(void *backend_widget,
                                  int top,
                                  int bottom,
                                  int left,
                                  int right);
int tinyui_list_set_margin_group(void *backend_widget,
                                 int top,
                                 int bottom,
                                 int left,
                                 int right);
int tinyui_list_set_text_color(void *backend_widget, unsigned int rgb);
int tinyui_list_set_bg_color(void *backend_widget, unsigned int rgb);
int tinyui_list_set_select_color(void *backend_widget, unsigned int rgb);
int tinyui_list_set_align(void *backend_widget, enum picoui_align align);
int tinyui_list_set_item_widget(void *backend_widget, int index, void *item_widget_backend);

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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;

    assert(list != 0);
    assert(list_with_props != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(backend != 0);
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_LIST);
    assert(backend->owner == parent_backend->owner);
    assert(backend->root == parent_backend->root);
    assert(backend->parent == parent_backend);
    assert(backend->ld_name_id != 0);
    assert(backend->host_widget == &list->widget);
    assert(backend->ld_widget != 0);
    assert(((ldBase_t *)backend->ld_widget)->pInfo == backend);
}

static void test_list_legacy_backend_constructor_is_disabled(struct picoui_window *win)
{
    (void)win;
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_create_list") == 0);
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
        int backend_selected = tinyui_list_get_selected_index(list->widget.backend_widget);
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

static void test_list_backend_moved_helpers_fail_closed_without_mutation(struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_backend_helper_guard");
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_set_item_height(list, 24) == 0);
    assert(picoui_list_set_padding_group(list, 1, 2, 3, 4) == 0);
    assert(picoui_list_set_margin_group(list, 5, 6, 7, 8) == 0);
    assert(picoui_list_set_text_color(list, 0x112233U) == 0);
    assert(picoui_list_set_bg_color(list, 0x445566U) == 0);
    assert(picoui_list_set_select_color(list, 0x778899U) == 0);
    assert(picoui_list_set_align(list, PICOUI_ALIGN_END) == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(tinyui_list_set_items(backend,
                                 (const char *const *)list->backend_item_ids,
                                 list->backend_item_texts,
                                 list->item_count) == -1);
    assert(tinyui_list_set_item_height(backend, 18) == -1);
    assert(tinyui_list_set_padding_group(backend, 9, 9, 9, 9) == -1);
    assert(tinyui_list_set_margin_group(backend, 6, 6, 6, 6) == -1);
    assert(tinyui_list_set_text_color(backend, 0x010203U) == -1);
    assert(tinyui_list_set_bg_color(backend, 0x040506U) == -1);
    assert(tinyui_list_set_select_color(backend, 0x070809U) == -1);
    assert(tinyui_list_set_align(backend, PICOUI_ALIGN_START) == -1);
    assert(tinyui_list_set_item_widget(backend, 0, backend) == -1);
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
    assert(tinyui_list_set_selected_index(backend, 1) == 0);
    assert(tinyui_list_get_selected_index(backend) == 1);
    assert(list->selected_index == -1);
    assert(tinyui_list_sync_selected_index(list, 0) == 0);
    assert(list->selected_index == 1);
    assert(backend->value == 1);
    assert(tinyui_list_set_selected_index(backend, -1) == -1);
    assert(tinyui_list_set_selected_index(backend, PICOUI_BACKEND_LIST_MAX_ITEMS) == -1);
    assert(tinyui_list_get_selected_index(backend) == 1);
}

static void test_list_legacy_backend_helper_symbols_are_removed(struct picoui_window *win)
{
    (void)win;
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_list_rgb_to_ld_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_list_map_align");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_list_backend");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_list_ld_widget");
    assert_source_file_has_symbol("tinyui/src/widgets/list.c", "tinyui_list_rgb_to_ld_color");
    assert_source_file_has_symbol("tinyui/src/widgets/list.c", "tinyui_list_map_align");
    assert_source_file_has_symbol("tinyui/src/widgets/list.c", "tinyui_list_backend");
    assert_source_file_has_symbol("tinyui/src/widgets/list.c", "tinyui_list_get_ld");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_items");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_item_height");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_padding_group");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_margin_group");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_text_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_bg_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_select_color");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_align");
    assert_source_file_lacks_symbol("tinyui/src/widgets/list.c", "picoui_backend_list_set_item_widget");
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_item_height") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_padding_group") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_margin_group") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_text_color") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_bg_color") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_select_color") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_align") == 0);
    assert(dlsym(RTLD_DEFAULT, "picoui_backend_list_set_item_widget") == 0);
    assert_self_binary_lacks_symbol("picoui_backend_list_set_items");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_item_height");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_padding_group");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_margin_group");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_text_color");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_bg_color");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_select_color");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_align");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_item_widget");
    assert_self_binary_lacks_symbol("picoui_backend_list_set_selected_index");
    assert_self_binary_lacks_symbol("picoui_backend_list_get_selected_index");
    assert_self_binary_lacks_symbol("picoui_backend_list_sync_selected_index");
}

static void test_list_backend_selected_index_sync_rejects_corrupted_binding_without_cache_pollution(
    struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_sync_binding_guard");
    struct picoui_backend_widget *backend;
    enum picoui_backend_widget_kind original_kind;

    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_set_selected_index(list, 1) == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    assert(backend->value == 1);

    original_kind = backend->kind;
    backend->kind = PICOUI_BACKEND_WIDGET_GRAPH;

    assert(tinyui_list_sync_selected_index(list, 0) == -1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);

    backend->kind = original_kind;
}

static void test_list_rejects_corrupted_backend_binding_without_mutating_widget_metadata(
    struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_binding_guard_style");
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;
    enum picoui_backend_widget_kind original_kind;

    assert(list != 0);
    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    assert(picoui_list_set_text_color(list, 0x112233U) == 0);
    assert(picoui_list_set_bg_color(list, 0x445566U) == 0);
    assert(picoui_list_set_select_color(list, 0x778899U) == 0);
    assert(picoui_list_set_align(list, PICOUI_ALIGN_END) == 0);

    original_kind = backend->kind;
    backend->kind = PICOUI_BACKEND_WIDGET_GRAPH;

    assert(picoui_list_set_text_color(list, 0x010203U) == -1);
    assert(picoui_list_set_bg_color(list, 0x040506U) == -1);
    assert(picoui_list_set_select_color(list, 0x070809U) == -1);
    assert(picoui_list_set_align(list, PICOUI_ALIGN_START) == -1);

    assert(list->widget.text_color == 0x112233U);
    assert(list->widget.bg_color == 0x445566U);
    assert(list->widget.border_color == 0x778899U);
    assert(ld_list->textColor == test_list_rgb_to_ld_color(0x112233U));
    assert(ld_list->bgColor == test_list_rgb_to_ld_color(0x445566U));
    assert(ld_list->selectColor == test_list_rgb_to_ld_color(0x778899U));
    assert(ld_list->tAlign == ARM_2D_ALIGN_RIGHT);

    backend->kind = original_kind;
}

static void test_list_native_item_widget_reparents_backend_tree(struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_list *nested_list;
    struct picoui_button *button;
    struct picoui_backend_widget *list_backend;
    struct picoui_backend_widget *nested_list_backend;
    struct picoui_backend_widget *button_backend;
    struct picoui_backend_widget *window_backend;
    ldList_t *ld_list;
    ldList_t *ld_nested_list;
    ldBase_t *ld_button;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_item_widget_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_item_widget");
    nested_list = picoui_list_create(parent, "list_item_widget_nested_list");
    button = picoui_button_create((struct picoui_widget *)nested_list, "list_item_widget_button");
    assert(list != 0);
    assert(nested_list != 0);
    assert(button != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);

    list_backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    nested_list_backend = (struct picoui_backend_widget *)nested_list->widget.backend_widget;
    button_backend = (struct picoui_backend_widget *)button->widget.backend_widget;
    window_backend = (struct picoui_backend_widget *)owned_win->widget.backend_widget;
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

    assert(nested_list_backend->parent == window_backend);
    assert(nested_list_backend->owner == app);
    assert(nested_list_backend->root == window_backend);
    assert(button_backend->parent == nested_list_backend);
    assert(button_backend->owner == app);
    assert(button_backend->root == window_backend);
    assert(ldBaseGetParent((ldBase_t *)ld_nested_list) == (ldBase_t *)window_backend->ld_widget);
    assert(ldBaseGetParent(ld_button) == (ldBase_t *)ld_nested_list);

    assert(picoui_list_set_item_widget(list, 1, &nested_list->widget) == 0);

    assert(nested_list_backend->parent == list_backend);
    assert(nested_list_backend->owner == app);
    assert(nested_list_backend->root == window_backend);
    assert(ldBaseGetParent((ldBase_t *)ld_nested_list) == (ldBase_t *)ld_list);
    assert(nested_list_backend->next_sibling == 0);
    assert(list_backend->first_child == nested_list_backend);
    assert(button_backend->parent == nested_list_backend);
    assert(button_backend->owner == app);
    assert(button_backend->root == window_backend);
    assert(ldBaseGetParent(ld_button) == (ldBase_t *)ld_nested_list);

    assert(picoui_list_set_item_widget(list, -1, &nested_list->widget) == -1);
    assert(picoui_list_set_item_widget(list, 2, &nested_list->widget) == -1);
    assert(picoui_list_set_item_widget(list, 1, 0) == -1);

    picoui_app_destroy(app);
}

static void test_list_set_item_widget_rejects_corrupted_binding_without_reparenting(
    struct picoui_window *win)
{
    struct picoui_app *app;
    struct picoui_window *owned_win;
    struct picoui_widget *parent;
    struct picoui_list *list;
    struct picoui_list *nested_list;
    struct picoui_backend_widget *list_backend;
    struct picoui_backend_widget *nested_list_backend;
    struct picoui_backend_widget *window_backend;
    ldList_t *ld_list;
    ldList_t *ld_nested_list;
    enum picoui_backend_widget_kind original_kind;

    (void)win;
    app = picoui_app_create();
    owned_win = picoui_window_create(app, "list_item_widget_binding_guard_root");
    assert(app != 0);
    assert(owned_win != 0);
    parent = (struct picoui_widget *)owned_win;
    list = picoui_list_create(parent, "list_item_widget_binding_guard");
    nested_list = picoui_list_create(parent, "list_item_widget_binding_guard_nested");
    assert(list != 0);
    assert(nested_list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);

    list_backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    nested_list_backend = (struct picoui_backend_widget *)nested_list->widget.backend_widget;
    window_backend = (struct picoui_backend_widget *)owned_win->widget.backend_widget;
    assert(list_backend != 0);
    assert(nested_list_backend != 0);
    assert(window_backend != 0);
    ld_list = (ldList_t *)list_backend->ld_widget;
    ld_nested_list = (ldList_t *)nested_list_backend->ld_widget;
    assert(ld_list != 0);
    assert(ld_nested_list != 0);

    original_kind = list_backend->kind;
    list_backend->kind = PICOUI_BACKEND_WIDGET_GRAPH;

    assert(picoui_list_set_item_widget(list, 0, &nested_list->widget) == -1);
    assert(nested_list_backend->parent == window_backend);
    assert(nested_list_backend->owner == app);
    assert(nested_list_backend->root == window_backend);
    assert(list_backend->first_child == 0);
    assert(ldBaseGetParent((ldBase_t *)ld_nested_list) == (ldBase_t *)window_backend->ld_widget);
    assert(ld_list->itemCount == 1);

    list_backend->kind = original_kind;
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

static void test_list_selected_index_getter_rejects_corrupted_native_truth_without_cache_pollution(
    struct picoui_window *win)
{
    struct picoui_widget *parent = (struct picoui_widget *)win;
    struct picoui_list *list = picoui_list_create(parent, "list_selected_binding_guard");
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    assert(list != 0);
    assert(picoui_list_add_item(list, "item_wifi", "Wi-Fi") == 0);
    assert(picoui_list_add_item(list, "item_bluetooth", "Bluetooth") == 0);
    assert(picoui_list_set_selected_index(list, 1) == 0);

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    assert(backend != 0);
    ld_list = (ldList_t *)backend->ld_widget;
    assert(ld_list != 0);

    ldListSetSelectItem(ld_list, 9);
    assert(picoui_list_get_selected_index(list) == 1);
    assert(list->selected_index == 1);
    assert(backend->value == 1);
    assert(ldListGetSelectItem(ld_list) == 9);
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

int main(int argc, char **argv)
{
    (void)argc;
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win = picoui_window_create(app, "root");

    test_self_binary_path = argv[0];
    assert(app != 0);
    assert(win != 0);

    test_create_and_props(win);
    test_list_legacy_backend_constructor_is_disabled(win);
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

    picoui_app_destroy(app);
    return 0;
}
