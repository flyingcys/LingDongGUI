#include "runtime.h"
#include "window.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

uint32_t arm_2d_helper_get_reference_clock_frequency(void)
{
    return 1000000u;
}

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

static void test_shared_sources_no_longer_include_tinyui_paths(void)
{
    char command[4096];

    snprintf(command,
             sizeof(command),
             "cd '%s' && "
             "rg -n '#include \"tinyui/' "
             "tinyui/src/core tinyui/src/display tinyui/src/indev tinyui/src/layout "
             "tinyui/src/theme tinyui/src/tick tinyui/src/osal >/dev/null",
             test_repo_path(""));
    int status = system(
        command);

    assert(status != 0);
}

static void test_runtime_internal_state_uses_tinyui_prefix(void)
{
    char old_command[4096];
    char new_command[4096];
    const char *runtime_source = test_repo_path("tinyui/src/core/runtime.c");

    snprintf(old_command,
             sizeof(old_command),
             "python3 - '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(1 if 'g_tinyui_runtime_app' in text else 0)\n"
             "PY",
             runtime_source);
    snprintf(new_command,
             sizeof(new_command),
             "python3 - '%s' <<'PY'\n"
             "from pathlib import Path\n"
             "import sys\n"
             "text = Path(sys.argv[1]).read_text()\n"
             "raise SystemExit(0 if 'g_tinyui_runtime_app' in text else 1)\n"
             "PY",
             runtime_source);

    int old_status = system(old_command);
    int new_status = system(new_command);

    assert(old_status != 0);
    assert(new_status == 0);
}

static void test_runtime_init_create_load_teardown(void)
{
    tinyui_obj_t *screen;

    assert(tinyui_init() == 0);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen) == 0);
    tinyui_deinit();
}

static void test_timer_handler_before_init_returns_error(void)
{
    /* The handler must not crash or exit the process when called
     * before tinyui_init(). It must return a stable error code. */
    int result = tinyui_timer_handler();
    assert(result < 0);
}

static void test_timer_handler_after_init_returns_status(void)
{
    /* After a full init→create→load cycle, the handler must not exit()
     * the process. It must return <0, 0, or >0 as a library status. */
    tinyui_obj_t *screen;

    assert(tinyui_init() == 0);
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    setenv("TINYUI_DEMO_AUTO_QUIT_MS", "1", 1);
    screen = tinyui_screen_create();
    assert(screen != NULL);
    assert(tinyui_screen_load(screen) == 0);

    /* Call once — must not exit, must return a valid status code.
     * In unit test context the backend may return -1 (no display),
     * 0 (running), or 1 (finished). All are valid non-exit paths. */
    int result = tinyui_timer_handler();
    assert(result >= -1 && result <= 1);

    tinyui_deinit();
}

int main(void)
{
    test_shared_sources_no_longer_include_tinyui_paths();
    test_runtime_internal_state_uses_tinyui_prefix();
    test_runtime_init_create_load_teardown();
    test_timer_handler_before_init_returns_error();
    test_timer_handler_after_init_returns_status();
    return 0;
}
