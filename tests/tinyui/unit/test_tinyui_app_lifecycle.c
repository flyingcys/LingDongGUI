#include "tinyui.h"
#include "internal.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char test_self_binary_path[PATH_MAX];
static char test_runtime_host_source[PATH_MAX];
static char test_animation_demo_path[PATH_MAX];

static void shell_quote_path(char *quoted, size_t quoted_size, const char *path)
{
    size_t used = 0;

    assert(quoted != NULL);
    assert(path != NULL);
    assert(quoted_size > 2);

    quoted[used++] = '\'';
    while (*path != '\0') {
        if (*path == '\'') {
            assert(used + 4 < quoted_size);
            quoted[used++] = '\'';
            quoted[used++] = '\\';
            quoted[used++] = '\'';
            quoted[used++] = '\'';
        } else {
            assert(used + 1 < quoted_size);
            quoted[used++] = *path;
        }
        path++;
    }
    assert(used + 1 < quoted_size);
    quoted[used++] = '\'';
    quoted[used] = '\0';
}

static void init_test_paths(const char *self_binary_path)
{
    char resolved_self_binary_path[PATH_MAX];
    char repo_root[PATH_MAX];
    char build_root[PATH_MAX];
    const char *build_tests_dir;
    size_t repo_root_len;
    size_t build_root_len;

    assert(self_binary_path != NULL);
    assert(realpath(self_binary_path, resolved_self_binary_path) != NULL);
    assert(strlen(resolved_self_binary_path) < sizeof(test_self_binary_path));
    strcpy(test_self_binary_path, resolved_self_binary_path);

    build_tests_dir = strstr(test_self_binary_path, "/build/tests/tinyui/");
    assert(build_tests_dir != NULL);
    repo_root_len = (size_t)(build_tests_dir - test_self_binary_path);
    assert(repo_root_len < sizeof(repo_root));
    memcpy(repo_root, test_self_binary_path, repo_root_len);
    repo_root[repo_root_len] = '\0';

    build_root_len = repo_root_len + strlen("/build");
    assert(build_root_len < sizeof(build_root));
    snprintf(build_root, sizeof(build_root), "%s/build", repo_root);

    snprintf(test_runtime_host_source,
             sizeof(test_runtime_host_source),
             "%s/tinyui/port/sdl/runtime_host.c",
             repo_root);
    snprintf(test_animation_demo_path,
             sizeof(test_animation_demo_path),
             "%s/examples/sdl/tinyui_demo",
             build_root);
}

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[8192];
    char quoted_path[4096];
    FILE *pipe;
    char line[512];
    char prefixed_symbol[256];
    const char *line_symbol;

    assert(test_self_binary_path[0] != '\0');
    snprintf(prefixed_symbol, sizeof(prefixed_symbol), "_%s", symbol);
    shell_quote_path(quoted_path, sizeof(quoted_path), test_self_binary_path);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", quoted_path);
    pipe = popen(command, "r");
    assert(pipe != NULL);
    while (fgets(line, sizeof(line), pipe) != NULL) {
        size_t line_len = strlen(line);

        while (line_len > 0 && (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line[--line_len] = '\0';
        }
        line_symbol = strrchr(line, ' ');
        line_symbol = line_symbol != NULL ? line_symbol + 1 : line;
        if (strcmp(line_symbol, symbol) == 0 || strcmp(line_symbol, prefixed_symbol) == 0) {
            assert(!"unexpected symbol still present in test binary");
        }
    }
    assert(pclose(pipe) == 0);
}

static void assert_source_lacks_function_definition(const char *path, const char *function_name)
{
    char command[8192];
    char quoted_path[4096];

    shell_quote_path(quoted_path, sizeof(quoted_path), path);
    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*(static[[:space:]]+)?(int|void)[[:space:]]+%s[[:space:]]*\\(\" %s >/dev/null",
             function_name,
             quoted_path);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected backend-local function still present: %s in %s\n", function_name, path);
        abort();
    }
}

static void assert_source_has_function_definition(const char *path, const char *function_name)
{
    char command[8192];
    char quoted_path[4096];

    shell_quote_path(quoted_path, sizeof(quoted_path), path);
    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*(static[[:space:]]+)?(int|void|uint32_t|Uint32)[[:space:]]+%s[[:space:]]*\\(\" %s >/dev/null",
             function_name,
             quoted_path);
    if (system(command) != 0) {
        fprintf(stderr, "expected function missing: %s in %s\n", function_name, path);
        abort();
    }
}

