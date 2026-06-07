#include "runtime_state.h"
#include "internal_v1_1.h"

#include "picoui/screen.h"
#include "picoui/widget.h"

static void picoui_v1_1_render_widget_tree(struct picoui_widget *widget)
{
    struct picoui_widget *child;

    if (widget == 0) {
        return;
    }

    widget->dirty = 0;

    child = picoui_widget_get_first_child(widget);
    while (child != 0) {
        picoui_v1_1_render_widget_tree(child);
        child = picoui_widget_get_next_sibling(child);
    }
}

int picoui_v1_1_render_active_screen(void)
{
    struct picoui_screen *screen = picoui_screen_active();
    struct picoui_window *root;

    if (screen == 0) {
        return -1;
    }

    root = picoui_screen_get_root_window(screen);
    if (root == 0) {
        return 0;
    }

    picoui_v1_1_render_widget_tree(&root->widget);
    return 0;
}
