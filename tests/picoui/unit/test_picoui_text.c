#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldText.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

static void test_text_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_test");
    struct picoui_backend_widget *backend;
    ldText_t *ld_text;

    assert(text != 0);
    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    assert(backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_TEXT);
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);
}

static void test_text_set_transparent_round_trip(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_transparent");
    assert(text != 0);
    assert(picoui_text_set_transparent(text, 1) == 0);
}

static void test_text_scroll_seek_and_move(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_scroll");
    assert(text != 0);
    assert(picoui_text_set_static_text(text, "Scrollable text content") == 0);
    assert(picoui_text_scroll_seek(text, 0) == 0);
    assert(picoui_text_scroll_move(text, 1) == 0);
}

static void test_text_create_with_props_sets_content(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create_with_props(
        win,
        &(struct picoui_text_props){
            .id = "text_props",
            .text = "Content",
            .width = 200,
            .height = 40,
        });
    struct picoui_backend_widget *backend;

    assert(text != 0);
    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Content") == 0);
}

static void test_text_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_text_create(0, "id") == 0);
    assert(picoui_text_create(win, 0) == 0);
    assert(picoui_text_set_static_text(0, "x") == -1);
}

int main(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_text_create_and_ld_mapping(win);
    test_text_set_transparent_round_trip(win);
    test_text_scroll_seek_and_move(win);
    test_text_create_with_props_sets_content(win);
    test_text_rejects_null_args(win);

    picoui_app_destroy(app);
    return 0;
}
