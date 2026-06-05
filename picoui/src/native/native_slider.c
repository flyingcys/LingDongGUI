#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_SLIDER_RENDER_MAX 32

struct picoui_native_slider_render_state {
    const struct picoui_slider *slider;
    int value;
    int rendered;
};

static struct picoui_native_slider_render_state
    g_picoui_native_slider_render_states[PICOUI_NATIVE_SLIDER_RENDER_MAX];

static struct picoui_native_slider_render_state *picoui_native_slider_find_render_state(
    const struct picoui_slider *slider)
{
    int i;

    if (slider == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_SLIDER_RENDER_MAX; ++i) {
        if (g_picoui_native_slider_render_states[i].slider == slider) {
            return &g_picoui_native_slider_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_slider_render_state *picoui_native_slider_alloc_render_state(
    const struct picoui_slider *slider)
{
    struct picoui_native_slider_render_state *state;
    int i;

    state = picoui_native_slider_find_render_state(slider);
    if (state != 0) {
        return state;
    }

    if (slider == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_SLIDER_RENDER_MAX; ++i) {
        if (g_picoui_native_slider_render_states[i].slider == 0) {
            g_picoui_native_slider_render_states[i].slider = slider;
            g_picoui_native_slider_render_states[i].value = 0;
            g_picoui_native_slider_render_states[i].rendered = 0;
            return &g_picoui_native_slider_render_states[i];
        }
    }

    return 0;
}

int picoui_native_slider_set_value(struct picoui_slider *slider, int value)
{
    struct picoui_backend_widget *backend;

    if (slider == 0 || slider->widget.backend_widget == 0
        || value < slider->min_value || value > slider->max_value) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)slider->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_SLIDER) {
        return -1;
    }

    backend->value = value;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

int picoui_native_slider_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_slider *slider;
    struct picoui_native_slider_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_SLIDER
        || backend->host_widget == 0) {
        return -1;
    }

    slider = (const struct picoui_slider *)backend->host_widget;
    state = picoui_native_slider_alloc_render_state(slider);
    if (state == 0) {
        return -1;
    }

    state->value = slider->value;
    state->rendered = 1;
    return 0;
}

int picoui_native_slider_get_rendered_value(const struct picoui_slider *slider, int *value)
{
    struct picoui_native_slider_render_state *state;

    if (value == 0) {
        return -1;
    }

    state = picoui_native_slider_find_render_state(slider);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *value = state->value;
    return 0;
}
