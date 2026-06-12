#include "tinyui.h"
#include "internal.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *test_self_binary_path =
    "/Users/cys/embedded/LingDongGUI/build/tests/picoui/test_tinyui_app_lifecycle";
static const char *test_runtime_host_source =
    "/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_host.c";
static const char *test_animation_demo_path =
    "/Users/cys/embedded/LingDongGUI/build/examples/sdl/picoui_animation_basic_demo";

static void assert_self_binary_lacks_symbol(const char *symbol)
{
    char command[1024];
    FILE *pipe;
    char line[512];
    char prefixed_symbol[256];
    const char *line_symbol;

    snprintf(prefixed_symbol, sizeof(prefixed_symbol), "_%s", symbol);
    snprintf(command, sizeof(command), "nm %s 2>/dev/null", test_self_binary_path);
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
    char command[1024];

    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*(static[[:space:]]+)?(int|void)[[:space:]]+%s[[:space:]]*\\(\" %s >/dev/null",
             function_name,
             path);
    if (system(command) == 0) {
        fprintf(stderr, "unexpected backend-local function still present: %s in %s\n", function_name, path);
        abort();
    }
}

static void assert_source_has_function_definition(const char *path, const char *function_name)
{
    char command[1024];

    snprintf(command,
             sizeof(command),
             "rg -n \"^[[:space:]]*(static[[:space:]]+)?(int|void|uint32_t|Uint32)[[:space:]]+%s[[:space:]]*\\(\" %s >/dev/null",
             function_name,
             path);
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
    char command[4096];
    char *stdout_text;
    int status;
    char *tmp_dir = mkdtemp(tmp_template);

    assert(tmp_dir != NULL);
    snprintf(stdout_path, sizeof(stdout_path), "%s/stdout.txt", tmp_dir);
    snprintf(stderr_path, sizeof(stderr_path), "%s/stderr.txt", tmp_dir);
    snprintf(capture_path, sizeof(capture_path), "%s/frame.ppm", tmp_dir);
    snprintf(command,
             sizeof(command),
             "PICOUI_DEMO_AUTO_QUIT_MS=200 PICOUI_CAPTURE_FILE='%s' SDL_VIDEODRIVER=dummy '%s' >'%s' 2>'%s'",
             capture_path,
             test_animation_demo_path,
             stdout_path,
             stderr_path);

    status = system(command);
    assert(status != -1);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);

    stdout_text = read_entire_file(stdout_path);
    assert(count_exact_line(stdout_text, "PICOUI_RUNTIME_READY") == 1);
    assert(count_exact_line(stdout_text, "PICOUI_FOCUS_RUNTIME_READY=1") == 1);
    assert(count_exact_line(stdout_text, "PICOUI_BACKEND_STATIC_MAPPING=REAL_LDGUI") == 1);
    assert(count_exact_line(stdout_text, "PICOUI_SMOKE_LAYOUT_USED=0") == 1);
    free(stdout_text);
}

// Test 1: create + destroy bare app
static void test_app_create_and_destroy(void)
{
    struct picoui_app *app = picoui_app_create();
    assert(app != 0);
    picoui_app_destroy(app);
    // no crash = pass
}

// Test 2: app with multiple windows
static void test_app_multiple_windows(void)
{
    struct picoui_app *app = picoui_app_create();
    assert(app != 0);
    struct picoui_window *w1 = picoui_window_create(app, "win1");
    struct picoui_window *w2 = picoui_window_create(app, "win2");
    assert(w1 != 0);
    assert(w2 != 0);
    // verify windows have different backend widgets
    assert(w1->widget.backend_widget != w2->widget.backend_widget);
    picoui_app_destroy(app);
}

// Test 3: create window with null app
static void test_app_rejects_null(void)
{
    assert(picoui_window_create(0, "x") == 0);
    assert(picoui_app_set_theme(0, 0) == -1);
}

static void test_app_backend_wrappers_are_no_longer_public(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_app_init");
    assert_self_binary_lacks_symbol("picoui_backend_app_init");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_app_run");
    assert_self_binary_lacks_symbol("picoui_backend_app_run");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_app_shutdown");
    assert_self_binary_lacks_symbol("picoui_backend_app_shutdown");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_ensure_window");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_runtime_step");
    assert_self_binary_lacks_symbol("picoui_backend_runtime_step");
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
    char command[1024];

    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_step_app");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_pump_sdl_events");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_render");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_write_capture");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_log_mapping_markers");
    assert_source_has_function_definition(test_runtime_host_source, "tinyui_runtime_host_apply_smoke_cursor_layout");

    snprintf(command,
             sizeof(command),
             "python3 - <<'PY'\n"
             "from pathlib import Path\n"
             "text = Path('%s').read_text()\n"
             "start = text.index('int tinyui_runtime_host_step_app(')\n"
             "pump = text.index('event_result = tinyui_runtime_host_pump_sdl_events(app, state);', start)\n"
             "body = text[start:pump]\n"
             "raise SystemExit(0 if 'SDL_PollEvent' not in body else 1)\n"
             "PY",
             test_runtime_host_source);
    assert(system(command) == 0);
}

static void test_runtime_host_internal_bootstrap_helpers_no_longer_use_picoui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_touch_log_enabled");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_runtime_bootstrap");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_runtime_page_init");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_runtime_page_quit");
    assert(system("rg -n \"struct[[:space:]]+picoui_backend_runtime_state|g_picoui_backend_runtime_page\" "
                  "/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime_host.c >/dev/null") != 0);
}

static void test_runtime_host_internal_mapping_helpers_no_longer_use_picoui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_widget_is_supported_real");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_widget_is_real_mapped");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_append_id");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_widget_needs_fallback");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_window_has_real_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_widget_excludes_formal_mapping");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_widget_allows_smoke_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_append_widget_ids");
}

static void test_runtime_host_internal_render_helpers_no_longer_use_picoui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_log_image_source_marker");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_parse_auto_quit_ms");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_pixel_to_rgb888");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_pixel_to_argb8888");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_runtime_state_from_app");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_app_state_from_window");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_present_real_frame");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_apply_real_widget_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_log_mapping_markers");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_write_capture");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_apply_smoke_cursor_layout");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_render");
}

static void test_runtime_host_internal_step_helpers_no_longer_use_picoui_prefix(void)
{
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_prepare_runtime_state");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_prepare_runtime_scene");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_log_runtime_ready");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_prepare_runtime");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_pump_sdl_events");
    assert_source_lacks_function_definition(test_runtime_host_source, "picoui_backend_step_app");
}

int main(void)
{
    assert_self_binary_lacks_symbol("picoui_backend_app_init");
    assert_self_binary_lacks_symbol("picoui_backend_app_run");
    assert_self_binary_lacks_symbol("picoui_backend_app_shutdown");
    test_app_create_and_destroy();
    test_app_multiple_windows();
    test_app_rejects_null();
    test_app_backend_wrappers_are_no_longer_public();
    test_runtime_prepare_helpers_exist();
    test_runtime_step_uses_event_pump_helper();
    test_runtime_host_internal_bootstrap_helpers_no_longer_use_picoui_prefix();
    test_runtime_host_internal_mapping_helpers_no_longer_use_picoui_prefix();
    test_runtime_host_internal_render_helpers_no_longer_use_picoui_prefix();
    test_runtime_host_internal_step_helpers_no_longer_use_picoui_prefix();
    test_runtime_markers_are_logged_once_per_run();
    return 0;
}
