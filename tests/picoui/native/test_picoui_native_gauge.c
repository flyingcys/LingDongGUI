#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_gauge_get_rendered_state(const struct picoui_gauge *gauge,
                                           int *min_value,
                                           int *max_value,
                                           int *value,
                                           int *tick_count,
                                           int *tick_step,
                                           float *needle_angle);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_gauge *gauge;
    int min_value = -1;
    int max_value = -1;
    int value = -1;
    int tick_count = -1;
    int tick_step = -1;
    float needle_angle = -1.0f;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    gauge = picoui_gauge_create((struct picoui_widget *)window, "gauge");
    assert(gauge != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)gauge, 160, 160) == 0);

    assert(picoui_gauge_set_range(gauge, 10, 90) == 0);
    assert(picoui_gauge_set_value(gauge, 42) == 0);
    assert(picoui_gauge_set_tick_count(gauge, 9) == 0);
    min_value = picoui_gauge_get_min_value(gauge);
    max_value = picoui_gauge_get_max_value(gauge);
    value = picoui_gauge_get_value(gauge);
    tick_count = picoui_gauge_get_tick_count(gauge);
    assert(min_value == 10);
    assert(max_value == 90);
    assert(value == 42);
    assert(tick_count == 9);
    assert(picoui_gauge_get_angle(gauge) == 72.0f);

    assert(picoui_gauge_set_value(gauge, 200) == 0);
    value = picoui_gauge_get_value(gauge);
    assert(value == 90);
    assert(picoui_gauge_get_angle(gauge) == 180.0f);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_gauge_get_rendered_state(gauge,
                                                  &min_value,
                                                  &max_value,
                                                  &value,
                                                  &tick_count,
                                                  &tick_step,
                                                  &needle_angle) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_gauge_get_rendered_state(gauge,
                                                  &min_value,
                                                  &max_value,
                                                  &value,
                                                  &tick_count,
                                                  &tick_step,
                                                  &needle_angle) == 0);
    assert(min_value == 10);
    assert(max_value == 90);
    assert(value == 90);
    assert(tick_count == 9);
    assert(tick_step == 8);
    assert(needle_angle == 180.0f);

    assert(picoui_gauge_set_range(gauge, 20, 80) == 0);
    assert(picoui_gauge_get_min_value(gauge) == 20);
    assert(picoui_gauge_get_max_value(gauge) == 80);
    assert(picoui_gauge_get_value(gauge) == 80);
    assert(picoui_gauge_get_angle(gauge) == 180.0f);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_gauge_get_rendered_state(gauge,
                                                  &min_value,
                                                  &max_value,
                                                  &value,
                                                  &tick_count,
                                                  &tick_step,
                                                  &needle_angle) == 0);
    assert(min_value == 20);
    assert(max_value == 80);
    assert(value == 80);
    assert(tick_count == 9);
    assert(tick_step == 6);
    assert(needle_angle == 180.0f);

    assert(picoui_gauge_set_range(gauge, 90, 10) == -1);
    assert(picoui_gauge_get_min_value(gauge) == 20);
    assert(picoui_gauge_get_max_value(gauge) == 80);
    assert(picoui_gauge_get_value(gauge) == 80);
    assert(picoui_gauge_set_tick_count(gauge, 0) == -1);
    assert(picoui_gauge_get_tick_count(gauge) == 9);

    picoui_deinit();
    return 0;
}
