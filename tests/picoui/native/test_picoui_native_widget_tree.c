#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_widget_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_widget_bind_child(struct picoui_widget *parent, struct picoui_widget *child);
struct picoui_screen *picoui_widget_get_screen(const struct picoui_widget *widget);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *root_window;
    struct picoui_label *label_a;
    struct picoui_button *button_b;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root_window = picoui_window_create_root(screen, "root");
    assert(root_window != 0);

    label_a = picoui_label_create(root_window, "label_a");
    assert(label_a != 0);

    button_b = picoui_button_create(root_window, "button_b");
    assert(button_b != 0);

    assert(picoui_native_widget_bind_root(screen, root_window) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)label_a) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)button_b) == 0);

    assert(picoui_widget_get_parent((const struct picoui_widget *)root_window) == 0);
    assert(picoui_widget_get_parent((const struct picoui_widget *)label_a)
           == (struct picoui_widget *)root_window);
    assert(picoui_widget_get_parent((const struct picoui_widget *)button_b)
           == (struct picoui_widget *)root_window);
    assert(picoui_widget_get_first_child((const struct picoui_widget *)root_window)
           == (struct picoui_widget *)label_a);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)label_a)
           == (struct picoui_widget *)button_b);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)button_b) == 0);
    assert(picoui_widget_get_root((const struct picoui_widget *)label_a)
           == (struct picoui_widget *)root_window);
    assert(picoui_widget_get_root((const struct picoui_widget *)button_b)
           == (struct picoui_widget *)root_window);
    assert(picoui_widget_get_screen((const struct picoui_widget *)root_window) == screen);
    assert(picoui_widget_get_screen((const struct picoui_widget *)label_a) == screen);
    assert(picoui_widget_get_screen((const struct picoui_widget *)button_b) == screen);

    picoui_deinit();
    return 0;
}
