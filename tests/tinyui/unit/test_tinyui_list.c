/*
 * TinyUI list unit tests — M3 Task 3 L3/L4 harness.
 *
 * Validates items/selection/capacity against real ldList_t, shared base
 * aliases, item-widget reparent, and VALUE_CHANGED from native
 * SIGNAL_CLICKED_ITEM via the unified event pool.
 */

#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldList.h"
#include "../../../src/misc/ldMsg.h"
#include "internal.h"
#include "widgets/button.h"
#include "widgets/list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int g_value_changed_count;
static int32_t g_last_value;
static tinyui_obj_t *g_last_target;
static void *g_last_user_data;
static int g_legacy_selected_count;
static int g_legacy_selected_index;

static void reset_event_fixture(void)
{
    g_value_changed_count = 0;
    g_last_value = -999;
    g_last_target = 0;
    g_last_user_data = 0;
    g_legacy_selected_count = 0;
    g_legacy_selected_index = -999;
}

static void on_value_changed(const tinyui_event_t *event)
{
    assert(event != 0);
    g_value_changed_count += 1;
    g_last_value = event->data.value;
    g_last_target = event->target;
    g_last_user_data = event->user_data;
    assert(event->code == TINYUI_EVENT_VALUE_CHANGED);
}

static void on_legacy_selected(tinyui_obj_t *list, int index, void *user_data)
{
    (void)list;
    g_legacy_selected_count += 1;
    g_legacy_selected_index = index;
    g_last_user_data = user_data;
}

static void inject_list_clicked_item(tinyui_obj_t *list_obj, int index)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)(void *)list_obj;

    assert(widget != 0);
    assert(tinyui_runtime_internal_widget_dispatch_native_signal(
               widget, SIGNAL_CLICKED_ITEM, (uint64_t)(uint32_t)index) == 0);
}

static ldList_t *list_ld(tinyui_obj_t *list)
{
    struct tinyui_widget *backend = (struct tinyui_widget *)(void *)list;
    assert(backend != 0);
    assert(backend->ld_widget != 0);
    return (ldList_t *)backend->ld_widget;
}

static void test_create_and_props(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    tinyui_list_props_t props;
    tinyui_obj_t *with_props;
    struct tinyui_widget *backend;
    ldList_t *ld_list;
    int cookie = 7;

    assert(list != 0);
    backend = (struct tinyui_widget *)(void *)list;
    assert(backend->kind == TINYUI_BACKEND_WIDGET_LIST);
    assert(backend->ld_widget != 0);
    assert(backend->ld_name_id != 0);
    assert(backend->ld_event_bridge_scene != 0);
    assert(backend->ld_event_bridge_sender == backend->ld_widget);
    ld_list = list_ld(list);
    assert(ld_list->itemCount == 0);
    assert(tinyui_list_get_selected_index(list) == -1);

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LIST_FIELD_USER_DATA | TINYUI_LIST_FIELD_STYLE_CLASS;
    props.user_data = &cookie;
    props.style_class = "list-style";
    with_props = tinyui_list_create_with_props(root, &props);
    assert(with_props != 0);
    backend = (struct tinyui_widget *)(void *)with_props;
    assert(backend->user_data == &cookie);
    assert(backend->style_class != 0);
    assert(strcmp(backend->style_class, "list-style") == 0);
}

static void test_items_selection_and_callback_contract(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldList_t *ld_list;
    tinyui_event_handle_t handle = 0;
    int i;

    assert(list != 0);
    ld_list = list_ld(list);
    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(list,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   (void *)0x11,
                                   &handle) == TINYUI_OK);
    tinyui_list_set_on_selected(list, on_legacy_selected, (void *)0x22);

    assert(tinyui_list_add_item(list, "a", "Alpha") == 0);
    assert(tinyui_list_add_item(list, "b", "Beta") == 0);
    assert(tinyui_list_add_item(list, "c", "Gamma") == 0);
    assert(ld_list->itemCount == 3);
    assert(ld_list->ppItemStrGroup != 0);
    assert(strcmp((const char *)ld_list->ppItemStrGroup[0], "Alpha") == 0);
    assert(strcmp((const char *)ld_list->ppItemStrGroup[1], "Beta") == 0);
    assert(strcmp((const char *)ld_list->ppItemStrGroup[2], "Gamma") == 0);

    assert(tinyui_list_set_selected_index(list, 1) == 0);
    assert(tinyui_list_get_selected_index(list) == 1);
    assert(ldListGetSelectItem(ld_list) == 1);
    assert(g_value_changed_count == 0);
    assert(g_legacy_selected_count == 0);

    inject_list_clicked_item(list, 2);
    assert(tinyui_list_get_selected_index(list) == 2);
    assert(ldListGetSelectItem(ld_list) == 2);
    assert(g_value_changed_count == 1);
    assert(g_last_value == 2);
    assert(g_last_target == list);
    assert(g_last_user_data == (void *)0x11);
    assert(g_legacy_selected_count == 1);
    assert(g_legacy_selected_index == 2);

    /* same index is noop for events */
    inject_list_clicked_item(list, 2);
    assert(g_value_changed_count == 1);
    assert(g_legacy_selected_count == 1);

    /* capacity+1 must fail with CAPACITY and not grow LD buffer.
     * LD stores pointers; keep stable storage for the rest of the test. */
    {
        static char ids[TINYUI_LIST_MAX_ITEMS][8];
        static char texts[TINYUI_LIST_MAX_ITEMS][16];
        for (i = 3; i < TINYUI_LIST_MAX_ITEMS; ++i) {
            snprintf(ids[i], sizeof(ids[i]), "i%d", i);
            snprintf(texts[i], sizeof(texts[i]), "Item%d", i);
            assert(tinyui_list_add_item(list, ids[i], texts[i]) == 0);
        }
    }
    assert(ld_list->itemCount == TINYUI_LIST_MAX_ITEMS);
    assert(tinyui_list_add_item(list, "overflow", "Overflow") == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_CAPACITY);
    assert(ld_list->itemCount == TINYUI_LIST_MAX_ITEMS);

    assert(tinyui_list_set_selected_index(list, TINYUI_LIST_MAX_ITEMS) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
    assert(tinyui_list_set_selected_index(list, -1) == -1);
    assert(tinyui_last_result() == TINYUI_ERROR_OUT_OF_RANGE);
}

