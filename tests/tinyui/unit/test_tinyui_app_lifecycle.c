#include "tinyui.h"
#include "internal.h"
#include "../../tinyui/port/sdl/host_internal.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char test_self_binary_path[PATH_MAX];
static char test_step_source[PATH_MAX];
static char test_observe_source[PATH_MAX];
static char test_hal_source[PATH_MAX];
static char test_driver_source[PATH_MAX];
static char test_animation_demo_path[PATH_MAX];
static char test_tinyui_header_source[PATH_MAX];

extern uint32_t tinyui_runtime_host_pixel_to_rgb888(COLOUR_INT pixel);
extern void tinyui_runtime_host_copy_flush_pixels(const struct tinyui_area *area,
                                                  const void *pixels,
                                                  void *user_data);

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
    char file_dir[PATH_MAX];
    char *last_slash;
    char command[PATH_MAX * 2];
    FILE *pipe;
    size_t len;

    assert(self_binary_path != NULL);
    assert(realpath(self_binary_path, resolved_self_binary_path) != NULL);
    assert(strlen(resolved_self_binary_path) < sizeof(test_self_binary_path));
    strcpy(test_self_binary_path, resolved_self_binary_path);

    /* Derive repo root from the test source file path (__FILE__) rather than
     * the binary location — build dir depth varies (build/tests/tinyui vs
     * build/<config>/tests/tinyui), but __FILE__ is always
     * <repo>/tests/tinyui/unit/test_tinyui_app_lifecycle.c. */
    assert(strlen(__FILE__) < sizeof(file_dir));
    strcpy(file_dir, __FILE__);
    last_slash = strrchr(file_dir, '/');
    assert(last_slash != 0);
    *last_slash = '\0';
    snprintf(command, sizeof(command),
             "cd \"%s/../../..\" && pwd", file_dir);
    pipe = popen(command, "r");
    assert(pipe != 0);
    assert(fgets(repo_root, sizeof(repo_root), pipe) != 0);
    assert(pclose(pipe) == 0);
    len = strlen(repo_root);
    while (len > 0 && (repo_root[len - 1] == '\n' || repo_root[len - 1] == '\r')) {
        repo_root[--len] = '\0';
    }
    assert(len > 0);

    /* Find the build root by walking up from the binary until tests/tinyui
     * is found, then take its parent. */
    {
        const char *tests_dir = strstr(test_self_binary_path, "/tests/tinyui/");
        size_t build_root_len;
        assert(tests_dir != NULL);
        build_root_len = (size_t)(tests_dir - test_self_binary_path);
        assert(build_root_len < sizeof(build_root));
        memcpy(build_root, test_self_binary_path, build_root_len);
        build_root[build_root_len] = '\0';
    }

    snprintf(test_step_source,
             sizeof(test_step_source),
             "%s/tinyui/port/sdl/step.c",
             repo_root);
    snprintf(test_observe_source,
             sizeof(test_observe_source),
             "%s/tinyui/port/sdl/observe.c",
             repo_root);
    snprintf(test_hal_source,
             sizeof(test_hal_source),
             "%s/tinyui/port/sdl/hal.c",
             repo_root);
    snprintf(test_driver_source,
             sizeof(test_driver_source),
             "%s/tinyui/src/drivers/tinyui_ldgui_disp_adapter.c",
             repo_root);
    snprintf(test_animation_demo_path,
             sizeof(test_animation_demo_path),
             "%s/examples/sdl/tinyui_demo",
             build_root);
    snprintf(test_tinyui_header_source,
             sizeof(test_tinyui_header_source),
             "%s/tinyui/include/tinyui.h",
             repo_root);
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

static void test_runtime_legacy_demo0_parity_starts_with_pfb_host(void)
{
    char tmp_template[] = "/tmp/test_tinyui_legacy_demo0_pfb.XXXXXX";
    char stdout_path[1024];
    char stderr_path[1024];
    char capture_path[1024];
    char command[8192];
    char quoted_stdout_path[4096];
    char quoted_stderr_path[4096];
    char quoted_capture_path[4096];
    char quoted_demo_path[4096];
    FILE *fp;
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
             "TINYUI_DEMO_AUTO_QUIT_MS=200 TINYUI_CAPTURE_FILE=%s SDL_VIDEODRIVER=dummy %s legacy_demo0_parity >%s 2>%s",
             quoted_capture_path,
             quoted_demo_path,
             quoted_stdout_path,
             quoted_stderr_path);

    status = system(command);
    assert(status != -1);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);

    fp = fopen(capture_path, "rb");
    assert(fp != NULL);
    {
        int byte;
        int has_nonzero_pixel = 0;
        int newlines = 0;

        while ((byte = fgetc(fp)) != EOF && newlines < 3) {
            if (byte == '\n') {
                newlines += 1;
            }
        }
        while ((byte = fgetc(fp)) != EOF) {
            if (byte != 0) {
                has_nonzero_pixel = 1;
                break;
            }
        }
        assert(has_nonzero_pixel);
    }
    fclose(fp);
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
    assert(w1->widget.ld_widget != w2->widget.ld_widget);
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
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_app_init");
    assert_self_binary_lacks_symbol("tinyui_backend_app_init");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_app_run");
    assert_self_binary_lacks_symbol("tinyui_backend_app_run");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_app_shutdown");
    assert_self_binary_lacks_symbol("tinyui_backend_app_shutdown");
    assert_source_lacks_function_definition(test_hal_source, "tinyui_backend_ensure_window");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_runtime_step");
    assert_self_binary_lacks_symbol("tinyui_backend_runtime_step");
}

