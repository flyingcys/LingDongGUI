#include <assert.h>
#include <string.h>
#include "internal.h"
#include "tinyui.h"
#include "widgets/window.h"
#include "widgets/keyboard.h"
#include "widgets/list.h"
#include "widgets/button.h"
#include "widgets/checkbox.h"
#include "widgets/table.h"
#include "widgets/label.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldButton.h"
#include "../../../src/gui/ldCheckBox.h"
#include "../../../src/gui/ldLabel.h"
#include "../../../src/gui/ldTable.h"
#include "../../../src/misc/xBtnAction.h"
#include "../../../examples/common/demo/widget/fonts/uiFonts.h"

static int g_prepare_native_depose_count = 0;

void tinyui_test_on_prepare_native_depose(const struct tinyui_widget *widget)
{
    (void)widget;
    g_prepare_native_depose_count++;
}

static void test_registry_register_lookup_unregister(void)
{
    struct tinyui_app app;
    struct tinyui_widget a, b;
    memset(&app, 0, sizeof(app));
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.ld_name_id = 1; b.ld_name_id = 2;

    tinyui_runtime_internal_app_register_host(&app, &a);
    tinyui_runtime_internal_app_register_host(&app, &b);
    assert(tinyui_runtime_internal_app_lookup_host(&app, 1) == &a);
    assert(tinyui_runtime_internal_app_lookup_host(&app, 2) == &b);
    assert(tinyui_runtime_internal_app_lookup_host(&app, 99) == 0);

    tinyui_runtime_internal_app_unregister_host(&app, &a);
    assert(tinyui_runtime_internal_app_lookup_host(&app, 1) == 0);
    assert(tinyui_runtime_internal_app_lookup_host(&app, 2) == &b);

    tinyui_runtime_internal_app_unregister_host(&app, &b);
    assert(app.host_list_head == 0);
}

/* ---- P2 helpers ---- */
static void p2_shutdown(struct tinyui_app *app)
{
    (void)app;
    tinyui_deinit();
}

static struct tinyui_window *p2_make_window(struct tinyui_app **out_app)
{
    tinyui_obj_t *root_obj;
    struct tinyui_window *win;
    struct tinyui_app *app;

    tinyui_deinit();
    assert(tinyui_init() == TINYUI_OK);
    root_obj = tinyui_screen_create();
    assert(root_obj != 0);
    win = (struct tinyui_window *)(void *)root_obj;
    app = win->widget.owner;
    assert(app != 0);
    *out_app = app;
    return win;
}

static void test_destroy_leaf_removes_from_tree(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *label_obj;
    struct tinyui_widget *w;
    uint16_t id;

    label_obj = tinyui_label_create((tinyui_obj_t *)win);
    assert(label_obj != 0);
    w = (struct tinyui_widget *)(void *)label_obj;
    id = w->ld_name_id;

    assert(tinyui_runtime_internal_app_lookup_host(app, id) == w);

    assert(tinyui_obj_delete(label_obj) == TINYUI_OK);
    assert(tinyui_runtime_internal_app_lookup_host(app, id) == 0);

    p2_shutdown(app);
}

static void test_shutdown_does_not_leak(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *l1 = tinyui_label_create((tinyui_obj_t *)win);
    tinyui_obj_t *l2 = tinyui_label_create((tinyui_obj_t *)win);
    (void)l1;
    (void)l2;
    p2_shutdown(app);
}

