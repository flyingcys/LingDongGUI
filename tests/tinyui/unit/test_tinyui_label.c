#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

extern int tinyui_widget_has_ld_binding(const struct tinyui_widget *widget);
static const char *test_self_binary_path = 0;

static const char *test_repo_path(const char *relative_path)
{
    static char path[2048];
    char base[2048];
    char *tests_dir;

    snprintf(base, sizeof(base), "%s", __FILE__);
    tests_dir = strstr(base, "tests/tinyui/unit/");
    assert(tests_dir != 0);
    *tests_dir = '\0';
    snprintf(path, sizeof(path), "%s%s", base, relative_path);
    return path;
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];

    assert(test_self_binary_path != 0);
    assert(symbol != 0);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
    pipe = popen(command, "r");
    assert(pipe != 0);
    while (fgets(line, sizeof(line), pipe) != 0) {
        size_t line_len = strlen(line);
        char *last_space;
        char *token;

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        last_space = strrchr(line, ' ');
        token = last_space != 0 ? last_space + 1 : line;
        if (strcmp(token, symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static void assert_source_lacks_text(const char *source_path, const char *needle)
{
    char command[1024];

    assert(source_path != 0);
    assert(needle != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(1 if sys.argv[2] in text else 0)\n"
             "PY",
             source_path,
             needle);
    assert(system(command) == 0);
}

static void assert_source_contains_text(const char *source_path, const char *needle)
{
    char command[1024];

    assert(source_path != 0);
    assert(needle != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(0 if sys.argv[2] in text else 1)\n"
             "PY",
             source_path,
             needle);
    assert(system(command) == 0);
}

static void test_label_create_and_ld_mapping(struct tinyui_window *win)
{
    struct tinyui_label *label = tinyui_label_create(win, "label_test");
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(label != 0);
    backend = &label->widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_LABEL);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeLabel);
}

static void test_label_set_text_round_trip(struct tinyui_window *win)
{
    struct tinyui_label *label = tinyui_label_create(win, "label_text");
    struct tinyui_widget *backend;
    int cookie = 7;

    assert(label != 0);
    assert(tinyui_label_set_text(label, "Hello TINYUI") == 0);
    backend = &label->widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "Hello TINYUI") == 0);
    assert(label->widget.text == (const char *)"Hello TINYUI");
    assert(tinyui_widget_set_style_class(&label->widget, "label-shared") == 0);
    assert(label->widget.style_class == (const char *)"label-shared");
    assert(backend->style_class == (const char *)"label-shared");
    assert(tinyui_widget_set_user_data(&label->widget, &cookie) == 0);
    assert(label->widget.user_data == &cookie);
    assert(backend->user_data == &cookie);
}

static void test_label_shared_text_helper_uses_tinyui_prefix(void)
{
    assert_source_contains_text(test_repo_path("tinyui/src/core/widget.c"),
                                "tinyui_widget_set_backend_text");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "tinyui_widget_set_backend_text");
    assert_self_binary_lacks_symbol("tinyui_backend_set_text");
    /* Phase C2: helpers below were collapsed into core helpers. */
    assert_source_lacks_text(test_repo_path("tinyui/src/widgets/label.c"),
                             "static ldLabel_t *tinyui_label_get_ld");
    assert_source_lacks_text(test_repo_path("tinyui/src/widgets/label.c"),
                             "static void tinyui_label_dispose_partial");
    assert_source_lacks_text(test_repo_path("tinyui/src/widgets/label.c"),
                             "static ldColor tinyui_label_rgb_to_ld_color");
    assert_source_lacks_text(test_repo_path("tinyui/src/widgets/label.c"),
                             "static unsigned int tinyui_label_ld_color_to_rgb");
    assert_source_lacks_text(test_repo_path("tinyui/src/widgets/label.c"),
                             "static arm_2d_align_t tinyui_label_map_align");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "static int tinyui_label_props_are_valid");
    /* Phase C2: backend lookup helper + unmap_align kept as 1-line wrappers. */
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "static ldLabel_t *tinyui_label_backend");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "static enum tinyui_align tinyui_label_unmap_align");
    /* Phase C2: uses core helpers instead of private copies. */
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "tinyui_rgb_to_ld_color");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "tinyui_ld_color_to_rgb");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "tinyui_align_to_arm2d");
    assert_source_contains_text(test_repo_path("tinyui/src/widgets/label.c"),
                                "tinyui_widget_destroy_common");
}

static void test_label_create_with_props_pushes_all_fields(struct tinyui_window *win)
{
    struct tinyui_label *label = tinyui_label_create_with_props(
        win,
        &(struct tinyui_label_props){
            .id = "label_props",
            .text = "PropsTest",
            .width = 200,
            .height = 30,
        });
    struct tinyui_widget *backend;

    assert(label != 0);
    backend = &label->widget;
    assert(backend->text != 0);
    assert(strcmp(backend->text, "PropsTest") == 0);
}

static void test_label_create_with_props_failure_rolls_back_attached_child(struct tinyui_window *win)
{
    ldBase_t *win_ld = (ldBase_t *)win->widget.ld_widget;
    ldBase_t *tail_ld = ldBaseGetChildList(win_ld);
    ldBase_t *next_before_ld = 0;
    struct tinyui_label *label;

    while (tail_ld != 0 && ldBaseGetNextSibling(tail_ld) != 0) {
        tail_ld = ldBaseGetNextSibling(tail_ld);
    }
    if (tail_ld != 0) {
        next_before_ld = ldBaseGetNextSibling(tail_ld);
    }

    label = tinyui_label_create_with_props(
        win,
        &(struct tinyui_label_props){
            .id = "label_props_invalid_align",
            .text = "bad",
            .align = (enum tinyui_align)99,
        });

    assert(label == 0);
    if (tail_ld != 0) {
        assert(ldBaseGetNextSibling(tail_ld) == next_before_ld);
    } else {
        assert(ldBaseGetChildList(win_ld) == 0);
    }
}

static void test_label_rejects_null_args(struct tinyui_window *win)
{
    assert(tinyui_label_create(0, "id") == 0);
    assert(tinyui_label_create(win, 0) == 0);
    assert(tinyui_label_set_text(0, "text") == -1);
}

static void test_label_destroy_clears_widget(struct tinyui_window *win)
{
    struct tinyui_label *label = tinyui_label_create(win, "label_to_del");
    assert(label != 0);
    assert(label->widget.ld_widget != 0);
    // destroy via widget API
    assert(tinyui_widget_destroy(&label->widget) == 0);
    assert(label->widget.ld_widget == 0);
}

static void test_label_constructor_binds_ld_without_backend_wrapper(struct tinyui_window *win)
{
    struct tinyui_label *label = tinyui_label_create(win, "label_direct_path");

    assert(label != 0);
    assert(tinyui_widget_has_ld_binding(&label->widget) == 1);
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_window *win;
    Dl_info self_info;
    assert(app != 0);
    assert(dladdr((void *)&main, &self_info) != 0);
    test_self_binary_path = self_info.dli_fname;
    win = tinyui_window_create(app, "root");
    assert(win != 0);

    test_label_create_and_ld_mapping(win);
    test_label_constructor_binds_ld_without_backend_wrapper(win);
    test_label_set_text_round_trip(win);
    test_label_shared_text_helper_uses_tinyui_prefix();
    test_label_create_with_props_pushes_all_fields(win);
    test_label_create_with_props_failure_rolls_back_attached_child(win);
    test_label_rejects_null_args(win);
    test_label_destroy_clears_widget(win);

    tinyui_app_destroy(app);
    return 0;
}
