#include "picoui/picoui.h"
#include "../../../picoui/src/core/internal_v1_1.h"
#include "../../../picoui/src/core/runtime_state.h"

#include <assert.h>

int main(void)
{
    struct picoui_screen *screen;
    struct picoui_screen *scratch;
    struct picoui_window *root;
    struct picoui_label *label_a;
    struct picoui_label *label_b;
    struct picoui_label *overflow_labels[29];
    int i;

    assert(picoui_init() == 0);

    screen = picoui_screen_active();
    assert(screen != 0);

    root = picoui_window_create_root(screen, "root");
    assert(root != 0);

    label_a = picoui_label_create(root, "label_a");
    label_b = picoui_label_create(root, "label_b");
    assert(label_a != 0);
    assert(label_b != 0);

    assert(picoui_widget_get_parent((const struct picoui_widget *)root) == 0);
    assert(picoui_widget_get_first_child((const struct picoui_widget *)root)
           == (struct picoui_widget *)label_a);
    assert(picoui_widget_get_parent((const struct picoui_widget *)label_a)
           == (struct picoui_widget *)root);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)label_a)
           == (struct picoui_widget *)label_b);
    assert(picoui_widget_get_parent((const struct picoui_widget *)label_b)
           == (struct picoui_widget *)root);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)label_b) == 0);
    assert(picoui_v1_1_widget_append_child(&root->widget, &label_a->widget) == -1);
    assert(picoui_widget_get_next_sibling((const struct picoui_widget *)label_b) == 0);

    scratch = picoui_screen_create();
    assert(scratch != 0);
    for (i = 0; i < 29; ++i) {
        overflow_labels[i] = picoui_label_create(root, "fill");
        assert(overflow_labels[i] != 0);
    }
    assert(picoui_window_create_root(scratch, "overflow_root") == 0);
    assert(picoui_screen_get_root_window(scratch) == 0);

    picoui_deinit();
    return 0;
}
