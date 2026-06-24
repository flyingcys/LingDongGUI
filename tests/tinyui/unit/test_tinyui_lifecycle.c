#include <assert.h>
#include <string.h>
#include "internal.h"
#include "tinyui.h"
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

int main(void)
{
    test_registry_register_lookup_unregister();
    test_destroy_leaf_removes_from_tree();
    return 0;
}
