#include "picoui/picoui.h"
#include "picoui/port/sdl.h"
#include "internal.h"

#include <assert.h>

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

static void test_sdl_attach_prefers_hal_display_size(void)
{
    struct picoui_app *app;
    struct picoui_display_config config = {0};

    assert(picoui_init() == 0);
    assert(picoui_sdl_hal_init(320, 480) == 0);
    app = picoui_app_create();
    assert(app != NULL);
    assert(picoui_port_sdl_attach(app) == 0);
    assert(picoui_display_get_config(app, &config) == 0);
    assert(config.width == 320);
    assert(config.height == 480);

    picoui_app_destroy(app);
    picoui_deinit();
}

static void test_root_window_inherits_sdl_hal_display_size(void)
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
    test_sdl_attach_rejects_null();
    test_sdl_attach_sets_default_display_contract();
    test_sdl_attach_prefers_hal_display_size();
    test_root_window_inherits_sdl_hal_display_size();
    return 0;
}
