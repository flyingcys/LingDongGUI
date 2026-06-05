#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_slider_get_rendered_value(const struct picoui_slider *slider, int *value);

static int g_values[4];
static int g_value_count;
static struct picoui_widget *g_last_widget;

static void on_slider_value_changed(struct picoui_widget *widget, int value, void *user_data)
{
    int *cookie = (int *)user_data;

    assert(cookie != 0);
    assert(*cookie == 37);
    assert(g_value_count < 4);
    g_values[g_value_count] = value;
    g_value_count++;
    g_last_widget = widget;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_slider *slider;
    struct picoui_app *app;
    struct picoui_point origin;
    int cookie = 37;
    int percent = -1;
    int rc;

    g_values[0] = -1;
    g_values[1] = -1;
    g_values[2] = -1;
    g_values[3] = -1;
    g_value_count = 0;
    g_last_widget = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    slider = picoui_slider_create(window, "volume");
    assert(slider != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)slider, 20, 50) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)slider, 100, 20) == 0);

    assert(picoui_slider_set_range(slider, 0, 100) == 0);
    assert(picoui_slider_set_value(slider, 25) == 0);
    assert(slider->value == 25);
    assert(g_value_count == 0);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 25);

    assert(picoui_slider_set_on_value_changed(slider, on_slider_value_changed, &cookie) == 0);
    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_slider_get_rendered_value(slider, &rc) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_slider_get_rendered_value(slider, &rc) == 0);
    assert(rc == 25);

    app = ((struct picoui_backend_widget *)slider->widget.backend_widget)->owner;
    assert(app != 0);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)slider,
                                            (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);

    assert(picoui_input_push_pointer(app, origin.x + 25, origin.y + 10, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(slider->value == 25);
    assert(g_value_count == 0);

    assert(picoui_input_push_pointer(app, origin.x + 99, origin.y + 10, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(slider->value == 100);
    assert(g_value_count == 1);
    assert(g_values[0] == 100);
    assert(g_last_widget == (struct picoui_widget *)slider);
    assert(picoui_slider_get_percent(slider, &percent) == 0);
    assert(percent == 100);

    assert(picoui_input_push_pointer(app, origin.x + 99, origin.y + 10, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(slider->value == 100);
    assert(g_value_count == 1);
    assert(g_values[0] == 100);

    assert(picoui_input_push_pointer(app, origin.x + 99, origin.y + 10, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(slider->value == 100);
    assert(g_value_count == 1);

    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_slider_get_rendered_value(slider, &rc) == 0);
    assert(rc == 100);

    picoui_deinit();
    return 0;
}