static void test_destroy_container_reclaims_subtree(void)
{
    struct tinyui_app *app;
    struct tinyui_window *root = p2_make_window(&app);
    tinyui_obj_t *child_obj;
    tinyui_obj_t *l1;
    tinyui_obj_t *l2;
    uint16_t cid, id1, id2;
    struct tinyui_widget *child_w;
    struct tinyui_widget *l1_w;
    struct tinyui_widget *l2_w;

    child_obj = tinyui_window_create((tinyui_obj_t *)root);
    assert(child_obj != 0);
    child_w = (struct tinyui_widget *)(void *)child_obj;

    l1 = tinyui_label_create(child_obj);
    l2 = tinyui_label_create(child_obj);
    assert(l1 != 0);
    assert(l2 != 0);
    l1_w = (struct tinyui_widget *)(void *)l1;
    l2_w = (struct tinyui_widget *)(void *)l2;

    cid = child_w->ld_name_id;
    id1 = l1_w->ld_name_id;
    id2 = l2_w->ld_name_id;

    assert(tinyui_obj_delete(child_obj) == TINYUI_OK);
    assert(tinyui_runtime_internal_app_lookup_host(app, cid) == 0);
    assert(tinyui_runtime_internal_app_lookup_host(app, id1) == 0);
    assert(tinyui_runtime_internal_app_lookup_host(app, id2) == 0);

    p2_shutdown(app);
}

static void test_keyboard_host_cleanup_frees_layout(void)
{
    static const struct tinyui_keyboard_button buttons[] = {
        {0, 0, 30, 30, "A", 'A', 0xFFFFFF, 0x888888},
        {30, 0, 30, 30, "B", 'B', 0xFFFFFF, 0x888888},
    };
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *kbd = tinyui_keyboard_create((tinyui_obj_t *)win);

    assert(kbd != 0);
    assert(tinyui_keyboard_set_buttons(kbd, buttons, 2) == 0);
    assert(tinyui_obj_delete(kbd) == TINYUI_OK);
    p2_shutdown(app);
}

static void test_composite_list_item_no_pinfo_clash(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *list = tinyui_list_create((tinyui_obj_t *)win);
    tinyui_obj_t *item = tinyui_button_create((tinyui_obj_t *)win);
    struct tinyui_widget *item_w;

    assert(list != 0);
    assert(item != 0);
    item_w = (struct tinyui_widget *)(void *)item;
    assert(tinyui_list_add_item(list, "item0", "Item 0") == 0);
    assert(tinyui_list_set_item_widget(list, 0, item) == 0);
    assert(tinyui_runtime_internal_widget_from_ld(item_w->ld_widget) == item_w);
    assert(tinyui_obj_delete(list) == TINYUI_OK);
    p2_shutdown(app);
}

static void test_name_id_reused_after_destroy(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *a = tinyui_label_create((tinyui_obj_t *)win);
    uint16_t id_a;
    tinyui_obj_t *b;
    struct tinyui_widget *a_w;
    struct tinyui_widget *b_w;

    assert(a != 0);
    a_w = (struct tinyui_widget *)(void *)a;
    id_a = a_w->ld_name_id;
    assert(tinyui_obj_delete(a) == TINYUI_OK);

    b = tinyui_label_create((tinyui_obj_t *)win);
    assert(b != 0);
    b_w = (struct tinyui_widget *)(void *)b;
    assert(b_w->ld_name_id == id_a);

    p2_shutdown(app);
}

static void test_destroy_button_unregisters_from_xbtn(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *btn = tinyui_button_create((tinyui_obj_t *)win);
    uint16_t id;
    struct tinyui_widget *btn_w;

    assert(btn != 0);
    btn_w = (struct tinyui_widget *)(void *)btn;
    id = btn_w->ld_name_id;

    assert(xBtnGetState(id, BTN_NO_OPERATION) == 1);
    assert(tinyui_obj_delete(btn) == TINYUI_OK);
    assert(xBtnGetState(id, BTN_NO_OPERATION) == 0);

    p2_shutdown(app);
}

