#include "picoui/app.h"
#include "picoui/clock.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <assert.h>

static void test_clock_create_and_props(struct picoui_window *win)
{
    int user_cookie = 17;
    struct picoui_clock_props props = {
        .id = "clock_props",
        .style_class = "clock",
        .user_data = &user_cookie,
        .step_second = 1,
    };
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock");
    struct picoui_clock *with_props =
        picoui_clock_create_with_props((struct picoui_widget *)win, &props);

    assert(clock != 0);
    assert(with_props != 0);
    assert(picoui_clock_get_step_second(clock) == 0);
    assert(picoui_clock_get_step_second(with_props) == props.step_second);
}

static void test_clock_step_second_state(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock_step_second");

    assert(clock != 0);
    assert(picoui_clock_set_step_second(clock, 0) == 0);
    assert(picoui_clock_get_step_second(clock) == 0);
    assert(picoui_clock_set_step_second(clock, 1) == 0);
    assert(picoui_clock_get_step_second(clock) == 1);
    assert(picoui_clock_set_step_second(clock, -1) == -1);
    assert(picoui_clock_get_step_second(clock) == 1);
    assert(picoui_clock_set_step_second(clock, 2) == -1);
    assert(picoui_clock_get_step_second(clock) == 1);
}

static void test_clock_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_clock *clock =
        picoui_clock_create((struct picoui_widget *)win, "clock_invalid");

    assert(clock != 0);
    assert(picoui_clock_create(0, "clock") == 0);
    assert(picoui_clock_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_clock_create_with_props(0,
                                          &(struct picoui_clock_props){
                                              .id = "bad_parent",
                                              .step_second = 0,
                                          }) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_clock_props){
                                              .step_second = 0,
                                          }) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_clock_props){
                                              .id = "bad_step_second_low",
                                              .step_second = -1,
                                          }) == 0);
    assert(picoui_clock_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_clock_props){
                                              .id = "bad_step_second_high",
                                              .step_second = 2,
                                          }) == 0);
    assert(picoui_clock_set_step_second(0, 1) == -1);
    assert(picoui_clock_get_step_second(0) == -1);
    assert(picoui_clock_set_step_second(clock, -1) == -1);
    assert(picoui_clock_set_step_second(clock, 2) == -1);
    assert(picoui_clock_get_step_second(clock) == 0);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_clock_create_and_props(win);
    test_clock_step_second_state(win);
    test_clock_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
