#include "internal/runtime_internal_legacy_api.h"
#include "widgets/button.h"
#include "widgets/window.h"
#include "internal.h"
#include "tinyui.h"

#include <assert.h>

/* Local adapters for pre-v2.3 2-arg create signatures used by this harness. */
static struct tinyui_window *test_window_create(struct tinyui_app *app, const char *id)
{
    (void)app;
    (void)id;
    return (struct tinyui_window *)(void *)tinyui_screen_create();
}

static struct tinyui_button *test_button_create(void *parent, const char *id)
{
    (void)id;
    return (struct tinyui_button *)(void *)tinyui_button_create((tinyui_obj_t *)parent);
}

static void test_app_set_window_switches_active_root_and_focus_scope(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_button *button_a;
    struct tinyui_button *button_b;
    struct tinyui_widget *backend_a;
    struct tinyui_widget *backend_b;

    assert(tinyui_init() == TINYUI_OK);
    app = tinyui_runtime_internal_app_current();
    assert(app != 0);
    win_a = test_window_create(app, "win_a");
    win_b = test_window_create(app, "win_b");
    assert(win_a != 0);
    assert(win_b != 0);

    button_a = test_button_create(win_a, "button_a");
    button_b = test_button_create(win_b, "button_b");
    assert(button_a != 0);
    assert(button_b != 0);
    assert(tinyui_runtime_internal_widget_set_selectable((struct tinyui_widget *)button_a, 1) == 0);
    assert(tinyui_runtime_internal_widget_set_selectable((struct tinyui_widget *)button_b, 1) == 0);

    backend_a = &win_a->widget;
    backend_b = &win_b->widget;
    assert(backend_a->ld_widget != 0);
    assert(backend_b->ld_widget != 0);

    assert(tinyui_runtime_internal_app_set_window(app, win_a) == 0);
    assert(app->root_window == win_a);
    assert(tinyui_focus_reset(app) == 0);
    assert(app->focus_owner == 0);
    assert(tinyui_runtime_internal_widget_claim_focus((struct tinyui_widget *)win_a) == 0);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_ENTER) == 0);
    assert(app->focus_owner == &button_a->widget);

    assert(tinyui_runtime_internal_app_set_window(app, win_b) == 0);
    assert(app->root_window == win_b);
    assert(app->focus_owner == 0);
    assert(tinyui_focus_reset(app) == 0);
    assert(tinyui_runtime_internal_widget_claim_focus((struct tinyui_widget *)win_b) == 0);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_ENTER) == 0);
    assert(app->focus_owner == &button_b->widget);

    assert(tinyui_runtime_internal_app_set_window(0, win_a) == -1);
    assert(tinyui_runtime_internal_app_set_window(app, 0) == -1);

    tinyui_deinit();
}

static void test_app_switch_window_persists_switch_metadata_contract(void)
{
    struct tinyui_app *app;
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_app *app_state;

    assert(tinyui_init() == TINYUI_OK);
    app = tinyui_runtime_internal_app_current();
    assert(app != 0);
    win_a = test_window_create(app, "win_switch_a");
    win_b = test_window_create(app, "win_switch_b");
    assert(win_a != 0);
    assert(win_b != 0);

    app_state = app;
    assert(app_state != 0);

    assert(tinyui_runtime_internal_app_switch_window(app, win_a, 0, 0) == 0);
    assert(app->root_window == win_a);

    assert(tinyui_runtime_internal_app_switch_window(app, win_b, 2, 180) == 0);
    assert(app->root_window == win_b);

    assert(tinyui_runtime_internal_app_switch_window(0, win_b, 0, 0) == -1);
    assert(tinyui_runtime_internal_app_switch_window(app, 0, 0, 0) == -1);

    tinyui_deinit();
}

int main(void)
{
    test_app_set_window_switches_active_root_and_focus_scope();
    test_app_switch_window_persists_switch_metadata_contract();
    return 0;
}
