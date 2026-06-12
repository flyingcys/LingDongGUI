#include "runtime.h"
#include "window.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static void test_shared_sources_no_longer_include_picoui_paths(void)
{
    int status = system(
        "rg -n '#include \"picoui/' "
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
        "raise SystemExit(1 if 'g_picoui_runtime_app' in text else 0)\n"
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

int main(void)
{
    test_shared_sources_no_longer_include_picoui_paths();
    test_runtime_internal_state_uses_tinyui_prefix();
    test_runtime_init_create_load_teardown();
    return 0;
}
