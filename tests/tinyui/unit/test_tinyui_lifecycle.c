#include <assert.h>
#include <string.h>
#include "internal.h"
#include "tinyui.h"
#include "window.h"
#include "keyboard.h"
#include "list.h"
#include "../../../src/gui/ldBase.h"

static void test_registry_register_lookup_unregister(void)
{
    struct tinyui_app app;
    struct tinyui_widget a, b;
    memset(&app, 0, sizeof(app));
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.ld_name_id = 1; b.ld_name_id = 2;

    tinyui_app_register_host(&app, &a);
    tinyui_app_register_host(&app, &b);
    assert(tinyui_app_lookup_host(&app, 1) == &a);
    assert(tinyui_app_lookup_host(&app, 2) == &b);
    assert(tinyui_app_lookup_host(&app, 99) == 0);

    tinyui_app_unregister_host(&app, &a);
    assert(tinyui_app_lookup_host(&app, 1) == 0);
    assert(tinyui_app_lookup_host(&app, 2) == &b);

    tinyui_app_unregister_host(&app, &b);
    assert(app.host_list_head == 0);
}

/* ---- P2 helpers ---- */
static struct tinyui_window *p2_make_window(struct tinyui_app **out_app)
{
    struct tinyui_app *app = tinyui_app_create();
    assert(app != 0);
    struct tinyui_window *win = tinyui_window_create(app, "root");
    assert(win != 0);
    *out_app = app;
    return win;
}

static void test_destroy_leaf_removes_from_tree(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);

    struct tinyui_label *label = tinyui_label_create(win, "leaf");
    assert(label != 0);
    struct tinyui_widget *w = &label->widget;
    uint16_t id = w->ld_name_id;

    assert(tinyui_app_lookup_host(app, id) == w);

    int ret = tinyui_widget_destroy(w);
    assert(ret == 0);

    assert(tinyui_app_lookup_host(app, id) == 0);

    tinyui_app_destroy(app);
}

static void test_shutdown_does_not_leak(void)
{
    struct tinyui_app *app = tinyui_app_create();
    assert(app != 0);
    struct tinyui_window *win = tinyui_window_create(app, "root");
    assert(win != 0);
    struct tinyui_label *l1 = tinyui_label_create(win, "a");
    struct tinyui_label *l2 = tinyui_label_create(win, "b");
    (void)l1; (void)l2;
    tinyui_app_destroy(app);
}

static void test_destroy_container_reclaims_subtree(void)
{
    struct tinyui_app *app;
    struct tinyui_window *root = p2_make_window(&app);
    struct tinyui_window *child = tinyui_window_create_child(root, "child_panel");
    struct tinyui_label *l1;
    struct tinyui_label *l2;
    uint16_t cid, id1, id2;

    assert(child != 0);
    l1 = tinyui_label_create(child, "c1");
    l2 = tinyui_label_create(child, "c2");
    assert(l1 != 0);
    assert(l2 != 0);

    cid = child->widget.ld_name_id;
    id1 = l1->widget.ld_name_id;
    id2 = l2->widget.ld_name_id;

    assert(tinyui_widget_destroy(&child->widget) == 0);
    assert(tinyui_app_lookup_host(app, cid) == 0);
    assert(tinyui_app_lookup_host(app, id1) == 0);
    assert(tinyui_app_lookup_host(app, id2) == 0);

    tinyui_app_destroy(app);
}

static void test_keyboard_host_cleanup_frees_layout(void)
{
    static const struct tinyui_keyboard_button buttons[] = {
        {0, 0, 30, 30, "A", 'A', 0xFFFFFF, 0x888888},
        {30, 0, 30, 30, "B", 'B', 0xFFFFFF, 0x888888},
    };
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    struct tinyui_keyboard *kbd = tinyui_keyboard_create(win, "kbd");
    assert(kbd != 0);
    assert(tinyui_keyboard_set_buttons(kbd, buttons, 2) == 0);
    /* host_cleanup must free the dynamic layout before ld depose */
    assert(tinyui_widget_destroy(&kbd->widget) == 0);
    tinyui_app_destroy(app);
}

static void test_composite_list_item_no_pinfo_clash(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    struct tinyui_list *list = tinyui_list_create(win, "list");
    struct tinyui_button *item = tinyui_button_create(win, "item_btn");
    assert(list != 0);
    assert(item != 0);
    /* Add a default text item so index 0 is valid */
    assert(tinyui_list_add_item(list, "item0", "Item 0") == 0);
    /* Composite item: pInfo used by LingDongGUI for coords; TinyUI uses nameId */
    assert(tinyui_list_set_item_widget(list, 0, &item->widget) == 0);
    /* nameId reverse-lookup must still work (pInfo no longer touched by TinyUI) */
    assert(tinyui_widget_from_ld(item->widget.ld_widget) == &item->widget);
    /* Destroy list (container with item child) without crash or double-free */
    assert(tinyui_widget_destroy(&list->widget) == 0);
    tinyui_app_destroy(app);
}

static void test_name_id_reused_after_destroy(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win = p2_make_window(&app);
    struct tinyui_label *a = tinyui_label_create(win, "a");
    uint16_t id_a;
    struct tinyui_label *b;

    assert(a != 0);
    id_a = a->widget.ld_name_id;
    assert(tinyui_widget_destroy(&a->widget) == 0);

    b = tinyui_label_create(win, "b");
    assert(b != 0);
    assert(b->widget.ld_name_id == id_a); /* id was returned to freelist and reused */

    tinyui_app_destroy(app);
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
    return 0;
}
