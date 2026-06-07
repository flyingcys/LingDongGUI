#include "picoui/picoui.h"

#include <assert.h>

int picoui_native_widget_bind_root(struct picoui_screen *screen, struct picoui_window *root_window);
int picoui_native_widget_bind_child(struct picoui_widget *parent, struct picoui_widget *child);
int picoui_native_layout_apply_root(struct picoui_window *root_window);

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *root_window;
    struct picoui_label *a;
    struct picoui_label *b;
    struct picoui_label *c;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root_window = picoui_window_create_root(screen, "root");
    assert(root_window != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)root_window, 300, 100) == 0);
    assert(picoui_window_set_layout_type(root_window, PICOUI_WINDOW_LAYOUT_FLEX) == 0);
    assert(picoui_flex_set_flow(root_window, PICOUI_FLEX_FLOW_ROW) == 0);
    assert(picoui_flex_set_gap(root_window, 10, 0) == 0);

    a = picoui_label_create(root_window, "a");
    b = picoui_label_create(root_window, "b");
    c = picoui_label_create(root_window, "c");
    assert(a != 0);
    assert(b != 0);
    assert(c != 0);

    assert(picoui_widget_set_size((struct picoui_widget *)a, 50, 20) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)b, 50, 20) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)c, 50, 20) == 0);

    assert(picoui_native_widget_bind_root(screen, root_window) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)a) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)b) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)c) == 0);

    assert(picoui_native_layout_apply_root(root_window) == 0);

    assert(picoui_widget_get_x((const struct picoui_widget *)a) == 0);
    assert(picoui_widget_get_x((const struct picoui_widget *)b) == 60);
    assert(picoui_widget_get_x((const struct picoui_widget *)c) == 120);
    assert(picoui_widget_get_y((const struct picoui_widget *)a) == 0);
    assert(picoui_widget_get_y((const struct picoui_widget *)b) == 0);
    assert(picoui_widget_get_y((const struct picoui_widget *)c) == 0);

    assert(picoui_window_set_layout_type(root_window, PICOUI_WINDOW_LAYOUT_FLEX) == 0);
    assert(picoui_flex_set_flow(root_window, PICOUI_FLEX_FLOW_COLUMN) == 0);
    assert(picoui_flex_set_align(root_window,
                                 PICOUI_ALIGN_CENTER,
                                 PICOUI_ALIGN_CENTER,
                                 PICOUI_ALIGN_CENTER) == 0);
    assert(picoui_flex_set_gap(root_window, 12, 12) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)root_window, 320, 240) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)a, 120, 20) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)b, 100, 20) == 0);
    assert(picoui_widget_set_size((struct picoui_widget *)c, 80, 20) == 0);

    assert(picoui_native_layout_apply_root(root_window) == 0);
    assert(picoui_widget_get_x((const struct picoui_widget *)a) == 100);
    assert(picoui_widget_get_x((const struct picoui_widget *)b) == 110);
    assert(picoui_widget_get_x((const struct picoui_widget *)c) == 120);
    assert(picoui_widget_get_y((const struct picoui_widget *)a) == 78);
    assert(picoui_widget_get_y((const struct picoui_widget *)b) == 110);
    assert(picoui_widget_get_y((const struct picoui_widget *)c) == 142);

    picoui_deinit();
    return 0;
}
