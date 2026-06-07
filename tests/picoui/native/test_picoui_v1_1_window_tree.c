#include "picoui/picoui.h"

#include <assert.h>

int main(void)
{
    struct picoui_display *display;
    struct picoui_screen *screen;
    struct picoui_window *root;
    struct picoui_label *a;
    struct picoui_label *b;

    assert(picoui_init() == 0);

    display = picoui_display_create(320, 480);
    assert(display != 0);
    assert(picoui_display_set_default(display) == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root = picoui_window_create_root(screen, "root");
    assert(root != 0);

    a = picoui_label_create(root, "a");
    b = picoui_label_create(root, "b");
    assert(a != 0);
    assert(b != 0);

    assert(picoui_screen_load(screen) == 0);

    assert(picoui_widget_get_child_count((const struct picoui_widget *)root) == 2);
    assert(picoui_widget_get_parent((const struct picoui_widget *)root) == 0);
    assert(picoui_widget_get_parent((const struct picoui_widget *)a) == (struct picoui_widget *)root);
    assert(picoui_widget_get_parent((const struct picoui_widget *)b) == (struct picoui_widget *)root);
    assert(picoui_widget_get_first_child((const struct picoui_widget *)root)
           == (struct picoui_widget *)a);
    assert(picoui_widget_get_first_child((const struct picoui_widget *)a) == 0);
    assert(picoui_widget_get_first_child((const struct picoui_widget *)b) == 0);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)a)
           == (struct picoui_widget *)b);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)b) == 0);
    assert(picoui_widget_get_root((const struct picoui_widget *)a) == (struct picoui_widget *)root);
    assert(picoui_widget_get_root((const struct picoui_widget *)b) == (struct picoui_widget *)root);

    picoui_deinit();
    return 0;
}
