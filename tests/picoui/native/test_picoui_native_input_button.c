#include "picoui/picoui.h"
#include "internal.h"

#include <assert.h>

int picoui_native_event_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);

static int g_click_count;

static void on_button_clicked(struct picoui_widget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    g_click_count++;
}

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_button *button;
    struct picoui_app *app;
    struct picoui_point origin;
    int rc;

    g_click_count = 0;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    window = picoui_window_create_root(screen, "root");
    assert(window != 0);

    button = picoui_button_create(window, "confirm_button");
    assert(button != 0);
    assert(picoui_button_set_text(button, "OK") == 0);
    assert(picoui_widget_set_pos((struct picoui_widget *)button, 24, 36) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)button, 120, 48) == 0);
    assert(picoui_button_set_on_clicked(button, on_button_clicked, 0) == 0);

    assert(picoui_native_event_bind_root(screen, window) == 0);
    app = ((struct picoui_backend_widget *)button->widget.backend_widget)->owner;
    assert(app != 0);
    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)button, (struct picoui_point){0, 0});
    assert(origin.x >= 0);
    assert(origin.y >= 0);

    assert(picoui_input_push_pointer(app, origin.x + 8, origin.y + 8, 1) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_click_count == 0);

    assert(picoui_input_push_pointer(app, origin.x + 8, origin.y + 8, 0) == 0);
    rc = picoui_timer_handler();
    assert(rc == 0 || rc == 1);
    assert(g_click_count == 1);

    picoui_deinit();
    return 0;
}