static void test_selected_index_getter_resynchronizes_internal_and_backend_cache_from_native_truth(
    tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldList_t *ld_list;
    struct tinyui_list *wrapper;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "0", "zero") == 0);
    assert(tinyui_list_add_item(list, "1", "one") == 0);
    assert(tinyui_list_add_item(list, "2", "two") == 0);
    ld_list = list_ld(list);
    wrapper = (struct tinyui_list *)(void *)list;

    assert(tinyui_list_set_selected_index(list, 0) == 0);
    ldListSetSelectItem(ld_list, 2);
    assert(wrapper->selected_index == 0);
    assert(tinyui_list_get_selected_index(list) == 2);
    assert(wrapper->selected_index == 2);
    assert(wrapper->widget.value == 2);
}

static void test_list_item_marker_is_support_not_reject(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldList_t *ld_list;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "m0", "M0") == 0);
    assert(tinyui_list_add_item(list, "m1", "M1") == 0);
    ld_list = list_ld(list);
    assert(tinyui_list_set_selected_index(list, 1) == 0);
    assert(ldListGetSelectItem(ld_list) == 1);
    assert(tinyui_list_get_selected_index(list) == 1);
}

static void test_list_native_item_height_padding_margin_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldList_t *ld_list;

    assert(list != 0);
    ld_list = list_ld(list);
    assert(tinyui_list_set_item_height(list, 28) == 0);
    assert(ld_list->itemHeight == 28);
    assert(tinyui_list_set_padding_group(list, 1, 2, 3, 4) == 0);
    assert(ld_list->padding.top == 1);
    assert(ld_list->padding.bottom == 2);
    assert(ld_list->padding.left == 3);
    assert(ld_list->padding.right == 4);
    assert(tinyui_list_set_margin_group(list, 5, 6, 7, 8) == 0);
    assert(ld_list->margin.top == 5);
    assert(ld_list->margin.bottom == 6);
    assert(ld_list->margin.left == 7);
    assert(ld_list->margin.right == 8);

    assert(tinyui_list_set_item_height(list, 0) == -1);
    assert(tinyui_list_set_item_height(list, 256) == -1);
}

static void test_list_native_color_and_align_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldList_t *ld_list;

    assert(list != 0);
    ld_list = list_ld(list);
    assert(tinyui_list_set_text_color(list, 0x112233) == 0);
    assert(ld_list->textColor == (ldColor)tinyui_rgb_to_ld_color(0x112233));
    assert(tinyui_list_set_bg_color(list, 0x445566) == 0);
    assert(ld_list->bgColor == (ldColor)tinyui_rgb_to_ld_color(0x445566));
    assert(tinyui_list_set_select_color(list, 0x778899) == 0);
    assert(ld_list->selectColor == (ldColor)tinyui_rgb_to_ld_color(0x778899));
    assert(tinyui_list_set_align(list, TINYUI_ALIGN_CENTER) == 0);
    assert(ld_list->tAlign == (arm_2d_align_t)tinyui_align_to_arm2d(TINYUI_ALIGN_CENTER));
    assert(tinyui_list_set_align(list, TINYUI_ALIGN_STRETCH) == -1);
}

