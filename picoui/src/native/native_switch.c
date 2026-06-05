#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_SWITCH_RENDER_MAX 32

struct picoui_native_switch_render_state {
    const struct picoui_switch *sw;
    int checked;
    int rendered;
};

static struct picoui_native_switch_render_state
    g_picoui_native_switch_render_states[PICOUI_NATIVE_SWITCH_RENDER_MAX];

static struct picoui_native_switch_render_state *picoui_native_switch_find_render_state(
    const struct picoui_switch *sw)
{
    int i;

    if (sw == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_SWITCH_RENDER_MAX; ++i) {
        if (g_picoui_native_switch_render_states[i].sw == sw) {
            return &g_picoui_native_switch_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_switch_render_state *picoui_native_switch_alloc_render_state(
    const struct picoui_switch *sw)
{
    struct picoui_native_switch_render_state *state;
    int i;

    state = picoui_native_switch_find_render_state(sw);
    if (state != 0) {
        return state;
    }

    if (sw == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_SWITCH_RENDER_MAX; ++i) {
        if (g_picoui_native_switch_render_states[i].sw == 0) {
            g_picoui_native_switch_render_states[i].sw = sw;
            g_picoui_native_switch_render_states[i].checked = 0;
            g_picoui_native_switch_render_states[i].rendered = 0;
            return &g_picoui_native_switch_render_states[i];
        }
    }

    return 0;
}

int picoui_native_switch_set_checked(struct picoui_switch *sw, int checked)
{
    struct picoui_backend_widget *backend;
    int normalized_checked;

    if (sw == 0 || sw->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)sw->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_SWITCH) {
        return -1;
    }

    normalized_checked = checked != 0;
    backend->value = normalized_checked;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

int picoui_native_switch_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_switch *sw;
    struct picoui_native_switch_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_SWITCH
        || backend->host_widget == 0) {
        return -1;
    }

    sw = (const struct picoui_switch *)backend->host_widget;
    state = picoui_native_switch_alloc_render_state(sw);
    if (state == 0) {
        return -1;
    }

    state->checked = sw->checked != 0;
    state->rendered = 1;
    return 0;
}

int picoui_native_switch_get_rendered_checked(const struct picoui_switch *sw, int *checked)
{
    struct picoui_native_switch_render_state *state;

    if (checked == 0) {
        return -1;
    }

    state = picoui_native_switch_find_render_state(sw);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *checked = state->checked;
    return 0;
}
