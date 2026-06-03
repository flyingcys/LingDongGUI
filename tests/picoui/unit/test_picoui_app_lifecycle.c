#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

// Test 1: create + destroy bare app
static void test_app_create_and_destroy(void)
{
    struct picoui_app *app = picoui_app_create();
    assert(app != 0);
    picoui_app_destroy(app);
    // no crash = pass
}

// Test 2: app with multiple windows
static void test_app_multiple_windows(void)
{
    struct picoui_app *app = picoui_app_create();
    assert(app != 0);
    struct picoui_window *w1 = picoui_window_create(app, "win1");
    struct picoui_window *w2 = picoui_window_create(app, "win2");
    assert(w1 != 0);
    assert(w2 != 0);
    // verify windows have different backend widgets
    assert(w1->widget.backend_widget != w2->widget.backend_widget);
    picoui_app_destroy(app);
}

// Test 3: create window with null app
static void test_app_rejects_null(void)
{
    assert(picoui_window_create(0, "x") == 0);
    assert(picoui_app_set_theme(0, 0) == -1);
}

int main(void)
{
    test_app_create_and_destroy();
    test_app_multiple_windows();
    test_app_rejects_null();
    return 0;
}
