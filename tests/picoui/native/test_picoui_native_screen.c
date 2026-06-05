#include "picoui/picoui.h"

#include <assert.h>

int main(void)
{
    struct picoui_screen *active;
    struct picoui_screen *second;

    assert(picoui_init() == 0);
    active = picoui_screen_active();
    assert(active != 0);
    second = picoui_screen_create();
    assert(second != 0);
    assert(picoui_screen_load(second) == 0);
    assert(picoui_screen_active() == second);
    picoui_deinit();
    return 0;
}
