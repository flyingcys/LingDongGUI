#ifndef PICOUI_V1_1_RUNTIME_STATE_H
#define PICOUI_V1_1_RUNTIME_STATE_H

#include "picoui/indev.h"
#include "picoui/screen.h"

struct picoui_widget;

enum {
    PICOUI_V1_1_WIDGET_TREE_CAPACITY = 32,
};

struct picoui_v1_1_widget_binding {
    struct picoui_widget *widget;
    struct picoui_screen *screen;
    struct picoui_widget *parent;
    struct picoui_widget *first_child;
    struct picoui_widget *next_sibling;
    struct picoui_widget *root;
};

struct picoui_screen {
    int active;
    struct picoui_window *root_window;
};

struct picoui_indev {
    enum picoui_indev_type type;
    picoui_indev_read_cb_t read_cb;
    void *read_user_data;
};

struct picoui_runtime_state {
    int initialized;
    struct picoui_screen default_screen;
    struct picoui_screen scratch_screen;
    struct picoui_screen *active_screen;
    struct picoui_indev default_indev;
    int pointer_x;
    int pointer_y;
    int pointer_pressed;
    enum picoui_input_key key;
    int key_pressed;
    struct picoui_v1_1_widget_binding widget_tree[PICOUI_V1_1_WIDGET_TREE_CAPACITY];
};

struct picoui_runtime_state *picoui_runtime_state(void);

struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_ensure(struct picoui_widget *widget);
const struct picoui_v1_1_widget_binding *picoui_v1_1_widget_binding_find(
    const struct picoui_widget *widget);
int picoui_v1_1_widget_bind_root(struct picoui_screen *screen, struct picoui_widget *root);
int picoui_v1_1_widget_append_child(struct picoui_widget *parent, struct picoui_widget *child);
void picoui_v1_1_widget_unbind_root(struct picoui_widget *root);
int picoui_v1_1_render_active_screen(void);

#endif
