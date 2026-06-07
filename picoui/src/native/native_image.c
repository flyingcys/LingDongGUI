#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include <stdlib.h>

#define PICOUI_NATIVE_IMAGE_RENDER_MAX 32

struct picoui_native_image_render_state {
    const struct picoui_image *image;
    struct picoui_image_source *source;
    struct picoui_rect dirty_rect;
    int changed_pixels;
    int rendered;
};

static struct picoui_native_image_render_state
    g_picoui_native_image_render_states[PICOUI_NATIVE_IMAGE_RENDER_MAX];
static int g_picoui_native_image_fail_next_set_source;

static struct picoui_native_image_render_state *picoui_native_image_find_render_state(
    const struct picoui_image *image)
{
    int i;

    if (image == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_IMAGE_RENDER_MAX; ++i) {
        if (g_picoui_native_image_render_states[i].image == image) {
            return &g_picoui_native_image_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_image_render_state *picoui_native_image_alloc_render_state(
    const struct picoui_image *image)
{
    struct picoui_native_image_render_state *state;
    int i;

    state = picoui_native_image_find_render_state(image);
    if (state != 0) {
        return state;
    }

    if (image == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_IMAGE_RENDER_MAX; ++i) {
        if (g_picoui_native_image_render_states[i].image == 0) {
            g_picoui_native_image_render_states[i].image = image;
            g_picoui_native_image_render_states[i].source = 0;
            g_picoui_native_image_render_states[i].dirty_rect.x = 0;
            g_picoui_native_image_render_states[i].dirty_rect.y = 0;
            g_picoui_native_image_render_states[i].dirty_rect.width = 0;
            g_picoui_native_image_render_states[i].dirty_rect.height = 0;
            g_picoui_native_image_render_states[i].changed_pixels = 0;
            g_picoui_native_image_render_states[i].rendered = 0;
            return &g_picoui_native_image_render_states[i];
        }
    }

    return 0;
}

int picoui_native_image_set_source(struct picoui_image *image, struct picoui_image_source *source)
{
    struct picoui_backend_widget *backend;

    if (image == 0 || image->widget.backend_widget == 0
        || (source != 0 && source->img_tile == 0)) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)image->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_IMAGE) {
        return -1;
    }

    if (g_picoui_native_image_fail_next_set_source != 0) {
        g_picoui_native_image_fail_next_set_source = 0;
        return -1;
    }

    backend->image_source = source;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

void picoui_native_image_test_fail_next_set_source(void)
{
    g_picoui_native_image_fail_next_set_source = 1;
}

int picoui_native_image_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_image *image;
    struct picoui_native_image_render_state *state;
    unsigned int *scratch = 0;
    size_t scratch_count;
    int i;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_IMAGE
        || backend->host_widget == 0) {
        return -1;
    }

    image = (const struct picoui_image *)backend->host_widget;
    state = picoui_native_image_alloc_render_state(image);
    if (state == 0) {
        return -1;
    }

    state->source = image->source;
    state->dirty_rect.x = 0;
    state->dirty_rect.y = 0;
    state->dirty_rect.width = 0;
    state->dirty_rect.height = 0;
    state->changed_pixels = 0;

    if (state->source != 0 && image->widget.width > 0 && image->widget.height > 0) {
        scratch_count = (size_t)image->widget.width * (size_t)image->widget.height;
        scratch = calloc(scratch_count, sizeof(*scratch));
        if (scratch == 0) {
            return -1;
        }

        if (picoui_native_image_render_buffer(state->source,
                                              image->widget.bg_color,
                                              scratch,
                                              image->widget.width,
                                              image->widget.height,
                                              &state->dirty_rect)
            != 0) {
            free(scratch);
            return -1;
        }

        for (i = 0; i < (int)scratch_count; ++i) {
            if (scratch[i] != 0U) {
                state->changed_pixels++;
            }
        }

        free(scratch);
    }

    state->rendered = 1;
    return 0;
}

int picoui_native_image_get_rendered_source(const struct picoui_image *image,
                                            struct picoui_image_source **source)
{
    struct picoui_native_image_render_state *state;

    if (source == 0) {
        return -1;
    }

    state = picoui_native_image_find_render_state(image);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *source = state->source;
    return 0;
}

int picoui_native_image_get_rendered_dirty_rect(const struct picoui_image *image,
                                                struct picoui_rect *dirty_rect)
{
    struct picoui_native_image_render_state *state;

    if (dirty_rect == 0) {
        return -1;
    }

    state = picoui_native_image_find_render_state(image);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *dirty_rect = state->dirty_rect;
    return 0;
}

int picoui_native_image_get_rendered_changed_pixels(const struct picoui_image *image, int *count)
{
    struct picoui_native_image_render_state *state;

    if (count == 0) {
        return -1;
    }

    state = picoui_native_image_find_render_state(image);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *count = state->changed_pixels;
    return 0;
}
