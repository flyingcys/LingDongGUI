#include "picoui/picoui.h"
#include "internal.h"
#include "backend.h"
#include "../../../src/gui/ldClock.h"

#include <assert.h>
#include <math.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_clock_render(const struct picoui_backend_widget *backend);
int picoui_native_clock_get_rendered_time(const struct picoui_clock *clock,
                                          int *hour,
                                          int *minute,
                                          int *second);

static void assert_close_radian(float actual, float expected)
{
    assert(fabsf(actual - expected) < 0.001f);
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_clock *clock;
    struct picoui_backend_widget *backend;
    ldClock_t *ld_clock;
    int hour = -1;
    int minute = -1;
    int second = -1;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    clock = picoui_clock_create((struct picoui_widget *)window, "clock");
    assert(clock != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)clock, 24, 32) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)clock, 200, 200) == 0);

    assert(picoui_clock_set_use_system_time(clock, 0) == 0);
    assert(picoui_clock_get_use_system_time(clock) == 0);
    assert(picoui_clock_set_step_second(clock, 1) == 0);
    assert(picoui_clock_set_time(clock, 23, 59, 59) == 0);
    assert(picoui_clock_get_time(clock, &hour, &minute, &second) == 0);
    assert(hour == 23);
    assert(minute == 59);
    assert(second == 59);

    backend = (struct picoui_backend_widget *)clock->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CLOCK);
    ld_clock = (ldClock_t *)backend->ld_widget;
    assert(ld_clock != 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_clock_get_rendered_time(clock, &hour, &minute, &second) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_clock_get_rendered_time(clock, &hour, &minute, &second) == 0);
    assert(hour == 23);
    assert(minute == 59);
    assert(second == 59);
    assert_close_radian(ld_clock->pointerInfo[0].radian, ANGLE_2_RADIAN(359.5f));
    assert_close_radian(ld_clock->pointerInfo[1].radian, ANGLE_2_RADIAN(354.0f));
    assert_close_radian(ld_clock->pointerInfo[2].radian, ANGLE_2_RADIAN(354.0f));

    assert(picoui_clock_tick(clock) == 0);
    assert(picoui_clock_get_time(clock, &hour, &minute, &second) == 0);
    assert(hour == 0);
    assert(minute == 0);
    assert(second == 0);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_clock_get_rendered_time(clock, &hour, &minute, &second) == 0);
    assert(hour == 0);
    assert(minute == 0);
    assert(second == 0);
    assert_close_radian(ld_clock->pointerInfo[0].radian, 0.0f);
    assert_close_radian(ld_clock->pointerInfo[1].radian, 0.0f);
    assert_close_radian(ld_clock->pointerInfo[2].radian, 0.0f);

    assert(picoui_clock_set_time(0, 12, 34, 56) == -1);
    assert(picoui_clock_get_time(0, &hour, &minute, &second) == -1);
    assert(picoui_clock_tick(0) == -1);
    assert(picoui_clock_set_time(clock, -1, 0, 0) == -1);
    assert(picoui_clock_set_time(clock, 24, 0, 0) == -1);
    assert(picoui_clock_set_time(clock, 0, -1, 0) == -1);
    assert(picoui_clock_set_time(clock, 0, 60, 0) == -1);
    assert(picoui_clock_set_time(clock, 0, 0, -1) == -1);
    assert(picoui_clock_set_time(clock, 0, 0, 60) == -1);
    assert(picoui_clock_get_time(clock, 0, &minute, &second) == -1);
    assert(picoui_clock_get_time(clock, &hour, 0, &second) == -1);
    assert(picoui_clock_get_time(clock, &hour, &minute, 0) == -1);

    picoui_deinit();
    return 0;
}
