#include <assert.h>
#include <string.h>
#include "internal.h"
#include "tinyui.h"
#include "window.h"
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

int main(void)
{
    test_registry_register_lookup_unregister();
    test_destroy_leaf_removes_from_tree();
    test_shutdown_does_not_leak();
    test_destroy_container_reclaims_subtree();
    return 0;
}
