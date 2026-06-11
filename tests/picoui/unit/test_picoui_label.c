#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

extern int picoui_widget_has_ld_binding(const struct picoui_widget *widget);

static void test_label_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_test");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(label != 0);
    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_LABEL);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeLabel);
}

static void test_label_set_text_round_trip(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_text");
    struct picoui_backend_widget *backend;

    assert(label != 0);
    assert(picoui_label_set_text(label, "Hello PicoUI") == 0);
    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Hello PicoUI") == 0);
}

static void test_label_create_with_props_pushes_all_fields(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create_with_props(
        win,
        &(struct picoui_label_props){
            .id = "label_props",
            .text = "PropsTest",
            .width = 200,
            .height = 30,
        });
    struct picoui_backend_widget *backend;

    assert(label != 0);
    backend = (struct picoui_backend_widget *)label->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "PropsTest") == 0);
}

static void test_label_create_with_props_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_label *label;

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    label = picoui_label_create_with_props(
        win,
        &(struct picoui_label_props){
            .id = "label_props_invalid_align",
            .text = "bad",
            .align = (enum picoui_align)99,
        });

    assert(label == 0);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }
}

static void test_label_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_label_create(0, "id") == 0);
    assert(picoui_label_create(win, 0) == 0);
    assert(picoui_label_set_text(0, "text") == -1);
}

static void test_label_destroy_clears_widget(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_to_del");
    assert(label != 0);
    assert(label->widget.backend_widget != 0);
    // destroy via widget API
    assert(picoui_widget_destroy(&label->widget) == 0);
    assert(label->widget.backend_widget == 0);
}

static void test_label_constructor_binds_ld_without_backend_wrapper(struct picoui_window *win)
{
    struct picoui_label *label = picoui_label_create(win, "label_direct_path");

    assert(label != 0);
    assert(picoui_widget_has_ld_binding(&label->widget) == 1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_label_create_and_ld_mapping(win);
    test_label_constructor_binds_ld_without_backend_wrapper(win);
    test_label_set_text_round_trip(win);
    test_label_create_with_props_pushes_all_fields(win);
    test_label_create_with_props_failure_rolls_back_attached_child(win);
    test_label_rejects_null_args(win);
    test_label_destroy_clears_widget(win);

    picoui_app_destroy(app);
    return 0;
}
