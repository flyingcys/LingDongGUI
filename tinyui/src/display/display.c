#include "internal.h"
#include "display/display.h"

#include <stddef.h>

static const struct tinyui_display_config g_tinyui_default_display_config = {
    .width = 480,
    .height = 320,
    .color_format = TINYUI_COLOR_FORMAT_RGB565,
    .buffer_height = 0,
    .user_data = NULL,
};
static struct tinyui_display_config g_tinyui_runner_default_display_config;

static int tinyui_display_config_is_valid(const struct tinyui_display_config *config)
{
    if (config == NULL) {
        return 0;
    }

    if (config->width <= 0 || config->height <= 0) {
        return 0;
    }

    if (config->color_format != TINYUI_COLOR_FORMAT_RGB565 &&
        config->color_format != TINYUI_COLOR_FORMAT_ARGB8888) {
        return 0;
    }

    if (config->buffer_height < 0) {
        return 0;
    }

    return 1;
}

static const struct tinyui_display_config *tinyui_display_resolve_config(
    const struct tinyui_display_port_state *state)
{
    if (state == NULL || !tinyui_display_config_is_valid(&state->config)) {
        if (tinyui_display_config_is_valid(&g_tinyui_runner_default_display_config)) {
            return &g_tinyui_runner_default_display_config;
        }
        return &g_tinyui_default_display_config;
    }

    return &state->config;
}

int tinyui_display_set_default_config(const struct tinyui_display_config *config)
{
    if (!tinyui_display_config_is_valid(config)) {
        return -1;
    }

    g_tinyui_runner_default_display_config = *config;
    return 0;
}

int tinyui_display_set_config(struct tinyui_app *app, const struct tinyui_display_config *config)
{
    if (app == NULL || !tinyui_display_config_is_valid(config)) {
        return -1;
    }

    app->display_port.config = *config;
    return 0;
}

int tinyui_display_get_config(const struct tinyui_app *app, struct tinyui_display_config *out_config)
{
    if (app == NULL || out_config == NULL) {
        return -1;
    }

    *out_config = *tinyui_display_resolve_config(&app->display_port);
    return 0;
}

int tinyui_display_set_flush_callback(struct tinyui_app *app,
                                      tinyui_display_flush_cb_t callback,
                                      void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->display_port.flush_callback = callback;
    app->display_port.flush_user_data = user_data;
    return 0;
}

int tinyui_display_set_present_callback(struct tinyui_app *app,
                                        tinyui_display_present_cb_t callback,
                                        void *user_data)
{
    if (app == NULL) {
        return -1;
    }

    app->display_port.present_callback = callback;
    app->display_port.present_user_data = user_data;
    return 0;
}
