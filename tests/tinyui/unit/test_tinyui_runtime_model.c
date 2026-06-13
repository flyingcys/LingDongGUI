#include "runtime.h"
#include "window.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static void test_shared_sources_no_longer_include_tinyui_paths(void)
{
    int status = system(
        "rg -n '#include \"tinyui/' "
        "tinyui/src/core tinyui/src/display tinyui/src/indev tinyui/src/layout "
        "tinyui/src/theme tinyui/src/tick tinyui/src/osal >/dev/null");

    assert(status != 0);
}

static void test_runtime_internal_state_uses_tinyui_prefix(void)
{
    int old_status = system(
        "python3 - <<'PY'\n"
        "from pathlib import Path\n"
        "text = Path('/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime.c').read_text()\n"
        "raise SystemExit(1 if 'g_tinyui_runtime_app' in text else 0)\n"
        "PY");
    int new_status = system(
        "python3 - <<'PY'\n"
        "from pathlib import Path\n"
        "text = Path('/Users/cys/embedded/LingDongGUI/tinyui/src/core/runtime.c').read_text()\n"
        "raise SystemExit(0 if 'g_tinyui_runtime_app' in text else 1)\n"
        "PY");

    assert(old_status == 0);
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
