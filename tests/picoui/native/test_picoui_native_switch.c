#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_switch_get_rendered_checked(const struct picoui_switch *sw, int *checked);

static int g_values[3];
static int g_value_count;
static struct picoui_widget *g_last_widget;

static void on_switch_toggled(struct picoui_widget *widget, int value, void *user_data)
{
    int *cookie = (int *)user_data;

    assert(cookie != 0);
    assert(*cookie == 91);
    assert(g_value_count < 3);
    g_values[g_value_count] = value;
    g_value_count++;
    g_last_widget = widget;
}

static void click_switch(struct picoui_app *app,
                         const struct picoui_switch *sw,
                         int expected_value,
                         int expected_count)
{
    struct picoui_point origin;
    int rc;

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)sw,
                                            (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);

    assert(picoui_input_push_pointer(app, origin.x + 20, origin.y + 12, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_value_count == expected_count - 1);

    assert(picoui_input_push_pointer(app, origin.x + 20, origin.y + 12, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_switch_is_checked((struct picoui_switch *)sw) == expected_value);
    assert(g_value_count == expected_count);
    assert(g_values[expected_count - 1] == expected_value);
    assert(g_last_widget == (struct picoui_widget *)sw);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_switch_get_rendered_checked(sw, &rc) == 0);
    assert(rc == expected_value);
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_switch *sw;
    struct picoui_app *app;
    int cookie = 91;
    int rc;

    g_value_count = 0;
    g_values[0] = -1;
    g_values[1] = -1;
    g_values[2] = -1;
    g_last_widget = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    sw = picoui_switch_create(window, "wifi");
    assert(sw != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)sw, 32, 48) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)sw, 64, 32) == 0);

    assert(picoui_switch_set_checked(sw, 1) == 0);
    assert(picoui_switch_is_checked(sw) == 1);
    assert(picoui_switch_set_checked(sw, 0) == 0);
    assert(picoui_switch_is_checked(sw) == 0);
    assert(g_value_count == 0);

    assert(picoui_switch_set_on_toggled(sw, on_switch_toggled, &cookie) == 0);
    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_switch_get_rendered_checked(sw, &rc) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_switch_get_rendered_checked(sw, &rc) == 0);
    assert(rc == 0);

    app = ((struct picoui_backend_widget *)sw->widget.backend_widget)->owner;
    assert(app != 0);

    click_switch(app, sw, 1, 1);
    click_switch(app, sw, 0, 2);
    click_switch(app, sw, 1, 3);

    picoui_deinit();
    return 0;
}
