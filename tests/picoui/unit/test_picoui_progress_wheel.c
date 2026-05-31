#include "picoui/app.h"
#include "picoui/progress_wheel.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <assert.h>

static void test_progress_wheel_create_and_props(struct picoui_window *win)
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

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_progress_wheel_create_and_props(win);
    test_progress_wheel_percent_bounds(win);
    test_progress_wheel_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
