#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_progress_bar_get_rendered_state(const struct picoui_progress_bar *bar,
                                                  int *percent,
                                                  int *horizontal,
                                                  int *inverted);
int picoui_native_progress_wheel_get_rendered_state(const struct picoui_progress_wheel *wheel,
                                                    int *percent,
                                                    int *dot_enabled);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_progress_bar *bar;
    struct picoui_progress_wheel *wheel;
    int percent = -1;
    int horizontal = -1;
    int inverted = -1;
    int dot_enabled = -1;
    int rc;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    bar = picoui_progress_bar_create(window, "bar");
    assert(bar != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)bar, 220, 24) == 0);
    assert(picoui_progress_bar_set_horizontal(bar, 1) == 0);
    assert(picoui_progress_bar_set_inverted(bar, 1) == 0);
    assert(picoui_progress_bar_set_percent(bar, 72) == 0);
    assert(picoui_progress_bar_get_percent(bar) == 72);
    assert(picoui_progress_bar_set_percent(bar, -1) == 0);
    assert(picoui_progress_bar_get_percent(bar) == 0);
    assert(picoui_progress_bar_set_percent(bar, 101) == 0);
    assert(picoui_progress_bar_get_percent(bar) == 100);
    assert(picoui_progress_bar_set_percent(bar, 72) == 0);

    wheel = picoui_progress_wheel_create((struct picoui_widget *)window, "wheel");
    assert(wheel != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)wheel, 96, 96) == 0);
    assert(picoui_progress_wheel_set_percent(wheel, 64) == 0);
    assert(picoui_progress_wheel_set_dot_enabled(wheel, 0) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 64);
    assert(picoui_progress_wheel_set_percent(wheel, -1) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 0);
    assert(picoui_progress_wheel_set_percent(wheel, 101) == 0);
    assert(picoui_progress_wheel_get_percent(wheel) == 100);
    assert(picoui_progress_wheel_set_percent(wheel, 64) == 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_progress_bar_get_rendered_state(bar, &percent, &horizontal, &inverted) == -1);
    assert(picoui_native_progress_wheel_get_rendered_state(wheel, &percent, &dot_enabled) == -1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);

    assert(picoui_native_progress_bar_get_rendered_state(bar, &percent, &horizontal, &inverted) == 0);
    assert(percent == 72);
    assert(horizontal == 1);
    assert(inverted == 1);

    assert(picoui_native_progress_wheel_get_rendered_state(wheel, &percent, &dot_enabled) == 0);
    assert(percent == 64);
    assert(dot_enabled == 0);

    picoui_deinit();
    return 0;
}
