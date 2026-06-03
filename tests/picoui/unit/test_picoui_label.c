#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

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

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_label_create_and_ld_mapping(win);
    test_label_set_text_round_trip(win);
    test_label_create_with_props_pushes_all_fields(win);
    test_label_rejects_null_args(win);
    test_label_destroy_clears_widget(win);

    picoui_app_destroy(app);
    return 0;
}
