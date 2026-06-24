#include "core/app.h"
#include "widgets/button.h"
#include "widgets/window.h"
#include "internal.h"

#include <assert.h>

static void test_app_set_window_switches_active_root_and_focus_scope(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_button *button_a;
    struct tinyui_button *button_b;
    struct tinyui_widget *backend_a;
    struct tinyui_widget *backend_b;

    assert(app != 0);
    win_a = tinyui_window_create(app, "win_a");
    win_b = tinyui_window_create(app, "win_b");
    assert(win_a != 0);
    assert(win_b != 0);

    button_a = tinyui_button_create(win_a, "button_a");
    button_b = tinyui_button_create(win_b, "button_b");
    assert(button_a != 0);
    assert(button_b != 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)button_a, 1) == 0);
    assert(tinyui_widget_set_selectable((struct tinyui_widget *)button_b, 1) == 0);

    backend_a = &win_a->widget;
    backend_b = &win_b->widget;
    assert(backend_a->ld_widget != 0);
    assert(backend_b->ld_widget != 0);

    assert(tinyui_app_set_window(app, win_a) == 0);
    assert(app->root_window == win_a);
    assert(tinyui_focus_reset(app) == 0);
    assert(app->focus_owner == 0);
    assert(tinyui_widget_claim_focus((struct tinyui_widget *)win_a) == 0);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_ENTER) == 0);
    assert(app->focus_owner == &button_a->widget);

    assert(tinyui_app_set_window(app, win_b) == 0);
    assert(app->root_window == win_b);
    assert(app->focus_owner == 0);
    assert(tinyui_focus_reset(app) == 0);
    assert(tinyui_widget_claim_focus((struct tinyui_widget *)win_b) == 0);
    assert(tinyui_focus_navigate(app, TINYUI_NATIVE_NAV_ENTER) == 0);
    assert(app->focus_owner == &button_b->widget);

    assert(tinyui_app_set_window(0, win_a) == -1);
    assert(tinyui_app_set_window(app, 0) == -1);

    tinyui_app_destroy(app);
}

static void test_app_switch_window_persists_switch_metadata_contract(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win_a;
    struct tinyui_window *win_b;
    struct tinyui_app *app_state;

    assert(app != 0);
    win_a = tinyui_window_create(app, "win_switch_a");
    win_b = tinyui_window_create(app, "win_switch_b");
    assert(win_a != 0);
    assert(win_b != 0);

    app_state = app;
    assert(app_state != 0);

    assert(tinyui_app_switch_window(app, win_a, 0, 0) == 0);
    assert(app->root_window == win_a);

    assert(tinyui_app_switch_window(app, win_b, 2, 180) == 0);
    assert(app->root_window == win_b);

    assert(tinyui_app_switch_window(0, win_b, 0, 0) == -1);
    assert(tinyui_app_switch_window(app, 0, 0, 0) == -1);

    tinyui_app_destroy(app);
}

int main(void)
{
    test_app_set_window_switches_active_root_and_focus_scope();
    test_app_switch_window_persists_switch_metadata_contract();
    return 0;
}
