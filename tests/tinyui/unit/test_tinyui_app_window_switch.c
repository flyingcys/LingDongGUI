#include "app.h"
#include "button.h"
#include "window.h"
#include "internal.h"

#include <assert.h>

static void test_app_set_window_switches_active_root_and_focus_scope(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win_a;
    struct picoui_window *win_b;
    struct picoui_button *button_a;
    struct picoui_button *button_b;
    struct picoui_backend_widget *backend_a;
    struct picoui_backend_widget *backend_b;

    assert(app != 0);
    win_a = picoui_window_create(app, "win_a");
    win_b = picoui_window_create(app, "win_b");
    assert(win_a != 0);
    assert(win_b != 0);

    button_a = picoui_button_create(win_a, "button_a");
    button_b = picoui_button_create(win_b, "button_b");
    assert(button_a != 0);
    assert(button_b != 0);
    assert(picoui_widget_set_selectable((struct picoui_widget *)button_a, 1) == 0);
    assert(picoui_widget_set_selectable((struct picoui_widget *)button_b, 1) == 0);

    backend_a = (struct picoui_backend_widget *)win_a->widget.backend_widget;
    backend_b = (struct picoui_backend_widget *)win_b->widget.backend_widget;
    assert(backend_a != 0);
    assert(backend_b != 0);

    assert(picoui_app_set_window(app, win_a) == 0);
    assert(app->root_window == win_a);
    assert(picoui_focus_reset(app) == 0);
    assert(app->focus_owner == 0);
    assert(picoui_widget_claim_focus((struct picoui_widget *)win_a) == 0);
    assert(picoui_focus_navigate(app, PICOUI_NATIVE_NAV_ENTER) == 0);
    assert(app->focus_owner == &button_a->widget);

    assert(picoui_app_set_window(app, win_b) == 0);
    assert(app->root_window == win_b);
    assert(app->focus_owner == 0);
    assert(picoui_focus_reset(app) == 0);
    assert(picoui_widget_claim_focus((struct picoui_widget *)win_b) == 0);
    assert(picoui_focus_navigate(app, PICOUI_NATIVE_NAV_ENTER) == 0);
    assert(app->focus_owner == &button_b->widget);

    assert(picoui_app_set_window(0, win_a) == -1);
    assert(picoui_app_set_window(app, 0) == -1);

    picoui_app_destroy(app);
}

static void test_app_switch_window_persists_switch_metadata_contract(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win_a;
    struct picoui_window *win_b;
    struct picoui_backend_app_state *app_state;

    assert(app != 0);
    win_a = picoui_window_create(app, "win_switch_a");
    win_b = picoui_window_create(app, "win_switch_b");
    assert(win_a != 0);
    assert(win_b != 0);

    app_state = (struct picoui_backend_app_state *)app->backend_app;
    assert(app_state != 0);

    assert(picoui_app_switch_window(app, win_a, 0, 0) == 0);
    assert(app->root_window == win_a);

    assert(picoui_app_switch_window(app, win_b, 2, 180) == 0);
    assert(app->root_window == win_b);

    assert(picoui_app_switch_window(0, win_b, 0, 0) == -1);
    assert(picoui_app_switch_window(app, 0, 0, 0) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_app_set_window_switches_active_root_and_focus_scope();
    test_app_switch_window_persists_switch_metadata_contract();
    return 0;
}