static int count_exact_line(const char *text, const char *needle)
{
    int count = 0;
    const char *cursor = text;
    size_t needle_len = strlen(needle);

    while (cursor != NULL && *cursor != '\0') {
        const char *line_end = strchr(cursor, '\n');
        size_t line_len = line_end != NULL ? (size_t)(line_end - cursor) : strlen(cursor);

        if (line_len == needle_len && strncmp(cursor, needle, needle_len) == 0) {
            count += 1;
        }

        cursor = line_end != NULL ? line_end + 1 : NULL;
    }

    return count;
}

static char *read_entire_file(const char *path)
{
    FILE *fp = fopen(path, "rb");
    char *buffer;
    long size;

    assert(fp != NULL);
    assert(fseek(fp, 0, SEEK_END) == 0);
    size = ftell(fp);
    assert(size >= 0);
    assert(fseek(fp, 0, SEEK_SET) == 0);

    buffer = calloc((size_t)size + 1U, 1U);
    assert(buffer != NULL);
    assert(fread(buffer, 1, (size_t)size, fp) == (size_t)size);
    fclose(fp);
    return buffer;
}

static void test_runtime_markers_are_logged_once_per_run(void)
{
    char tmp_template[] = "/tmp/test_tinyui_app_lifecycle.XXXXXX";
    char stdout_path[1024];
    char stderr_path[1024];
    char capture_path[1024];
    char command[8192];
    char quoted_stdout_path[4096];
    char quoted_stderr_path[4096];
    char quoted_capture_path[4096];
    char quoted_demo_path[4096];
    char *stdout_text;
    int status;
    char *tmp_dir = mkdtemp(tmp_template);

    assert(tmp_dir != NULL);
    snprintf(stdout_path, sizeof(stdout_path), "%s/stdout.txt", tmp_dir);
    snprintf(stderr_path, sizeof(stderr_path), "%s/stderr.txt", tmp_dir);
    snprintf(capture_path, sizeof(capture_path), "%s/frame.ppm", tmp_dir);
    shell_quote_path(quoted_stdout_path, sizeof(quoted_stdout_path), stdout_path);
    shell_quote_path(quoted_stderr_path, sizeof(quoted_stderr_path), stderr_path);
    shell_quote_path(quoted_capture_path, sizeof(quoted_capture_path), capture_path);
    shell_quote_path(quoted_demo_path, sizeof(quoted_demo_path), test_animation_demo_path);
    snprintf(command,
             sizeof(command),
             "TINYUI_DEMO_AUTO_QUIT_MS=200 TINYUI_CAPTURE_FILE=%s SDL_VIDEODRIVER=dummy %s animation_basic >%s 2>%s",
             quoted_capture_path,
             quoted_demo_path,
             quoted_stdout_path,
             quoted_stderr_path);

    status = system(command);
    assert(status != -1);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);

    stdout_text = read_entire_file(stdout_path);
    assert(count_exact_line(stdout_text, "TINYUI_RUNTIME_READY") == 1);
    assert(count_exact_line(stdout_text, "TINYUI_FOCUS_RUNTIME_READY=1") == 1);
    assert(count_exact_line(stdout_text, "TINYUI_BACKEND_STATIC_MAPPING=REAL_LDGUI") == 1);
    assert(count_exact_line(stdout_text, "TINYUI_SMOKE_LAYOUT_USED=0") == 1);
    free(stdout_text);
}

// Test 1: create + destroy bare app
static void test_app_create_and_destroy(void)
{
    struct tinyui_app *app = tinyui_app_create();
    assert(app != 0);
    tinyui_app_destroy(app);
    // no crash = pass
}

// Test 2: app with multiple windows
static void test_app_multiple_windows(void)
{
    struct tinyui_app *app = tinyui_app_create();
    assert(app != 0);
    struct tinyui_window *w1 = tinyui_window_create(app, "win1");
    struct tinyui_window *w2 = tinyui_window_create(app, "win2");
    assert(w1 != 0);
    assert(w2 != 0);
    // verify windows have different backend widgets
    assert(w1->widget.backend_widget != w2->widget.backend_widget);
    tinyui_app_destroy(app);
}