static void test_tinyui_umbrella_no_longer_reexports_app_header(void)
{
    char *tinyui_header_text;

    tinyui_header_text = read_entire_file(test_tinyui_header_source);
    assert(strstr(tinyui_header_text, "#ifndef TINYUI_H") != NULL);
    assert(strstr(tinyui_header_text, "#include \"widgets/animation.h\"") != NULL);
    assert(strstr(tinyui_header_text, "#include \"app.h\"") == NULL);
    free(tinyui_header_text);
}

static void test_runtime_prepare_helpers_exist(void)
{
    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_prepare_runtime_state");
    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_prepare_runtime_scene");
    assert_source_has_function_definition(test_observe_source, "tinyui_runtime_host_log_runtime_ready");
    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_prepare_runtime");
}

static void test_runtime_step_uses_event_pump_helper(void)
{
    char command[8192];
    char quoted_path[4096];

    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_step_app");
    assert_source_has_function_definition(test_hal_source, "tinyui_runtime_host_pump_sdl_events");
    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_render");
    assert_source_has_function_definition(test_observe_source, "tinyui_runtime_host_write_capture");
    assert_source_has_function_definition(test_observe_source, "tinyui_runtime_host_log_mapping_markers");
    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_apply_smoke_cursor_layout");

    shell_quote_path(quoted_path, sizeof(quoted_path), test_step_source);
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

static void test_runtime_host_render_uses_backend_pfb_step(void)
{
    char command[8192];
    char quoted_path[4096];

    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_render");
    shell_quote_path(quoted_path, sizeof(quoted_path), test_step_source);
    snprintf(command,
             sizeof(command),
             "python3 - %s <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "start = text.index('static void tinyui_runtime_host_render(')\n"
             "end = text.index('\\n}', start) + 2\n"
             "body = text[start:end]\n"
             "ok = (\n"
             "    'tinyui_backend_step(app_state);' in body\n"
             "    and 'ldGuiDraw(app_state->ld_scene, &state->real_tile, true);' not in body\n"
             "    and 'memset(state->real_pixels,' not in body\n"
             ")\n"
             "raise SystemExit(0 if ok else 1)\n"
             "PY",
             quoted_path);
    assert(system(command) == 0);
}

static void test_ldgui_pfb_draw_handler_uses_legacy_dirty_region_flow(void)
{
    char command[8192];
    char quoted_path[4096];

    shell_quote_path(quoted_path, sizeof(quoted_path), test_driver_source);
    snprintf(command,
             sizeof(command),
             "python3 - %s <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "start = text.index('static void ldgui_port_update_widget_dirty_region(')\n"
             "end = text.index('/* ─── PFB flush handler ─── */', start)\n"
             "body = text[start:end]\n"
             "ok = (\n"
             "    'arm_2d_dynamic_dirty_region_wait_next' in body\n"
             "    and 'arm_2d_dynamic_dirty_region_update' in body\n"
             "    and 'arm_2d_helper_control_enum_get_next_node' in body\n"
             "    and 'ldgui_port_update_widget_dirty_region(scene,' in body\n"
             "    and 'arm_2d_dynamic_dirty_region_update(&scene->tDirtyRegionItem,\\n"
             "                                                   ptTile,' in body\n"
             "    and 'arm_2d_dynamic_dirty_region_update(&scene->tDirtyRegionItem,\\n"
             "                                                       ptTile,' in body\n"
             "    and 'arm_2d_dynamic_dirty_region_update(&scene->tDirtyRegionItem,\\n"
             "                                                   NULL,' not in body\n"
             ")\n"
             "raise SystemExit(0 if ok else 1)\n"
             "PY",
             quoted_path);
    assert(system(command) == 0);
}

static void test_backend_step_uses_scene_dirty_region_list(void)
{
    char command[8192];
    char quoted_path[4096];

    shell_quote_path(quoted_path, sizeof(quoted_path), test_driver_source);
    snprintf(command,
             sizeof(command),
             "python3 - %s <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "ok = (\n"
             "    'app_state->ld_scene->use_as__arm_2d_scene_t.ptDirtyRegion' in text\n"
             "    and 'arm_2d_helper_pfb_task(&s_tPFBHelper, NULL)' not in text\n"
             ")\n"
             "raise SystemExit(0 if ok else 1)\n"
             "PY",
             quoted_path);
    assert(system(command) == 0);
}

static void test_runtime_host_registers_display_flush_callback(void)
{
    char command[8192];
    char quoted_path[4096];

    assert_source_has_function_definition(test_step_source, "tinyui_runtime_host_prepare_runtime");
    assert_source_has_function_definition(test_hal_source, "tinyui_runtime_host_copy_flush_pixels");
    shell_quote_path(quoted_path, sizeof(quoted_path), test_step_source);
    snprintf(command,
             sizeof(command),
             "rg -n \"tinyui_display_set_flush_callback\\(app,\\s*tinyui_runtime_host_copy_flush_pixels,\\s*state\\)\" %s >/dev/null",
             quoted_path);
    assert(system(command) == 0);
}

static void test_runtime_host_flush_callback_copies_pfb_block_into_real_frame(void)
{
    COLOUR_INT frame[20];
    const COLOUR_INT block[6] = {
        (COLOUR_INT)0x0001U, (COLOUR_INT)0x0002U, (COLOUR_INT)0x0003U,
        (COLOUR_INT)0x0004U, (COLOUR_INT)0x0005U, (COLOUR_INT)0x0006U,
    };
    struct tinyui_area area = {
        .x = 1,
        .y = 2,
        .width = 3,
        .height = 2,
    };
    struct tinyui_runtime_host_state state;
    int i;

    for (i = 0; i < (int)(sizeof(frame) / sizeof(frame[0])); ++i) {
        frame[i] = (COLOUR_INT)0xAAAAU;
    }
    memset(&state, 0, sizeof(state));
    state.real_pixels = frame;
    state.display_width = 5;
    state.display_height = 4;

    tinyui_runtime_host_copy_flush_pixels(&area, block, &state);

    assert(frame[11] == (COLOUR_INT)0x0001U);
    assert(frame[12] == (COLOUR_INT)0x0002U);
    assert(frame[13] == (COLOUR_INT)0x0003U);
    assert(frame[16] == (COLOUR_INT)0x0004U);
    assert(frame[17] == (COLOUR_INT)0x0005U);
    assert(frame[18] == (COLOUR_INT)0x0006U);
    assert(frame[0] == (COLOUR_INT)0xAAAAU);
    assert(frame[10] == (COLOUR_INT)0xAAAAU);
    assert(frame[14] == (COLOUR_INT)0xAAAAU);
    assert(frame[19] == (COLOUR_INT)0xAAAAU);
}

static void test_runtime_host_internal_bootstrap_helpers_no_longer_use_tinyui_prefix(void)
{
    char command[8192];
    char quoted_path[4096];

    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_touch_log_enabled");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_runtime_bootstrap");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_runtime_page_init");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_runtime_page_quit");
    shell_quote_path(quoted_path, sizeof(quoted_path), test_step_source);
    snprintf(command,
             sizeof(command),
             "rg -n \"struct[[:space:]]+tinyui_backend_runtime_state|g_tinyui_backend_runtime_page\" %s >/dev/null",
             quoted_path);
    assert(system(command) != 0);
}

