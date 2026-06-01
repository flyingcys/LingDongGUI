#include "picoui/app.h"
#include "picoui/progress_bar.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldProgressBar.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static void test_progress_bar_create_and_props(struct picoui_window *win)
{
    int user_cookie = 42;
    struct picoui_progress_bar_props props = {
        .id = "progress_with_props",
        .style_class = "meter",
        .user_data = &user_cookie,
        .percent = 65,
        .horizontal = 1,
    };
    struct picoui_progress_bar *bar = picoui_progress_bar_create(win, "progress");
    struct picoui_progress_bar *bar_with_props = picoui_progress_bar_create_with_props(win, &props);

    assert(bar != 0);
    assert(bar_with_props != 0);
    assert(picoui_progress_bar_get_percent(bar) == 0);
    assert(picoui_progress_bar_get_percent(bar_with_props) == 65);
    assert(picoui_progress_bar_get_horizontal(bar) == 0);
    assert(picoui_progress_bar_get_horizontal(bar_with_props) == 1);
}

static void test_progress_bar_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_progress_bar *bar = picoui_progress_bar_create(win, "progress_invalid");

    assert(bar != 0);
    assert(picoui_progress_bar_create(0, "progress") == 0);
    assert(picoui_progress_bar_create(win, 0) == 0);
    assert(picoui_progress_bar_create_with_props(0,
                                                 &(struct picoui_progress_bar_props){
                                                     .id = "bad_parent",
                                                     .percent = 0,
                                                 }) == 0);
    assert(picoui_progress_bar_create_with_props(win, 0) == 0);
    assert(picoui_progress_bar_create_with_props(win,
                                                 &(struct picoui_progress_bar_props){
                                                     .percent = 0,
                                                 }) == 0);
    assert(picoui_progress_bar_create_with_props(win,
                                                 &(struct picoui_progress_bar_props){
                                                     .id = "bad_percent_low",
                                                     .percent = -1,
                                                 }) == 0);
    assert(picoui_progress_bar_create_with_props(win,
                                                 &(struct picoui_progress_bar_props){
                                                     .id = "bad_percent_high",
                                                     .percent = 101,
                                                 }) == 0);
    assert(picoui_progress_bar_set_percent(0, 10) == -1);
    assert(picoui_progress_bar_get_percent(0) == -1);
    assert(picoui_progress_bar_set_horizontal(0, 1) == -1);
    assert(picoui_progress_bar_get_horizontal(0) == -1);
    assert(picoui_progress_bar_set_percent(bar, -3) == -1);
    assert(picoui_progress_bar_set_percent(bar, 130) == -1);
    assert(picoui_progress_bar_get_percent(bar) == 0);
}

static void test_progress_bar_percent_bounds(struct picoui_window *win)
{
    struct picoui_progress_bar *bar = picoui_progress_bar_create(win, "progress_bounds");

    assert(bar != 0);
    assert(picoui_progress_bar_set_percent(bar, 0) == 0);
    assert(picoui_progress_bar_get_percent(bar) == 0);
    assert(picoui_progress_bar_set_percent(bar, 100) == 0);
    assert(picoui_progress_bar_get_percent(bar) == 100);
    assert(picoui_progress_bar_set_percent(bar, -1) == -1);
    assert(picoui_progress_bar_get_percent(bar) == 100);
    assert(picoui_progress_bar_set_percent(bar, 101) == -1);
    assert(picoui_progress_bar_get_percent(bar) == 100);
}

static void test_progress_bar_horizontal_state(struct picoui_window *win)
{
    struct picoui_progress_bar *bar = picoui_progress_bar_create(win, "progress_horizontal");

    assert(bar != 0);
    assert(picoui_progress_bar_get_horizontal(bar) == 0);
    assert(picoui_progress_bar_set_horizontal(bar, 1) == 0);
    assert(picoui_progress_bar_get_horizontal(bar) == 1);
    assert(picoui_progress_bar_set_horizontal(bar, 7) == 0);
    assert(picoui_progress_bar_get_horizontal(bar) == 1);
    assert(picoui_progress_bar_set_horizontal(bar, 0) == 0);
    assert(picoui_progress_bar_get_horizontal(bar) == 0);
    assert(picoui_progress_bar_set_horizontal(bar, -5) == 0);
    assert(picoui_progress_bar_get_horizontal(bar) == 1);
}

static void test_progress_bar_release_contract_covers_theme_and_config_boundary(
    struct picoui_window *win)
{
    struct picoui_progress_bar *bar = picoui_progress_bar_create_with_props(
        win,
        &(struct picoui_progress_bar_props){
            .id = "progress_release_ready",
            .style_class = "meter",
            .percent = 40,
            .horizontal = 1,
        });
    struct picoui_backend_widget *backend;
    ldProgressBar_t *ld_progress_bar;

    assert(bar != 0);
    backend = (struct picoui_backend_widget *)bar->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_PROGRESS_BAR);
    assert(backend->style_class == (const char *)"meter");
    ld_progress_bar = (ldProgressBar_t *)backend->ld_widget;
    assert(ld_progress_bar != 0);

    assert(picoui_progress_bar_get_percent(bar) == 40);
    assert(picoui_progress_bar_get_horizontal(bar) == 1);
    assert(ld_progress_bar->permille == 400U);
    assert(ld_progress_bar->isHorizontal == true);
    assert(ld_progress_bar->bgColor != ld_progress_bar->fgColor);
    assert(ld_progress_bar->frameColorSize == 1);
    assert(ld_progress_bar->frameColor != ld_progress_bar->bgColor);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_progress_bar_create_and_props(win);
    test_progress_bar_percent_bounds(win);
    test_progress_bar_horizontal_state(win);
    test_progress_bar_rejects_invalid_inputs(win);
    test_progress_bar_release_contract_covers_theme_and_config_boundary(win);

    picoui_app_destroy(app);
    return 0;
}
