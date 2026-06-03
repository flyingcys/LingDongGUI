#include "picoui/app.h"
#include "picoui/progress_wheel.h"
#include "picoui/widget.h"
#include "picoui/window.h"
#include "../../../src/gui/ldProgressWheel.h"
#include "backend.h"
#include "internal.h"

#include <assert.h>

static struct picoui_progress_wheel *test_progress_wheel_create_and_props(struct picoui_window *win)
{
    int user_cookie = 9;
    struct picoui_progress_wheel_props props = {
        .id = "wheel_props",
        .style_class = "wheel",
        .user_data = &user_cookie,
        .percent = 72,
    };
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel");
    struct picoui_progress_wheel *with_props =
        picoui_progress_wheel_create_with_props((struct picoui_widget *)win, &props);

    assert(wheel != 0);
    assert(with_props != 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
    assert(picoui_progress_wheel_get_percent(with_props) == props.percent);

    return with_props;
}

static void test_progress_wheel_percent_bounds(struct picoui_window *win)
{
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel_bounds");

    assert(wheel != 0);
    assert(picoui_progress_wheel_set_percent(wheel, 0) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
    assert(picoui_progress_wheel_set_percent(wheel, 100) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
    assert(picoui_progress_wheel_set_percent(wheel, -1) == -1);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
    assert(picoui_progress_wheel_set_percent(wheel, 101) == -1);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
}

static void test_progress_wheel_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_progress_wheel *wheel =
        picoui_progress_wheel_create((struct picoui_widget *)win, "wheel_invalid");

    assert(wheel != 0);
    assert(picoui_progress_wheel_create(0, "wheel") == 0);
    assert(picoui_progress_wheel_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_progress_wheel_create_with_props(0,
                                                   &(struct picoui_progress_wheel_props){
                                                       .id = "bad_parent",
                                                       .percent = 0,
                                                   }) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win,
                                                   &(struct picoui_progress_wheel_props){
                                                       .percent = 0,
                                                   }) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win,
                                                   &(struct picoui_progress_wheel_props){
                                                       .id = "bad_percent_low",
                                                       .percent = -1,
                                                   }) == 0);
    assert(picoui_progress_wheel_create_with_props((struct picoui_widget *)win,
                                                   &(struct picoui_progress_wheel_props){
                                                       .id = "bad_percent_high",
                                                       .percent = 101,
                                                   }) == 0);
    assert(picoui_progress_wheel_set_percent(0, 10) == -1);
    assert(picoui_progress_wheel_get_percent(0) == -1);
    assert(picoui_progress_wheel_set_percent(wheel, -3) == -1);
    assert(picoui_progress_wheel_set_percent(wheel, 130) == -1);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
}

static void test_progress_wheel_release_contract_covers_animation_and_style_boundary(
    struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL);
    assert(backend->style_class == (const char *)"wheel");
    assert(backend->user_data != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(picoui_progress_wheel_get_percent(wheel) == 72);
    assert(ld_progress_wheel->iProgress == 720);
    assert(ld_progress_wheel->tWheel.tCFG.bUseDirtyRegions == false);
    assert(ld_progress_wheel->tWheel.tCFG.tWheelColour != GLCD_COLOR_WHITE);
    assert(ld_progress_wheel->tWheel.tCFG.tDotColour == GLCD_COLOR_WHITE);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == false);
}

static void test_progress_wheel_native_color_and_dot_enable_round_trip(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(picoui_progress_wheel_set_wheel_color(wheel, 0x123456U) == 0);
    assert(picoui_progress_wheel_set_dot_color(wheel, 0xABCDEFU) == 0);
    assert(picoui_progress_wheel_set_dot_enabled(wheel, 0) == 0);
    assert(picoui_progress_wheel_get_dot_enabled(wheel) == 0);

    assert(ld_progress_wheel->tWheel.tCFG.tWheelColour != GLCD_COLOR_WHITE);
    assert(ld_progress_wheel->tWheel.tCFG.tDotColour != GLCD_COLOR_WHITE);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == true);

    assert(picoui_progress_wheel_set_dot_enabled(wheel, 1) == 0);
    assert(picoui_progress_wheel_get_dot_enabled(wheel) == 1);
    assert(ld_progress_wheel->tWheel.tCFG.bIgnoreDot == false);

    assert(picoui_progress_wheel_set_wheel_color(0, 0x111111U) == -1);
    assert(picoui_progress_wheel_set_dot_color(0, 0x222222U) == -1);
    assert(picoui_progress_wheel_set_dot_enabled(0, 1) == -1);
    assert(picoui_progress_wheel_get_dot_enabled(0) == -1);
}

static void test_progress_wheel_progress_alias_round_trip(struct picoui_progress_wheel *wheel)
{
    struct picoui_backend_widget *backend;
    ldProgressWheel_t *ld_progress_wheel;

    assert(wheel != 0);
    backend = (struct picoui_backend_widget *)wheel->widget.backend_widget;
    assert(backend != 0);
    ld_progress_wheel = (ldProgressWheel_t *)backend->ld_widget;
    assert(ld_progress_wheel != 0);

    assert(picoui_progress_wheel_set_progress(wheel, 37) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 37);
    assert(ld_progress_wheel->iProgress == 370);
}

static void test_progress_wheel_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_progress_wheel_create(0, "id") == 0);
    assert(picoui_progress_wheel_create(win, 0) == 0);
    assert(picoui_progress_wheel_set_percent(0, 50) == -1);
    assert(picoui_progress_wheel_set_wheel_color(0, 0xFFFFFFU) == -1);
    assert(picoui_progress_wheel_set_dot_color(0, 0x000000U) == -1);
    assert(picoui_progress_wheel_set_dot_enabled(0, 1) == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    struct picoui_progress_wheel *wheel_with_props;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    wheel_with_props = test_progress_wheel_create_and_props(win);
    test_progress_wheel_percent_bounds(win);
    test_progress_wheel_rejects_invalid_inputs(win);
    test_progress_wheel_release_contract_covers_animation_and_style_boundary(wheel_with_props);
    test_progress_wheel_native_color_and_dot_enable_round_trip(wheel_with_props);
    test_progress_wheel_progress_alias_round_trip(wheel_with_props);
    test_progress_wheel_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
