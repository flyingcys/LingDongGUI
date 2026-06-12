#include "picoui/picoui.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldText.h"
#include "internal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tinyui_text_test_fail_next_set_font(void);

static void *g_test_text_alloc_fail_once_result = (void *)1;
static const char *test_source_file_path = __FILE__;

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
    assert(text->widget.text == (const char *)"Content");
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

    tinyui_text_test_fail_next_set_font();
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

static int file_contains_pattern(const char *path, const char *pattern)
{
    FILE *fp = fopen(path, "rb");
    long size;
    char *buffer;
    int found = 0;

    if (fp == NULL) {
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    size = ftell(fp);
    if (size < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    buffer = (char *)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        fclose(fp);
        return 0;
    }

    if (fread(buffer, 1, (size_t)size, fp) == (size_t)size) {
        buffer[size] = '\0';
        found = strstr(buffer, pattern) != NULL;
    }

    free(buffer);
    fclose(fp);
    return found;
}

static const char *resolve_repo_path(const char *repo_relative_path)
{
    static char resolved_path[1024];
    char base_path[1024];
    char *tests_dir;
    size_t base_len;

    assert(test_source_file_path != 0);
    assert(repo_relative_path != 0);
    assert(strlen(test_source_file_path) < sizeof(base_path));
    snprintf(base_path, sizeof(base_path), "%s", test_source_file_path);
    tests_dir = strstr(base_path, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    base_len = strlen(base_path);
    assert(base_len + strlen(repo_relative_path) + 1 < sizeof(resolved_path));
    snprintf(resolved_path, sizeof(resolved_path), "%s%s", base_path, repo_relative_path);
    return resolved_path;
}

static int binary_symbol_exists(const char *binary_path, const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[1024];
    int found = 0;

    if (snprintf(command, sizeof(command), "nm \"%s\" 2>/dev/null", binary_path) <= 0) {
        return 0;
    }

    pipe = popen(command, "r");
    if (pipe == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), pipe) != NULL) {
        if (strstr(line, symbol) != NULL) {
            found = 1;
            break;
        }
    }

    (void)pclose(pipe);
    return found;
}

static void compose_legacy_text_symbol(char *buffer, size_t size, const char *suffix)
{
    int written = snprintf(buffer, size, "%s%s", "picoui_backend_text_", suffix);
    assert(written > 0);
    assert((size_t)written < size);
}

static void test_text_legacy_backend_helper_names_are_gone(const char *binary_path)
{
    static const char *source_only_text_internal_symbols[] = {
        "picoui_text_dispose_partial",
        "picoui_text_props_are_valid",
    };
    static const char *source_only_suffixes[] = {
        "rgb_to_ld_color",
        "get_ld_text",
        "apply_consumed_font",
        "default_font",
        "resolve_font",
    };
    static const char *binary_and_source_suffixes[] = {
        "set_font",
        "set_static_text",
        "set_transparent",
        "set_text_color",
        "set_bg_color",
        "set_background_source",
        "scroll_seek",
        "scroll_move",
        "test_fail_next_set_font",
    };
    char symbol[128];
    size_t index;
    const char *text_source_path = resolve_repo_path("tinyui/src/widgets/text.c");

    for (index = 0; index < sizeof(source_only_text_internal_symbols)
                            / sizeof(source_only_text_internal_symbols[0]); ++index) {
        assert(file_contains_pattern(text_source_path,
                                     source_only_text_internal_symbols[index]) == 0);
    }

    for (index = 0; index < sizeof(source_only_suffixes) / sizeof(source_only_suffixes[0]); ++index) {
        compose_legacy_text_symbol(symbol, sizeof(symbol), source_only_suffixes[index]);
        assert(file_contains_pattern(text_source_path, symbol) == 0);
    }

    for (index = 0; index < sizeof(binary_and_source_suffixes) / sizeof(binary_and_source_suffixes[0]); ++index) {
        compose_legacy_text_symbol(symbol, sizeof(symbol), binary_and_source_suffixes[index]);
        assert(file_contains_pattern(text_source_path, symbol) == 0);
        assert(binary_symbol_exists(binary_path, symbol) == 0);
    }
}

static void test_text_shared_text_helper_uses_tinyui_prefix(const char *binary_path)
{
    const char *widget_source_path = resolve_repo_path("tinyui/src/core/widget.c");
    const char *text_source_path = resolve_repo_path("tinyui/src/widgets/text.c");

    assert(file_contains_pattern(widget_source_path, "picoui_backend_set_text") == 0);
    assert(file_contains_pattern(widget_source_path, "tinyui_widget_set_backend_text") != 0);
    assert(file_contains_pattern(text_source_path, "tinyui_widget_set_backend_text") != 0);
    assert(binary_symbol_exists(binary_path, "picoui_backend_set_text") == 0);
}

int main(int argc, char **argv)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;
    assert(argc > 0);
    assert(app != 0);
    win = picoui_window_create(app, "root");
    assert(win != 0);

    test_text_legacy_backend_helper_names_are_gone(argv[0]);
    test_text_create_and_ld_mapping(win);
    test_text_set_transparent_round_trip(win);
    test_text_scroll_seek_and_move(win);
    test_text_create_with_props_sets_content(win);
    test_text_shared_text_helper_uses_tinyui_prefix(argv[0]);
    test_text_create_with_props_font_failure_rolls_back_attached_child(win);
    test_text_rejects_null_args(win);
    test_text_set_text_handles_alloc_failure_without_crash(win);

    picoui_app_destroy(app);
    return 0;
}
