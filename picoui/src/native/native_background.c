#include "../backend/ldgui/backend.h"
#include "../core/internal.h"
#include "ldBase.h"
#include "ldWindow.h"

#define PICOUI_NATIVE_BACKGROUND_RENDER_MAX 16

struct picoui_native_background_render_state {
    const struct picoui_background *background;
    int width;
    int height;
    unsigned int rgb;
    struct picoui_image_source *source;
    int rendered;
};

static struct picoui_native_background_render_state
    g_picoui_native_background_render_states[PICOUI_NATIVE_BACKGROUND_RENDER_MAX];

static struct picoui_native_background_render_state *picoui_native_background_find_render_state(
    const struct picoui_background *background)
{
    int i;

    if (background == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_BACKGROUND_RENDER_MAX; ++i) {
        if (g_picoui_native_background_render_states[i].background == background) {
            return &g_picoui_native_background_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_background_render_state *picoui_native_background_alloc_render_state(
    const struct picoui_background *background)
{
    struct picoui_native_background_render_state *state;
    int i;

    state = picoui_native_background_find_render_state(background);
    if (state != 0) {
        return state;
    }

    if (background == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_BACKGROUND_RENDER_MAX; ++i) {
        if (g_picoui_native_background_render_states[i].background == 0) {
            g_picoui_native_background_render_states[i].background = background;
            g_picoui_native_background_render_states[i].width = 0;
            g_picoui_native_background_render_states[i].height = 0;
            g_picoui_native_background_render_states[i].rgb = 0;
            g_picoui_native_background_render_states[i].source = 0;
            g_picoui_native_background_render_states[i].rendered = 0;
            return &g_picoui_native_background_render_states[i];
        }
    }

    return 0;
}

static unsigned int picoui_native_background_ld_color_to_rgb(ldColor color)
{
    unsigned int red = ((unsigned int)color >> 11) & 0x1FU;
    unsigned int green = ((unsigned int)color >> 5) & 0x3FU;
    unsigned int blue = (unsigned int)color & 0x1FU;

    red = (red * 255U) / 31U;
    green = (green * 255U) / 63U;
    blue = (blue * 255U) / 31U;
    return (red << 16) | (green << 8) | blue;
}

int picoui_native_background_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_background *background;
    struct picoui_native_background_render_state *state;
    const ldWindow_t *ld_window;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_BACKGROUND
        || backend->host_widget == 0 || backend->ld_widget == 0) {
        return -1;
    }

    background = (const struct picoui_background *)backend->host_widget;
    state = picoui_native_background_alloc_render_state(background);
    if (state == 0) {
        return -1;
    }

    ld_window = (const ldWindow_t *)backend->ld_widget;
    state->width = ldBaseGetWidth((ldBase_t *)ld_window);
    state->height = ldBaseGetHeight((ldBase_t *)ld_window);
    state->rgb = picoui_native_background_ld_color_to_rgb(ldWindowGetColor((ldWindow_t *)ld_window));
    state->source = backend->image_source;
    state->rendered = 1;
    return 0;
}

int picoui_native_background_get_rendered_size(const struct picoui_background *background,
                                               int *width,
                                               int *height)
{
    struct picoui_native_background_render_state *state;

    if (width == 0 || height == 0) {
        return -1;
    }

    state = picoui_native_background_find_render_state(background);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *width = state->width;
    *height = state->height;
    return 0;
}

int picoui_native_background_get_rendered_color(const struct picoui_background *background,
                                                unsigned int *rgb)
{
    struct picoui_native_background_render_state *state;

    if (rgb == 0) {
        return -1;
    }

    state = picoui_native_background_find_render_state(background);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *rgb = state->rgb;
    return 0;
}

int picoui_native_background_get_rendered_source(const struct picoui_background *background,
                                                 struct picoui_image_source **source)
{
    struct picoui_native_background_render_state *state;

    if (source == 0) {
        return -1;
    }

    state = picoui_native_background_find_render_state(background);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *source = state->source;
    return 0;
}