static void test_runtime_host_internal_mapping_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_widget_is_supported_real");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_widget_is_real_mapped");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_append_id");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_widget_needs_fallback");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_window_has_real_layout");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_widget_excludes_formal_mapping");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_widget_allows_smoke_layout");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_append_widget_ids");
}

static void test_runtime_host_internal_render_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_log_image_source_marker");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_parse_auto_quit_ms");
    assert_source_lacks_function_definition(test_hal_source, "tinyui_backend_pixel_to_rgb888");
    assert_source_lacks_function_definition(test_hal_source, "tinyui_backend_pixel_to_argb8888");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_runtime_state_from_app");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_app_state_from_window");
    assert_source_lacks_function_definition(test_hal_source, "tinyui_backend_present_real_frame");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_apply_real_widget_layout");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_log_mapping_markers");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_write_capture");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_apply_smoke_cursor_layout");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_render");
}

static void test_runtime_host_internal_step_helpers_no_longer_use_tinyui_prefix(void)
{
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_prepare_runtime_state");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_prepare_runtime_scene");
    assert_source_lacks_function_definition(test_observe_source, "tinyui_backend_log_runtime_ready");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_prepare_runtime");
    assert_source_lacks_function_definition(test_hal_source, "tinyui_backend_pump_sdl_events");
    assert_source_lacks_function_definition(test_step_source, "tinyui_backend_step_app");
}