static void test_prepare_native_depose_detaches_static_fonts(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *label = tinyui_label_create((tinyui_obj_t *)win);
    tinyui_obj_t *button = tinyui_button_create((tinyui_obj_t *)win);
    tinyui_obj_t *checkbox = tinyui_checkbox_create((tinyui_obj_t *)win);
    tinyui_table_props_t table_props;
    tinyui_obj_t *table;
    ldLabel_t *ld_label;
    ldButton_t *ld_button;
    ldCheckBox_t *ld_checkbox;
    ldTable_t *ld_table;
    ldTableItem_t *item;
    struct tinyui_widget *label_w;
    struct tinyui_widget *button_w;
    struct tinyui_widget *checkbox_w;
    struct tinyui_widget *table_w;

    memset(&table_props, 0, sizeof(table_props));
    table_props.fields = TINYUI_TABLE_FIELD_ROWS | TINYUI_TABLE_FIELD_COLUMNS;
    table_props.rows = 2;
    table_props.columns = 2;
    table = tinyui_table_create_with_props((tinyui_obj_t *)win, &table_props);

    assert(label != 0);
    assert(button != 0);
    assert(checkbox != 0);
    assert(table != 0);

    label_w = (struct tinyui_widget *)(void *)label;
    button_w = (struct tinyui_widget *)(void *)button;
    checkbox_w = (struct tinyui_widget *)(void *)checkbox;
    table_w = (struct tinyui_widget *)(void *)table;

    ld_label = (ldLabel_t *)label_w->ld_widget;
    ld_button = (ldButton_t *)button_w->ld_widget;
    ld_checkbox = (ldCheckBox_t *)checkbox_w->ld_widget;
    ld_table = (ldTable_t *)table_w->ld_widget;
    assert(ld_label != 0);
    assert(ld_button != 0);
    assert(ld_checkbox != 0);
    assert(ld_table != 0);
    assert(tinyui_table_set_excel_type(table) == 0);
    item = &ld_table->ptItemInfo[ld_table->columnCount];

    assert(ld_label->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(ld_button->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(ld_checkbox->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);
    assert(item->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);

    tinyui_runtime_internal_widget_prepare_native_depose(label_w);
    tinyui_runtime_internal_widget_prepare_native_depose(button_w);
    tinyui_runtime_internal_widget_prepare_native_depose(checkbox_w);
    tinyui_runtime_internal_widget_prepare_native_depose(table_w);

    assert(ld_label->ptFont == 0);
    assert(ld_button->ptFont == 0);
    assert(ld_checkbox->ptFont == 0);
    assert(item->ptFont == 0);

    p2_shutdown(app);
}

static void test_prepare_native_depose_keeps_non_static_fonts(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *label = tinyui_label_create((tinyui_obj_t *)win);
    ldLabel_t *ld_label;
    arm_2d_font_t non_static_font;
    struct tinyui_widget *label_w;

    assert(label != 0);
    label_w = (struct tinyui_widget *)(void *)label;
    ld_label = (ldLabel_t *)label_w->ld_widget;
    assert(ld_label != 0);
    memset(&non_static_font, 0, sizeof(non_static_font));
    ld_label->ptFont = &non_static_font;

    tinyui_runtime_internal_widget_prepare_native_depose(label_w);

    assert(ld_label->ptFont == &non_static_font);
    ld_label->ptFont = 0;
    p2_shutdown(app);
}

static void test_app_destroy_prepares_native_depose_for_remaining_hosts(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    tinyui_obj_t *label = tinyui_label_create((tinyui_obj_t *)win);
    ldLabel_t *ld_label;
    int before_count;
    struct tinyui_widget *label_w;

    assert(label != 0);
    label_w = (struct tinyui_widget *)(void *)label;
    ld_label = (ldLabel_t *)label_w->ld_widget;
    assert(ld_label != 0);
    assert(ld_label->ptFont == (arm_2d_font_t *)FONT_ARIAL_12);

    before_count = g_prepare_native_depose_count;
    p2_shutdown(app);

    assert(g_prepare_native_depose_count > before_count);
}

/* M2 Task 2: LD tree is sole ownership truth; ID get/find; delete invalidates host. */
static void test_v23_object_tree_and_id_queries(void)
{
    struct tinyui_app *app;
    struct tinyui_window *root = p2_make_window(&app);
    tinyui_obj_t *root_obj = (tinyui_obj_t *)root;
    tinyui_obj_t *parent_obj;
    tinyui_obj_t *label_obj;
    tinyui_obj_t *first;
    tinyui_obj_t *sibling;
    uint16_t label_id = 0U;
    uint16_t parent_id = 0U;
    uint16_t count = 0U;
    uint16_t root_count_before = 0U;
    ldBase_t *ld_parent;
    ldBase_t *ld_label;
    struct tinyui_widget *parent_w;
    struct tinyui_widget *label_w;

    parent_obj = tinyui_window_create(root_obj);
    assert(parent_obj != 0);
    parent_w = (struct tinyui_widget *)(void *)parent_obj;

    label_obj = tinyui_label_create(parent_obj);
    assert(label_obj != 0);
    label_w = (struct tinyui_widget *)(void *)label_obj;

    assert(tinyui_obj_get_id(label_obj, &label_id) == TINYUI_OK);
    assert(label_id != 0U);
    assert(tinyui_obj_get_id(parent_obj, &parent_id) == TINYUI_OK);
    assert(parent_id != 0U);
    assert(label_id != parent_id);

    assert(tinyui_obj_find_by_id(root_obj, label_id) == label_obj);
    assert(tinyui_obj_find_by_id(root_obj, parent_id) == parent_obj);
    assert(tinyui_obj_find_by_id(root_obj, 0U) == 0);
    assert(tinyui_obj_get_parent(label_obj) == parent_obj);
    assert(tinyui_obj_get_root(label_obj) == root_obj);
    assert(tinyui_obj_get_root(parent_obj) == root_obj);
    assert(tinyui_obj_get_child_count(parent_obj, &count) == TINYUI_OK);
    assert(count == 1U);

    first = tinyui_obj_get_first_child(parent_obj);
    assert(first == label_obj);
    sibling = tinyui_obj_get_next_sibling(label_obj);
    assert(sibling == 0);

    /* Tree order must come from real LD nodes, not a second TinyUI ownership tree. */
    ld_parent = (ldBase_t *)parent_w->ld_widget;
    ld_label = (ldBase_t *)label_w->ld_widget;
    assert(ld_parent != 0);
    assert(ld_label != 0);
    assert(ldBaseGetParent(ld_label) == ld_parent);
    assert(ldBaseGetChildList(ld_parent) == ld_label);
    assert(ldBaseGetChildCount(ld_parent) == 1U);
    assert(tinyui_runtime_internal_app_lookup_host(app, label_id) == label_w);

    assert(tinyui_obj_get_child_count(root_obj, &root_count_before) == TINYUI_OK);
    assert(root_count_before >= 1U);

    assert(tinyui_obj_delete(parent_obj) == TINYUI_OK);
    assert(tinyui_runtime_internal_app_lookup_host(app, parent_id) == 0);
    assert(tinyui_runtime_internal_app_lookup_host(app, label_id) == 0);
    assert(tinyui_obj_find_by_id(root_obj, parent_id) == 0);
    assert(tinyui_obj_find_by_id(root_obj, label_id) == 0);

    p2_shutdown(app);
}

static void test_v23_explicit_id_conflict_keeps_backend_count(void)
{
    struct tinyui_app *app;
    struct tinyui_window *root = p2_make_window(&app);
    tinyui_obj_t *root_obj = (tinyui_obj_t *)root;
    tinyui_label_props_t props;
    tinyui_obj_t *first;
    tinyui_obj_t *second;
    uint16_t count_before = 0U;
    uint16_t count_after = 0U;
    uint16_t first_id = 0U;
    struct tinyui_widget *root_w = &root->widget;

    memset(&props, 0, sizeof(props));
    props.fields = TINYUI_LABEL_FIELD_ID;
    props.id = 77U;

    first = tinyui_label_create_with_props(root_obj, &props);
    assert(first != 0);
    assert(tinyui_obj_get_id(first, &first_id) == TINYUI_OK);
    assert(first_id == 77U);
    assert(tinyui_obj_find_by_id(root_obj, 77U) == first);

    assert(tinyui_obj_get_child_count(root_obj, &count_before) == TINYUI_OK);

    second = tinyui_label_create_with_props(root_obj, &props);
    assert(second == 0);
    assert(tinyui_obj_get_child_count(root_obj, &count_after) == TINYUI_OK);
    assert(count_after == count_before);
    assert(tinyui_obj_find_by_id(root_obj, 77U) == first);
    assert(ldBaseGetChildCount((ldBase_t *)root_w->ld_widget) == (uint16_t)count_before);

    p2_shutdown(app);
}

/* M2 Task 2 review fix: mid-process delete only marks; process flushes destroy. */
static void test_v23_deferred_delete_flushed_by_process(void)
{
    struct tinyui_app *app;
    struct tinyui_window *root = p2_make_window(&app);
    tinyui_obj_t *root_obj = (tinyui_obj_t *)root;
    tinyui_obj_t *label_obj;
    tinyui_obj_t *other_obj;
    struct tinyui_widget *label_w;
    struct tinyui_runtime_state *rt;
    uint16_t label_id = 0U;
    uint32_t next_ms = 0U;

    label_obj = tinyui_label_create(root_obj);
    other_obj = tinyui_label_create(root_obj);
    assert(label_obj != 0);
    assert(other_obj != 0);
    label_w = (struct tinyui_widget *)(void *)label_obj;
    assert(tinyui_obj_get_id(label_obj, &label_id) == TINYUI_OK);
    assert(label_id != 0U);
    assert(tinyui_runtime_internal_app_lookup_host(app, label_id) == label_w);

    rt = tinyui_runtime_state_get();
    assert(rt != 0);

    /* Simulate mid-dispatch / mid-process: delete only marks deferred. */
    rt->processing = true;
    assert(tinyui_obj_delete(label_obj) == TINYUI_OK);
    assert(label_w->deleting == 1);
    assert(rt->delete_pending == 1);
    assert(rt->delete_target == label_obj);
    assert(tinyui_runtime_internal_app_lookup_host(app, label_id) == label_w);
    assert(tinyui_obj_find_by_id(root_obj, label_id) == label_obj);

    /* Repeat delete / second delete while pending must not destroy immediately. */
    assert(tinyui_obj_delete(label_obj) == TINYUI_ERROR_INVALID_STATE);
    assert(tinyui_obj_delete(other_obj) == TINYUI_ERROR_INVALID_STATE);
    assert(tinyui_runtime_internal_app_lookup_host(app, label_id) == label_w);

    /* Setters blocked after deleting mark. */
    assert(tinyui_runtime_internal_widget_set_text(label_w, "blocked") == -1);

    /* Allow process entry; pending remains until process end flushes destroy. */
    rt->processing = false;
    assert(tinyui_process(&next_ms) == TINYUI_OK);
    assert(rt->delete_pending == 0);
    assert(rt->delete_target == 0);
    assert(tinyui_runtime_internal_app_lookup_host(app, label_id) == 0);
    assert(tinyui_obj_find_by_id(root_obj, label_id) == 0);

    p2_shutdown(app);
}

int main(void)
{
    test_registry_register_lookup_unregister();
    test_destroy_leaf_removes_from_tree();
    test_shutdown_does_not_leak();
    test_destroy_container_reclaims_subtree();
    test_keyboard_host_cleanup_frees_layout();
    test_composite_list_item_no_pinfo_clash();
    test_name_id_reused_after_destroy();
    test_destroy_button_unregisters_from_xbtn();
    test_prepare_native_depose_detaches_static_fonts();
    test_prepare_native_depose_keeps_non_static_fonts();
    test_app_destroy_prepares_native_depose_for_remaining_hosts();
    test_v23_object_tree_and_id_queries();
    test_v23_explicit_id_conflict_keeps_backend_count();
    test_v23_deferred_delete_flushed_by_process();
    return 0;
}
