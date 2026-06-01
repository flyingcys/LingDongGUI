#include "picoui/app.h"
#include "picoui/gauge.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <assert.h>

static void test_gauge_create_and_props(struct picoui_window *win)
{
    struct picoui_gauge_props props = {
        .id = "gauge_props",
        .angle = 45.0f,
        .pointer_color = 0x334455,
        .auto_move = 1,
    };
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge");
    struct picoui_gauge *with_props =
        picoui_gauge_create_with_props((struct picoui_widget *)win, &props);

    assert(gauge != 0);
    assert(with_props != 0);
    assert(picoui_gauge_get_angle(gauge) == 0.0f);
    assert(picoui_gauge_get_pointer_color(gauge) == 0x000000);
    assert(picoui_gauge_get_auto_move(gauge) == 0);
    assert(picoui_gauge_get_angle(with_props) == props.angle);
    assert(picoui_gauge_get_pointer_color(with_props) != 0U);
    assert(picoui_gauge_get_auto_move(with_props) == props.auto_move);
}

static void test_gauge_value_and_pointer_contract_match_backend_truth(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_angle");

    assert(gauge != 0);
    assert(picoui_gauge_set_angle(gauge, 90.0f) == 0);
    assert(picoui_gauge_get_angle(gauge) == 90.0f);
    assert(picoui_gauge_set_pointer_color(gauge, 0xAABBCC) == 0);
    assert(picoui_gauge_get_pointer_color(gauge) != 0U);
    assert(picoui_gauge_set_auto_move(gauge, 1) == 0);
    assert(picoui_gauge_get_auto_move(gauge) == 1);
    assert(picoui_gauge_set_auto_move(gauge, 0) == 0);
    assert(picoui_gauge_get_auto_move(gauge) == 0);
}

static void test_gauge_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_gauge *gauge = picoui_gauge_create((struct picoui_widget *)win, "gauge_invalid");

    assert(gauge != 0);
    assert(picoui_gauge_create(0, "gauge") == 0);
    assert(picoui_gauge_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_gauge_create_with_props(0,
                                          &(struct picoui_gauge_props){
                                              .id = "bad_parent",
                                          }) == 0);
    assert(picoui_gauge_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_gauge_create_with_props((struct picoui_widget *)win,
                                          &(struct picoui_gauge_props){
                                              .angle = 30.0f,
                                          }) == 0);
    assert(picoui_gauge_set_angle(0, 0.0f) == -1);
    assert(picoui_gauge_set_pointer_color(0, 0x0) == -1);
    assert(picoui_gauge_set_auto_move(0, 0) == -1);
    assert(picoui_gauge_get_angle(0) == 0.0f);
    assert(picoui_gauge_get_pointer_color(0) == 0x000000);
    assert(picoui_gauge_get_auto_move(0) == -1);
    assert(picoui_gauge_set_auto_move(gauge, 7) == 0);
    assert(picoui_gauge_get_auto_move(gauge) == 1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_gauge_create_and_props(win);
    test_gauge_value_and_pointer_contract_match_backend_truth(win);
    test_gauge_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
