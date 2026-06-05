#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_checkbox_get_rendered_checked(const struct picoui_checkbox *checkbox, int *checked);

static int g_toggled_count;
static int g_last_value;
static struct picoui_widget *g_last_widget;

static void on_checkbox_toggled(struct picoui_widget *widget, int value, void *user_data)
{
    int *cookie = (int *)user_data;

    assert(cookie != 0);
    assert(*cookie == 73);
    g_toggled_count++;
    g_last_value = value;
    g_last_widget = widget;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_checkbox *checkbox;
    struct picoui_app *app;
    struct picoui_point origin;
    int cookie = 73;
    int rc;

    g_toggled_count = 0;
    g_last_value = -1;
    g_last_widget = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    checkbox = picoui_checkbox_create(window, "accept_terms");
    assert(checkbox != 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)checkbox, 32, 48) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)checkbox, 96, 40) == 0);

    assert(picoui_checkbox_set_checked(checkbox, 1) == 0);
    assert(picoui_checkbox_is_checked(checkbox) == 1);
    assert(picoui_checkbox_set_checked(checkbox, 0) == 0);
    assert(picoui_checkbox_is_checked(checkbox) == 0);
    assert(g_toggled_count == 0);

    assert(picoui_checkbox_set_on_toggled(checkbox, on_checkbox_toggled, &cookie) == 0);
    assert(picoui_native_event_bind_root(screen, window) == 0);
    assert(picoui_native_checkbox_get_rendered_checked(checkbox, &rc) == -1);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_native_checkbox_get_rendered_checked(checkbox, &rc) == 0);
    assert(rc == 0);

    app = ((struct picoui_backend_widget *)checkbox->widget.backend_widget)->owner;
    assert(app != 0);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)checkbox,
                                            (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);

    assert(picoui_input_push_pointer(app, origin.x + 8, origin.y + 8, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 0);
    assert(g_toggled_count == 0);

    assert(picoui_input_push_pointer(app, origin.x + 8, origin.y + 8, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 1);
    assert(g_toggled_count == 1);
    assert(g_last_value == 1);
    assert(g_last_widget == (struct picoui_widget *)checkbox);

    assert(picoui_input_push_pointer(app, origin.x + 8, origin.y + 8, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 1);
    assert(g_toggled_count == 1);

    assert(picoui_input_push_pointer(app, origin.x + 8, origin.y + 8, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 0);
    assert(g_toggled_count == 2);
    assert(g_last_value == 0);
    assert(g_last_widget == (struct picoui_widget *)checkbox);

    picoui_deinit();
    return 0;
}
