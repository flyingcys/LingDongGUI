#include "tinyui.h"
#include "../../../src/gui/ldBase.h"
#include "internal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *test_source_file_path = __FILE__;

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

static int test_source_contains_symbol_definition(const char *path, const char *name)
{
    char command[1024];

    assert(path != 0);
    assert(name != 0);
    snprintf(command,
             sizeof(command),
             "python3 - '%s' '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import re\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "symbol = sys.argv[2]\n"
             "pattern = re.compile(r'(^|\\n)\\s*(?:static\\s+)?(?:struct\\s+)?[A-Za-z_][A-Za-z0-9_\\s\\*]*\\b' + re.escape(symbol) + r'\\b(?:\\s*\\(|\\s*\\{)', re.MULTILINE)\n"
             "raise SystemExit(0 if pattern.search(text) else 1)\n"
             "PY",
             path,
             name);
    return system(command) == 0;
}

static void assert_source_lacks_symbol_definition(const char *path, const char *name)
{
    assert(!test_source_contains_symbol_definition(path, name));
}

static void assert_source_has_symbol_definition(const char *path, const char *name)
{
    assert(test_source_contains_symbol_definition(path, name));
}

static void test_background_create_and_backend_mapping(struct tinyui_background *bg)
{
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    assert(bg != 0);
    backend = &bg->window.widget;
    assert(backend->ld_widget != 0);
    assert(backend->kind == TINYUI_BACKEND_WIDGET_BACKGROUND);
    ld_base = (ldBase_t *)backend->ld_widget;
    assert(ld_base != 0);
    assert(ld_base->widgetType == widgetTypeBackground);
}

static void test_background_window_accepts_widget_base_api(struct tinyui_background *bg)
{
    struct tinyui_widget *backend;
    ldBase_t *ld_base;

    backend = &bg->window.widget;
    ld_base = (ldBase_t *)backend->ld_widget;

    assert(tinyui_widget_set_pos(&bg->window.widget, 10, 20) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iX == 10);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tLocation.iY == 20);

    assert(tinyui_widget_set_size(&bg->window.widget, 400, 300) == 0);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iWidth == 400);
    assert(ld_base->use_as__arm_2d_control_node_t.tRegion.tSize.iHeight == 300);
}

static void test_background_offset_round_trip(struct tinyui_background *bg)
{
    assert(tinyui_window_set_background_offset((struct tinyui_window *)bg, 5, 10) == 0);
    assert(bg->window.background_offset_x == 5);
    assert(bg->window.background_offset_y == 10);
}

static void test_background_widget_file_owns_internal_helper_truth(void)
{
    const char *source_path = resolve_repo_path("tinyui/src/widgets/background.c");

    /* C3-T4: the local accessor tinyui_background_backend_host() and the
     * standalone tinyui_background_get_root_size() helper are gone —
     * background now folds its binding state directly onto
     * background->window.widget (single calloc, no host wrapper).  Pin down
     * the post-C3 contract: neither helper may reappear. */
    assert_source_lacks_symbol_definition(source_path, "tinyui_background_backend_host");
    assert_source_lacks_symbol_definition(source_path, "tinyui_background_get_root_size");
}

int main(void)
{
    struct tinyui_app *app = tinyui_app_create();
    struct tinyui_background *bg;

    assert(app != 0);
    bg = tinyui_background_create(app, "bg_root");
    assert(bg != 0);

    test_background_create_and_backend_mapping(bg);
    test_background_window_accepts_widget_base_api(bg);
    test_background_offset_round_trip(bg);
    test_background_widget_file_owns_internal_helper_truth();

    tinyui_app_destroy(app);
    return 0;
}
