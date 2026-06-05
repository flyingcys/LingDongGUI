#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_widget_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_widget_bind_child(struct picoui_widget *parent, struct picoui_widget *child);
int picoui_native_dirty_is_widget_dirty(const struct picoui_widget *widget);
int picoui_native_dirty_is_screen_dirty(const struct picoui_screen *screen);
int picoui_native_dirty_is_display_dirty(const struct picoui_display *display);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *root_window;
    struct picoui_label *label;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root_window = picoui_window_create_root(screen, "root");
    assert(root_window != 0);

    label = picoui_label_create(root_window, "geometry_label");
    assert(label != 0);

    assert(picoui_native_widget_bind_root(screen, root_window) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)label) == 0);

    assert(picoui_widget_set_pos((struct picoui_widget *)label, 10, 20) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)label, 80, 24) == 0);

    assert(picoui_widget_get_x((const struct picoui_widget *)label) == 10);
    assert(picoui_widget_get_y((const struct picoui_widget *)label) == 20);
    assert(picoui_widget_get_width((const struct picoui_widget *)label) == 80);
    assert(picoui_widget_get_height((const struct picoui_widget *)label) == 24);

    assert(picoui_native_dirty_is_widget_dirty((const struct picoui_widget *)label) == 1);
    assert(picoui_native_dirty_is_screen_dirty(screen) == 1);
    assert(picoui_native_dirty_is_display_dirty(display) == 1);

    picoui_deinit();
    return 0;
}
