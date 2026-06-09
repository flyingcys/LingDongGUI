#include "picoui/runtime.h"
#include "picoui/window.h"

#include <assert.h>
#include <stddef.h>

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
    test_runtime_init_create_load_teardown();
    return 0;
}
