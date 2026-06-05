#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_IMAGE_RENDER_MAX 32

struct picoui_native_image_render_state {
    const struct picoui_image *image;
    struct picoui_image_source *source;
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
