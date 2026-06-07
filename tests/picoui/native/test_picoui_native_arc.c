#include "picoui/picoui.h"
#include "internal.h"
#include "../../../src/gui/ldBase.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_arc_render(const struct picoui_backend_widget *backend);
int picoui_native_arc_get_rendered_value(const struct picoui_arc *arc, int *value);
int picoui_native_arc_get_rendered_angles(const struct picoui_arc *arc,
                                          float *bg_start_angle,
                                          float *bg_end_angle,
                                          float *fg_end_angle);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_arc *arc;
    struct picoui_backend_widget *backend;
    int rendered_value = -1;
    float rendered_bg_start = -1.0f;
    float rendered_bg_end = -1.0f;
    float rendered_fg_end = -1.0f;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    arc = picoui_arc_create((struct picoui_widget *)window, "speed");
    assert(arc != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)arc, 24, 32) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)arc, 160, 160) == 0);

    assert(picoui_arc_set_range(arc, 10, 90) == 0);
    assert(picoui_arc_set_value(arc, 42) == 0);
    assert(picoui_arc_set_background_angle(arc, 30.0f, 300.0f) == 0);
    assert(picoui_arc_set_end_angle(arc, 210.0f) == 0);

    assert(picoui_arc_get_min_value(arc) == 10);
    assert(picoui_arc_get_max_value(arc) == 90);
    assert(picoui_arc_get_value(arc) == 42);
    assert(picoui_arc_get_background_start_angle(arc) == 30.0f);
    assert(picoui_arc_get_background_angle(arc) == 270.0f);
    assert(picoui_arc_get_end_angle(arc) == 210.0f);

    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_ARC);
    assert(backend->ld_widget != 0);
    assert(ldBaseGetWidgetType((ldBase_t *)backend->ld_widget) == widgetTypeArc);
    assert(backend->value == 42);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_arc_get_rendered_value(arc, &rendered_value) == -1);
    assert(picoui_native_arc_get_rendered_angles(arc,
                                                 &rendered_bg_start,
                                                 &rendered_bg_end,
                                                 &rendered_fg_end) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_arc_get_rendered_value(arc, &rendered_value) == 0);
    assert(rendered_value == 42);
    assert(picoui_native_arc_get_rendered_angles(arc,
                                                 &rendered_bg_start,
                                                 &rendered_bg_end,
                                                 &rendered_fg_end) == 0);
    assert(rendered_bg_start == 30.0f);
    assert(rendered_bg_end == 300.0f);
    assert(rendered_fg_end == 210.0f);

    assert(picoui_arc_set_value(arc, 90) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_arc_get_rendered_value(arc, &rendered_value) == 0);
    assert(rendered_value == 90);

    assert(picoui_arc_set_range(arc, 20, 80) == 0);
    assert(picoui_arc_get_min_value(arc) == 20);
    assert(picoui_arc_get_max_value(arc) == 80);
    assert(picoui_arc_get_value(arc) == 80);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_arc_get_rendered_value(arc, &rendered_value) == 0);
    assert(rendered_value == 80);

    picoui_deinit();
    return 0;
}
