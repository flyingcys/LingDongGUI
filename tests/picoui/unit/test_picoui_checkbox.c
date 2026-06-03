#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldCheckBox.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

static void test_checkbox_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_test");
    struct picoui_backend_widget *backend;
    ldBase_t *ld_base;

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_CHECKBOX);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeCheckBox);
}

static void test_checkbox_create_with_props_pushes_all_fields(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create_with_props(
        win,
        &(struct picoui_checkbox_props){
            .id = "cb_props",
            .text = "Agree",
            .checked = 1,
        });
    struct picoui_backend_widget *backend;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Agree") == 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);
    assert(ld_checkbox->isChecked == true);
}

static void test_checkbox_set_checked_round_trip(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_checked");
    struct picoui_backend_widget *backend;
    ldCheckBox_t *ld_checkbox;

    assert(checkbox != 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend != 0);
    ld_checkbox = (ldCheckBox_t *)backend->ld_widget;
    assert(ld_checkbox != 0);

    assert(picoui_checkbox_set_checked(checkbox, 1) == 0);
    assert(ld_checkbox->isChecked == true);
    assert(checkbox->checked == 1);
    assert(picoui_checkbox_is_checked(checkbox) == 1);

    assert(picoui_checkbox_set_checked(checkbox, 0) == 0);
    assert(ld_checkbox->isChecked == false);
    assert(checkbox->checked == 0);
    assert(picoui_checkbox_is_checked(checkbox) == 0);
}

static void test_checkbox_set_text_round_trip(struct picoui_window *win)
{
    struct picoui_checkbox *checkbox = picoui_checkbox_create(win, "cb_text");
    struct picoui_backend_widget *backend;

    assert(checkbox != 0);
    assert(picoui_checkbox_set_text(checkbox, "Label") == 0);
    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Label") == 0);
}

static void test_checkbox_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_checkbox_create(0, "id") == 0);
    assert(picoui_checkbox_create(win, 0) == 0);
    assert(picoui_checkbox_set_text(0, "text") == -1);
    assert(picoui_checkbox_set_checked(0, 1) == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_checkbox_create_and_ld_mapping(win);
    test_checkbox_create_with_props_pushes_all_fields(win);
    test_checkbox_set_checked_round_trip(win);
    test_checkbox_set_text_round_trip(win);
    test_checkbox_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
