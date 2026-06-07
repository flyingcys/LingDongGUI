#include "picoui/picoui.h"
#include "picoui/port/sdl.h"
#include "internal.h"

#include <assert.h>

static void test_root_window_inherits_default_display_contract(void)
{
    struct picoui_screen *screen;
    struct picoui_window *window;
    struct picoui_backend_widget *backend;
    struct picoui_display_config config = {0};

    assert(picoui_init() == 0);
    assert(picoui_sdl_hal_init(320, 480) == 0);

    screen = picoui_screen_active();
    assert(screen != NULL);
    window = picoui_window_create_root(screen, "root");
    assert(window != NULL);

    backend = (struct picoui_backend_widget *)window->widget.backend_widget;
    assert(backend != NULL);
    assert(backend->owner != NULL);
    assert(picoui_display_get_config(backend->owner, &config) == 0);
    assert(config.width == 320);
    assert(config.height == 480);

    picoui_deinit();
}

int main(void)
{
    test_root_window_inherits_default_display_contract();
    return 0;
}
