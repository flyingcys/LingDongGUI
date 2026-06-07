#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_ARC_RENDER_MAX 32

struct picoui_native_arc_render_state {
    const struct picoui_arc *arc;
    int value;
    float bg_start_angle;
    float bg_end_angle;
    float fg_end_angle;
    int rendered;
};

static struct picoui_native_arc_render_state
    g_picoui_native_arc_render_states[PICOUI_NATIVE_ARC_RENDER_MAX];

static struct picoui_native_arc_render_state *picoui_native_arc_find_render_state(
    const struct picoui_arc *arc)
{
    int i;

    if (arc == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_ARC_RENDER_MAX; ++i) {
        if (g_picoui_native_arc_render_states[i].arc == arc) {
            return &g_picoui_native_arc_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_arc_render_state *picoui_native_arc_alloc_render_state(
    const struct picoui_arc *arc)
{
    struct picoui_native_arc_render_state *state;
    int i;

    state = picoui_native_arc_find_render_state(arc);
    if (state != 0) {
        return state;
    }

    if (arc == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_ARC_RENDER_MAX; ++i) {
        if (g_picoui_native_arc_render_states[i].arc == 0) {
            g_picoui_native_arc_render_states[i].arc = arc;
            g_picoui_native_arc_render_states[i].value = 0;
            g_picoui_native_arc_render_states[i].bg_start_angle = 0.0f;
            g_picoui_native_arc_render_states[i].bg_end_angle = 0.0f;
            g_picoui_native_arc_render_states[i].fg_end_angle = 0.0f;
            g_picoui_native_arc_render_states[i].rendered = 0;
            return &g_picoui_native_arc_render_states[i];
        }
    }

    return 0;
}

int picoui_native_arc_set_value(struct picoui_arc *arc, int value)
{
    struct picoui_backend_widget *backend;

    if (arc == 0 || arc->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)arc->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_ARC) {
        return -1;
    }

    backend->value = value;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

int picoui_native_arc_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_arc *arc;
    struct picoui_native_arc_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_ARC
        || backend->host_widget == 0) {
        return -1;
    }

    arc = (const struct picoui_arc *)backend->host_widget;
    state = picoui_native_arc_alloc_render_state(arc);
    if (state == 0) {
        return -1;
    }

    state->value = backend->value;
    state->bg_start_angle = arc->bg_start_angle;
    state->bg_end_angle = arc->bg_end_angle;
    state->fg_end_angle = arc->fg_end_angle;
    state->rendered = 1;
    return 0;
}

int picoui_native_arc_get_rendered_value(const struct picoui_arc *arc, int *value)
{
    struct picoui_native_arc_render_state *state;

    if (value == 0) {
        return -1;
    }

    state = picoui_native_arc_find_render_state(arc);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *value = state->value;
    return 0;
}

int picoui_native_arc_get_rendered_angles(const struct picoui_arc *arc,
                                          float *bg_start_angle,
                                          float *bg_end_angle,
                                          float *fg_end_angle)
{
    struct picoui_native_arc_render_state *state;

    if (bg_start_angle == 0 || bg_end_angle == 0 || fg_end_angle == 0) {
        return -1;
    }

    state = picoui_native_arc_find_render_state(arc);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *bg_start_angle = state->bg_start_angle;
    *bg_end_angle = state->bg_end_angle;
    *fg_end_angle = state->fg_end_angle;
    return 0;
}