// Test 3: create window with null app
static void test_app_rejects_null(void)
{
    assert(tinyui_window_create(0, "x") == 0);
    assert(tinyui_app_set_theme(0, 0) == -1);
}

static void test_app_backend_wrappers_are_no_longer_public(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_app_init");
    assert_self_binary_lacks_symbol("tinyui_backend_app_init");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_app_run");
    assert_self_binary_lacks_symbol("tinyui_backend_app_run");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_app_shutdown");
    assert_self_binary_lacks_symbol("tinyui_backend_app_shutdown");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_ensure_window");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_runtime_step");
    assert_self_binary_lacks_symbol("tinyui_backend_runtime_step");
}

static void test_runtime_prepare_helpers_exist(void)
{
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_prepare_runtime_state");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_prepare_runtime_scene");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_log_runtime_ready");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_prepare_runtime");
}

static void test_runtime_step_uses_event_pump_helper(void)
{
    char command[8192];
    char quoted_path[4096];

    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_step_app");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_pump_sdl_events");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_render");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_write_capture");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_log_mapping_markers");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_apply_smoke_cursor_layout");

    shell_quote_path(quoted_path, sizeof(quoted_path), test_runtime_host_source);
    snprintf(command,
             sizeof(command),
             "python3 - %s <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "start = text.index('int tinyui_runtime_host_step_app(')\n"
             "pump = text.index('event_result = tinyui_runtime_host_pump_sdl_events(app, state);', start)\n"
             "body = text[start:pump]\n"
             "raise SystemExit(0 if 'SDL_PollEvent' not in body else 1)\n"
             "PY",
             quoted_path);
    assert(system(command) == 0);
}

static void test_runtime_host_internal_bootstrap_helpers_no_longer_use_tinyui_prefix(void)
{
    char command[8192];
    char quoted_path[4096];

    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_touch_log_enabled");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_runtime_bootstrap");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_runtime_page_init");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_runtime_page_quit");
    shell_quote_path(quoted_path, sizeof(quoted_path), test_runtime_host_source);
    snprintf(command,
             sizeof(command),
             "rg -n \"struct[[:space:]]+tinyui_backend_runtime_state|g_tinyui_backend_runtime_page\" %s >/dev/null",
             quoted_path);
    assert(system(command) != 0);
}

static void test_runtime_host_internal_mapping_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_widget_is_supported_real");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_widget_is_real_mapped");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_append_id");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_widget_needs_fallback");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_window_has_real_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_widget_excludes_formal_mapping");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_widget_allows_smoke_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_append_widget_ids");
}

static void test_runtime_host_internal_render_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_log_image_source_marker");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_parse_auto_quit_ms");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_pixel_to_rgb888");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_pixel_to_argb8888");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_runtime_state_from_app");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_app_state_from_window");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_present_real_frame");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_apply_real_widget_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_log_mapping_markers");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_write_capture");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_apply_smoke_cursor_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_render");
}

static void test_runtime_host_internal_step_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_prepare_runtime_state");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_prepare_runtime_scene");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_log_runtime_ready");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_prepare_runtime");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_pump_sdl_events");
    assert_source_lacks_function_definition(test_runtime_host_source, "tinyui_backend_step_app");
}

int main(int argc, char **argv)
{
    (void)argc;
    init_test_paths(argv[0]);

    assert_self_binary_lacks_symbol("tinyui_backend_app_init");
    assert_self_binary_lacks_symbol("tinyui_backend_app_run");
    assert_self_binary_lacks_symbol("tinyui_backend_app_shutdown");
    test_app_create_and_destroy();
    test_app_multiple_windows();
    test_app_rejects_null();
    test_app_backend_wrappers_are_no_longer_public();
    test_runtime_prepare_helpers_exist();
    test_runtime_step_uses_event_pump_helper();
    test_runtime_host_internal_bootstrap_helpers_no_longer_use_tinyui_prefix();
    test_runtime_host_internal_mapping_helpers_no_longer_use_tinyui_prefix();
    test_runtime_host_internal_render_helpers_no_longer_use_tinyui_prefix();
    test_runtime_host_internal_step_helpers_no_longer_use_tinyui_prefix();
    test_runtime_markers_are_logged_once_per_run();
    return 0;
}