static void test_runtime_host_rgb565_expands_like_legacy_sdl_capture(void)
{
#if __DISP0_CFG_COLOUR_DEPTH__ == 16
    assert(tinyui_runtime_host_pixel_to_rgb888((COLOUR_INT)0xC618U) == 0xC0C0C0U);
    assert(tinyui_runtime_host_pixel_to_rgb888((COLOUR_INT)0xFFFFU) == 0xF8FCF8U);
    assert(tinyui_runtime_host_pixel_to_rgb888((COLOUR_INT)0x55DCU) == 0x50B8E0U);
#endif
}

static void test_runtime_host_capture_defaults_to_first_rendered_frame(void)
{
    char tmp_template[] = "/tmp/test_tinyui_capture_first_frame.XXXXXX";
    char capture_path[1024];
    char *tmp_dir = mkdtemp(tmp_template);
    COLOUR_INT pixels[1] = {(COLOUR_INT)0xC618U};
    struct tinyui_runtime_host_state state;
    FILE *fp;

    assert(tmp_dir != NULL);
    snprintf(capture_path, sizeof(capture_path), "%s/frame.ppm", tmp_dir);
    assert(setenv("TINYUI_CAPTURE_FILE", capture_path, 1) == 0);
    assert(unsetenv("TINYUI_CAPTURE_MIN_FRAMES") == 0);

    memset(&state, 0, sizeof(state));
    state.real_pixels = pixels;
    state.display_width = 1;
    state.display_height = 1;
    state.rendered_frames = 1U;

    assert(tinyui_runtime_host_write_capture(&state) == 0);
    assert(state.capture_written == 1);
    fp = fopen(capture_path, "rb");
    assert(fp != NULL);
    fclose(fp);
}

static void test_runtime_host_capture_min_frames_can_wait_for_stable_frame(void)
{
    char tmp_template[] = "/tmp/test_tinyui_capture_min_frames.XXXXXX";
    char capture_path[1024];
    char *tmp_dir = mkdtemp(tmp_template);
    COLOUR_INT pixels[1] = {(COLOUR_INT)0xC618U};
    struct tinyui_runtime_host_state state;
    FILE *fp;

    assert(tmp_dir != NULL);
    snprintf(capture_path, sizeof(capture_path), "%s/frame.ppm", tmp_dir);
    assert(setenv("TINYUI_CAPTURE_FILE", capture_path, 1) == 0);
    assert(setenv("TINYUI_CAPTURE_MIN_FRAMES", "3", 1) == 0);

    memset(&state, 0, sizeof(state));
    state.real_pixels = pixels;
    state.display_width = 1;
    state.display_height = 1;
    state.rendered_frames = 2U;

    assert(tinyui_runtime_host_write_capture(&state) == 0);
    assert(state.capture_written == 0);
    fp = fopen(capture_path, "rb");
    assert(fp == NULL);

    state.rendered_frames = 3U;
    assert(tinyui_runtime_host_write_capture(&state) == 0);
    assert(state.capture_written == 1);
    fp = fopen(capture_path, "rb");
    assert(fp != NULL);
    fclose(fp);
    assert(unsetenv("TINYUI_CAPTURE_MIN_FRAMES") == 0);
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
    test_tinyui_umbrella_no_longer_reexports_app_header();
    test_runtime_prepare_helpers_exist();
    test_runtime_step_uses_event_pump_helper();
    test_runtime_host_render_uses_backend_pfb_step();
    test_ldgui_pfb_draw_handler_uses_legacy_dirty_region_flow();
    test_backend_step_uses_scene_dirty_region_list();
    test_runtime_host_registers_display_flush_callback();
    test_runtime_host_flush_callback_copies_pfb_block_into_real_frame();
    test_runtime_host_internal_bootstrap_helpers_no_longer_use_tinyui_prefix();
    test_runtime_host_internal_mapping_helpers_no_longer_use_tinyui_prefix();
    test_runtime_host_internal_render_helpers_no_longer_use_tinyui_prefix();
    test_runtime_host_rgb565_expands_like_legacy_sdl_capture();
    test_runtime_host_capture_defaults_to_first_rendered_frame();
    test_runtime_host_capture_min_frames_can_wait_for_stable_frame();
    test_runtime_legacy_demo0_parity_starts_with_pfb_host();
    test_runtime_host_internal_step_helpers_no_longer_use_tinyui_prefix();
    test_runtime_markers_are_logged_once_per_run();
    return 0;
}
