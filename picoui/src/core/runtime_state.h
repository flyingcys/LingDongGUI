#ifndef PICOUI_V1_1_RUNTIME_STATE_H
#define PICOUI_V1_1_RUNTIME_STATE_H

struct picoui_indev;
struct picoui_screen;
struct picoui_widget;

#define PICOUI_V1_1_WIDGET_TREE_CAPACITY 64

struct picoui_v1_1_widget_binding {
    struct picoui_widget *widget;
    struct picoui_screen *screen;
    struct picoui_widget *parent;
    struct picoui_widget *first_child;
    struct picoui_widget *next_sibling;
    struct picoui_widget *root;
};

struct picoui_runtime_state {
    int initialized;
    struct picoui_screen *active_screen;
    struct picoui_indev *default_indev;
    struct picoui_v1_1_widget_binding widget_tree[PICOUI_V1_1_WIDGET_TREE_CAPACITY];
};

struct picoui_runtime_state *picoui_runtime_state(void);
struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_ensure(struct picoui_widget *widget);
const struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_find(
    const struct picoui_widget *widget);
int picoui_v1_1_widget_bind_root(struct picoui_screen *screen, struct picoui_widget *root);
void picoui_v1_1_widget_unbind_root(struct picoui_widget *root);
int picoui_v1_1_widget_append_child(struct picoui_widget *parent, struct picoui_widget *child);

#endif
