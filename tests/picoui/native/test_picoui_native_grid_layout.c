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
    const int columns[2] = { 100, 120 };
    const int rows[2] = { 60, 80 };

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root_window = picoui_window_create_root(screen, "root");
    assert(root_window != 0);
    assert(picoui_widget_set_size((struct picoui_widget *)root_window, 320, 240) == 0);
    assert(picoui_window_set_layout_type(root_window, PICOUI_WINDOW_LAYOUT_GRID) == 0);
    assert(picoui_grid_set_columns(root_window, columns, 2) == 0);
    assert(picoui_grid_set_rows(root_window, rows, 2) == 0);
    assert(picoui_grid_set_gap(root_window, 10, 20) == 0);

    a = picoui_label_create(root_window, "a");
    b = picoui_label_create(root_window, "b");
    c = picoui_label_create(root_window, "c");
    assert(a != 0);
    assert(b != 0);
    assert(c != 0);

    assert(picoui_widget_set_grid_cell((struct picoui_widget *)a,
                                       0,
                                       0,
                                       1,
                                       1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)b,
                                       1,
                                       0,
                                       1,
                                       1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == 0);
    assert(picoui_widget_set_grid_cell((struct picoui_widget *)c,
                                       0,
                                       1,
                                       2,
                                       1,
                                       PICOUI_ALIGN_START,
                                       PICOUI_ALIGN_START) == 0);

    assert(picoui_native_widget_bind_root(screen, root_window) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)a) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)b) == 0);
    assert(picoui_native_widget_bind_child((struct picoui_widget *)root_window,
                                           (struct picoui_widget *)c) == 0);

    assert(picoui_native_layout_apply_root(root_window) == 0);

    assert(picoui_widget_get_x((const struct picoui_widget *)a) == 0);
    assert(picoui_widget_get_y((const struct picoui_widget *)a) == 0);
    assert(picoui_widget_get_x((const struct picoui_widget *)b) > picoui_widget_get_x((const struct picoui_widget *)a));
    assert(picoui_widget_get_y((const struct picoui_widget *)b) == 0);
    assert(picoui_widget_get_x((const struct picoui_widget *)c) == 0);
    assert(picoui_widget_get_y((const struct picoui_widget *)c) > picoui_widget_get_y((const struct picoui_widget *)a));
    assert(picoui_widget_get_width((const struct picoui_widget *)c)
           == picoui_widget_get_width((const struct picoui_widget *)a)
               + picoui_widget_get_width((const struct picoui_widget *)b));

    picoui_deinit();
    return 0;
}