static void test_list_native_item_widget_round_trip(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    tinyui_obj_t *btn = tinyui_button_create(root);
    ldList_t *ld_list;
    ldBase_t *btn_ld;
    ldBase_t *list_ld_base;

    assert(list != 0);
    assert(btn != 0);
    assert(tinyui_list_add_item(list, "w0", "Widget0") == 0);
    assert(tinyui_list_add_item(list, "w1", "Widget1") == 0);
    ld_list = list_ld(list);
    btn_ld = (ldBase_t *)((struct tinyui_widget *)(void *)btn)->ld_widget;
    list_ld_base = (ldBase_t *)ld_list;
    assert(tinyui_list_set_item_widget(list, 1, btn) == 0);
    assert(ldBaseGetParent(btn_ld) == list_ld_base);
    assert(tinyui_list_set_item_widget(list, 2, btn) == -1);
    assert(tinyui_list_set_item_widget(list, -1, btn) == -1);
    assert(tinyui_list_set_item_widget(list, 0, 0) == -1);
}

static void test_enabled_contract_and_native_selected_bridge(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);

    assert(list != 0);
    assert(tinyui_list_add_item(list, "e0", "E0") == 0);
    assert(tinyui_list_add_item(list, "e1", "E1") == 0);

    assert(tinyui_obj_set_selectable(list, 1) == TINYUI_OK);
    assert(tinyui_obj_set_selected(list, 1) == TINYUI_OK);
    assert(tinyui_list_set_selected_index(list, 1) == 0);
    assert(tinyui_list_get_selected_index(list) == 1);
}

static void test_hidden_or_disabled_list_releases_focus_and_rejects_native_selection(
    tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldList_t *ld_list;
    tinyui_event_handle_t handle = 0;

    assert(list != 0);
    assert(tinyui_list_add_item(list, "h0", "H0") == 0);
    assert(tinyui_list_add_item(list, "h1", "H1") == 0);
    ld_list = list_ld(list);
    assert(tinyui_list_set_selected_index(list, 0) == 0);
    reset_event_fixture();
    assert(tinyui_obj_add_event_cb(list,
                                   TINYUI_EVENT_MASK(TINYUI_EVENT_VALUE_CHANGED),
                                   on_value_changed,
                                   0,
                                   &handle) == TINYUI_OK);

    assert(tinyui_obj_set_visible(list, 0) == TINYUI_OK);
    inject_list_clicked_item(list, 1);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(ldListGetSelectItem(ld_list) == 0);
    assert(g_value_changed_count == 0);

    assert(tinyui_obj_set_visible(list, 1) == TINYUI_OK);
    assert(tinyui_obj_set_enabled(list, 0) == TINYUI_OK);
    inject_list_clicked_item(list, 1);
    assert(tinyui_list_get_selected_index(list) == 0);
    assert(g_value_changed_count == 0);
    assert(tinyui_obj_set_enabled(list, 1) == TINYUI_OK);
}

static void test_widget_native_base_flags_round_trip_to_ldbase(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);
    ldBase_t *ld_base;
    struct tinyui_widget *backend;

    assert(list != 0);
    backend = (struct tinyui_widget *)(void *)list;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(tinyui_obj_set_pos(list, 12, 34) == TINYUI_OK);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 12);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 34);

    assert(tinyui_obj_set_opacity(list, 180) == TINYUI_OK);
    assert(tinyui_obj_set_visible(list, 0) == TINYUI_OK);
    assert(backend->visible == 0);
    assert(tinyui_obj_set_visible(list, 1) == TINYUI_OK);
    assert(backend->visible == 1);

    assert(tinyui_obj_set_selectable(list, 1) == TINYUI_OK);
    assert(tinyui_obj_set_selected(list, 1) == TINYUI_OK);
}

static void test_list_rejects_invalid_inputs(tinyui_obj_t *root)
{
    tinyui_obj_t *list = tinyui_list_create(root);

    assert(tinyui_list_create(0) == 0);
    assert(list != 0);
    assert(tinyui_list_add_item(0, "id", "text") == -1);
    assert(tinyui_list_add_item(list, 0, "text") == -1);
    assert(tinyui_list_add_item(list, "id", 0) == -1);
    assert(tinyui_list_set_selected_index(0, 0) == -1);
    assert(tinyui_list_get_selected_index(0) == -1);
    assert(tinyui_list_set_item_height(0, 10) == -1);
    assert(tinyui_list_set_text_color(0, 0) == -1);
}

int main(void)
{
    tinyui_obj_t *root;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root = tinyui_screen_create();
    assert(root != 0);

    test_create_and_props(root);
    test_items_selection_and_callback_contract(root);
    test_selected_index_getter_resynchronizes_internal_and_backend_cache_from_native_truth(root);
    test_list_item_marker_is_support_not_reject(root);
    test_list_native_item_height_padding_margin_round_trip(root);
    test_list_native_color_and_align_round_trip(root);
    test_list_native_item_widget_round_trip(root);
    test_enabled_contract_and_native_selected_bridge(root);
    test_hidden_or_disabled_list_releases_focus_and_rejects_native_selection(root);
    test_widget_native_base_flags_round_trip_to_ldbase(root);
    test_list_rejects_invalid_inputs(root);

    tinyui_deinit();
    return 0;
}
