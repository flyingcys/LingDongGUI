#include "internal.h"
#include "picoui/display.h"

#include <stddef.h>

static const struct picoui_display_config g_picoui_default_display_config = {
    .width = 480,
    .height = 320,
    .color_format = PICOUI_COLOR_FORMAT_RGB565,
    .buffer_height = 0,
    .user_data = NULL,
};

static int picoui_display_config_is_valid(const struct picoui_display_config *config)
{
    if (config == NULL) {
        return 0;
    }

    if (config->width <= 0 || config->height <= 0) {
        return 0;
    }

    if (config->color_format != PICOUI_COLOR_FORMAT_RGB565 &&
        config->color_format != PICOUI_COLOR_FORMAT_ARGB8888) {
        return 0;
    }

    if (config->buffer_height < 0) {
        return 0;
    }

    return 1;
}

static const struct picoui_display_config *picoui_display_resolve_config(
    const struct picoui_display_port_state *state)
{
    if (state == NULL || !picoui_display_config_is_valid(&state->config)) {
        return &g_picoui_default_display_config;
    }

    return &state->config;
}

int picoui_display_set_config(struct picoui_app *app, const struct picoui_display_config *config)
{
    if (app == NULL || !picoui_display_config_is_valid(config)) {
        return -1;
    }

    app->display_port.config = *config;
    return 0;
}

int picoui_display_get_config(const struct picoui_app *app, struct picoui_display_config *out_config)
{
    if (app == NULL || out_config == NULL) {
        return -1;
    }

    *out_config = *picoui_display_resolve_config(&app->display_port);
    return 0;
}

int picoui_display_set_flush_callback(struct picoui_app *app,
                                      picoui_display_flush_cb_t callback,
                                      void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->display_port.flush_callback = callback;
    app->display_port.flush_user_data = user_data;
    return 0;
}
