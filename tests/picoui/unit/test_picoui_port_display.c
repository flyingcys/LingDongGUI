#include "picoui/picoui.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

static void test_default_display_config(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {0};

    assert(app != NULL);
    assert(picoui_display_get_config(app, &config) == 0);
    assert(config.width == 480);
    assert(config.height == 320);
    assert(config.color_format == PICOUI_COLOR_FORMAT_RGB565);
    assert(config.buffer_height == 0);
    assert(config.user_data == NULL);

    picoui_app_destroy(app);
}

static void test_display_config_round_trip(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {
        .width = 320,
        .height = 240,
        .color_format = PICOUI_COLOR_FORMAT_ARGB8888,
        .buffer_height = 32,
        .user_data = (void *)(uintptr_t)0x1234,
    };
    struct picoui_display_config readback = {0};

    assert(app != NULL);
    assert(picoui_display_set_config(app, &config) == 0);
    assert(picoui_display_get_config(app, &readback) == 0);
    assert(readback.width == 320);
    assert(readback.height == 240);
    assert(readback.color_format == PICOUI_COLOR_FORMAT_ARGB8888);
    assert(readback.buffer_height == 32);
    assert(readback.user_data == (void *)(uintptr_t)0x1234);

    picoui_app_destroy(app);
}

static void test_display_rejects_invalid_config(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_display_config config = {
        .width = 0,
        .height = 240,
        .color_format = PICOUI_COLOR_FORMAT_RGB565,
        .buffer_height = 0,
        .user_data = NULL,
    };

    assert(app != NULL);
    assert(picoui_display_set_config(app, NULL) == -1);
    assert(picoui_display_get_config(NULL, &config) == -1);
    assert(picoui_display_get_config(app, NULL) == -1);
    assert(picoui_display_set_config(app, &config) == -1);

    config.width = 320;
    config.height = -1;
    assert(picoui_display_set_config(app, &config) == -1);

    picoui_app_destroy(app);
}

int main(void)
{
    test_default_display_config();
    test_display_config_round_trip();
    test_display_rejects_invalid_config();
    return 0;
}
