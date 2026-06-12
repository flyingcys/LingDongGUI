#include "tinyui.h"
#include "port/sdl.h"

#include <assert.h>
#include <stddef.h>

static void test_sdl_attach_rejects_null(void)
{
    assert(picoui_port_sdl_attach(NULL) == -1);
}

static void test_sdl_attach_sets_default_display_contract(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {0};

    assert(app != NULL);
    assert(picoui_port_sdl_attach(app) == 0);
    assert(picoui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == PICOUI_COLOR_FORMAT_RGB565);
    assert(config.buffer_height == 0);

    picoui_app_destroy(app);
}

int main(void)
{
    test_sdl_attach_rejects_null();
    test_sdl_attach_sets_default_display_contract();
    return 0;
}
