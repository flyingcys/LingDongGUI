#include "picoui/app.h"
#include "picoui/arc.h"
#include "picoui/widget.h"
#include "picoui/window.h"

#include <assert.h>

static void test_arc_create_and_props(struct picoui_window *win)
{
    struct picoui_arc_props props = {
        .id = "arc_props",
        .bg_start_angle = 30.0f,
        .bg_end_angle = 270.0f,
        .fg_end_angle = 60.0f,
        .rotation_angle = 15.0f,
        .bg_color = 0x112233,
        .fg_color = 0x445566,
    };
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc");
    struct picoui_arc *with_props =
        picoui_arc_create_with_props((struct picoui_widget *)win, &props);

    assert(arc != 0);
    assert(with_props != 0);
    assert(picoui_arc_get_background_start_angle(arc) == 0.0f);
    assert(picoui_arc_get_background_angle(arc) == 360.0f);
    assert(picoui_arc_get_foreground_angle(arc) == 0.0f);
    assert(picoui_arc_get_rotation_angle(arc) == 0.0f);
    assert(picoui_arc_get_background_start_angle(with_props) == props.bg_start_angle);
    assert(picoui_arc_get_background_angle(with_props) == props.bg_end_angle - props.bg_start_angle);
    assert(picoui_arc_get_foreground_angle(with_props) == props.fg_end_angle);
    assert(picoui_arc_get_rotation_angle(with_props) == props.rotation_angle);
    assert(picoui_arc_get_background_color(with_props) != 0U);
    assert(picoui_arc_get_foreground_color(with_props) != 0U);
    assert(picoui_arc_get_background_color(with_props) != picoui_arc_get_foreground_color(with_props));
}

static void test_arc_value_and_angle_readback_match_backend_truth(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc_angles");

    assert(arc != 0);
    assert(picoui_arc_set_background_angle(arc, 45.0f, 315.0f) == 0);
    assert(picoui_arc_get_background_start_angle(arc) == 45.0f);
    assert(picoui_arc_get_background_angle(arc) == 270.0f);
    assert(picoui_arc_set_foreground_angle(arc, 135.0f) == 0);
    assert(picoui_arc_get_foreground_angle(arc) == 135.0f);
    assert(picoui_arc_set_rotation_angle(arc, 30.0f) == 0);
    assert(picoui_arc_get_rotation_angle(arc) == 30.0f);
    assert(picoui_arc_set_color(arc, 0xAABBCC, 0x223344) == 0);
    assert(picoui_arc_get_background_color(arc) == picoui_arc_get_background_color(arc));
    assert(picoui_arc_get_foreground_color(arc) == picoui_arc_get_foreground_color(arc));
    assert(picoui_arc_get_background_color(arc) != picoui_arc_get_foreground_color(arc));
}

static void test_arc_rejects_invalid_inputs(struct picoui_window *win)
{
    struct picoui_arc *arc = picoui_arc_create((struct picoui_widget *)win, "arc_invalid");

    assert(arc != 0);
    assert(picoui_arc_create(0, "arc") == 0);
    assert(picoui_arc_create((struct picoui_widget *)win, 0) == 0);
    assert(picoui_arc_create_with_props(0,
                                        &(struct picoui_arc_props){
                                            .id = "bad_parent",
                                        }) == 0);
    assert(picoui_arc_create_with_props((struct picoui_widget *)win, 0) == 0);
    assert(picoui_arc_create_with_props((struct picoui_widget *)win,
                                        &(struct picoui_arc_props){
                                            .bg_start_angle = -10.0f,
                                            .bg_end_angle = 180.0f,
                                        }) == 0);
    assert(picoui_arc_create_with_props((struct picoui_widget *)win,
                                        &(struct picoui_arc_props){
                                            .id = "bad_angle_order",
                                            .bg_start_angle = 40.0f,
                                            .bg_end_angle = 20.0f,
                                        }) == 0);
    assert(picoui_arc_set_background_angle(0, 0.0f, 0.0f) == -1);
    assert(picoui_arc_set_background_angle(arc, -1.0f, 180.0f) == -1);
    assert(picoui_arc_set_foreground_angle(0, 0.0f) == -1);
    assert(picoui_arc_set_foreground_angle(arc, -1.0f) == -1);
    assert(picoui_arc_set_rotation_angle(0, 0.0f) == -1);
    assert(picoui_arc_set_rotation_angle(arc, -1.0f) == -1);
    assert(picoui_arc_set_color(0, 0x0, 0x0) == -1);
    assert(picoui_arc_get_background_start_angle(0) == 0.0f);
    assert(picoui_arc_get_background_angle(0) == 0.0f);
    assert(picoui_arc_get_foreground_angle(0) == 0.0f);
    assert(picoui_arc_get_rotation_angle(0) == 0.0f);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_arc_create_and_props(win);
    test_arc_value_and_angle_readback_match_backend_truth(win);
    test_arc_rejects_invalid_inputs(win);

    picoui_app_destroy(app);
    return 0;
}
