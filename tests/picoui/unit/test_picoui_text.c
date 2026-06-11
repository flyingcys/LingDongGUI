#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldText.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

void picoui_backend_text_test_fail_next_set_font(void);

static void *g_test_text_alloc_fail_once_result = (void *)1;

void *ldCalloc(uint32_t num, uint32_t size)
{
    if (g_test_text_alloc_fail_once_result == NULL) {
        g_test_text_alloc_fail_once_result = (void *)1;
        return NULL;
    }
    return calloc((size_t)num, (size_t)size);
}

static void test_text_create_and_ld_mapping(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_test");
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    ldText_t *ld_text;

    assert(text != 0);
    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    assert(backend != 0);
    parent_backend = (struct picoui_backend_widget *)win->widget.backend_widget;
    assert(parent_backend != 0);
    assert(backend->kind == PICOUI_BACKEND_WIDGET_TEXT);
    assert(backend->parent == parent_backend);
    assert(backend->root == parent_backend->root);
    assert(backend->owner == parent_backend->owner);
    assert(backend->host_widget == &text->widget);
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

static void test_text_create_with_props_font_failure_rolls_back_attached_child(struct picoui_window *win)
{
    struct picoui_backend_widget *parent_backend =
        (struct picoui_backend_widget *)win->widget.backend_widget;
    struct picoui_backend_widget *tail = parent_backend->first_child;
    struct picoui_backend_widget *next_before = 0;
    struct picoui_font failed_font = {"Sans", 24};
    struct picoui_text *text;

    while (tail != 0 && tail->next_sibling != 0) {
        tail = tail->next_sibling;
    }
    if (tail != 0) {
        next_before = tail->next_sibling;
    }

    picoui_backend_text_test_fail_next_set_font();
    text = picoui_text_create_with_props(
        win,
        &(struct picoui_text_props){
            .id = "text_props_font_fail",
            .text = "Content",
            .font = &failed_font,
        });

    assert(text == 0);
    if (tail != 0) {
        assert(tail->next_sibling == next_before);
    } else {
        assert(parent_backend->first_child == 0);
    }
}

static void test_text_rejects_null_args(struct picoui_window *win)
{
    assert(picoui_text_create(0, "id") == 0);
    assert(picoui_text_create(win, 0) == 0);
    assert(picoui_text_set_static_text(0, "x") == -1);
}

static void test_text_set_text_handles_alloc_failure_without_crash(struct picoui_window *win)
{
    struct picoui_text *text = picoui_text_create(win, "text_alloc_failure");
    struct picoui_backend_widget *backend;
    ldText_t *ld_text;

    assert(text != 0);
    backend = (struct picoui_backend_widget *)text->widget.backend_widget;
    assert(backend != 0);
    ld_text = (ldText_t *)backend->ld_widget;
    assert(ld_text != 0);

    g_test_text_alloc_fail_once_result = NULL;
    assert(picoui_text_set_text(text, "oom") == 0);
    assert(ld_text->pStr == NULL);
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
    test_text_create_with_props_font_failure_rolls_back_attached_child(win);
    test_text_rejects_null_args(win);
    test_text_set_text_handles_alloc_failure_without_crash(win);

    picoui_app_destroy(app);
    return 0;
}
