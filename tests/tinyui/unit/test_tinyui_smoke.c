#include "tinyui.h"
#include "internal/runtime_internal_legacy_api.h"

#include <assert.h>
#include <stddef.h>

int main(void)
{
    struct tinyui_window *win;

    assert(tinyui_init() == TINYUI_OK);

    win = (struct tinyui_window *)(void *)tinyui_screen_create();
    assert(win != NULL);

    tinyui_deinit();
    return 0;
}
