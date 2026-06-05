#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_CHECKBOX_RENDER_MAX 32

struct picoui_native_checkbox_render_state {
    const struct picoui_checkbox *checkbox;
    int checked;
    int rendered;
};

static struct picoui_native_checkbox_render_state
    g_picoui_native_checkbox_render_states[PICOUI_NATIVE_CHECKBOX_RENDER_MAX];

static struct picoui_native_checkbox_render_state *picoui_native_checkbox_find_render_state(
    const struct picoui_checkbox *checkbox)
{
    int i;

    if (checkbox == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_CHECKBOX_RENDER_MAX; ++i) {
        if (g_picoui_native_checkbox_render_states[i].checkbox == checkbox) {
            return &g_picoui_native_checkbox_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_checkbox_render_state *picoui_native_checkbox_alloc_render_state(
    const struct picoui_checkbox *checkbox)
{
    struct picoui_native_checkbox_render_state *state;
    int i;

    state = picoui_native_checkbox_find_render_state(checkbox);
    if (state != 0) {
        return state;
    }

    if (checkbox == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_CHECKBOX_RENDER_MAX; ++i) {
        if (g_picoui_native_checkbox_render_states[i].checkbox == 0) {
            g_picoui_native_checkbox_render_states[i].checkbox = checkbox;
            g_picoui_native_checkbox_render_states[i].checked = 0;
            g_picoui_native_checkbox_render_states[i].rendered = 0;
            return &g_picoui_native_checkbox_render_states[i];
        }
    }

    return 0;
}

int picoui_native_checkbox_set_checked(struct picoui_checkbox *checkbox, int checked)
{
    struct picoui_backend_widget *backend;
    int normalized_checked;

    if (checkbox == 0 || checkbox->widget.backend_widget == 0) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)checkbox->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_CHECKBOX) {
        return -1;
    }

    normalized_checked = checked != 0;
    backend->value = normalized_checked;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    return 0;
}

int picoui_native_checkbox_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_checkbox *checkbox;
    struct picoui_native_checkbox_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_CHECKBOX
        || backend->host_widget == 0) {
        return -1;
    }

    checkbox = (const struct picoui_checkbox *)backend->host_widget;
    state = picoui_native_checkbox_alloc_render_state(checkbox);
    if (state == 0) {
        return -1;
    }

    state->checked = checkbox->checked != 0;
    state->rendered = 1;
    return 0;
}

int picoui_native_checkbox_get_rendered_checked(const struct picoui_checkbox *checkbox, int *checked)
{
    struct picoui_native_checkbox_render_state *state;

    if (checked == 0) {
        return -1;
    }

    state = picoui_native_checkbox_find_render_state(checkbox);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *checked = state->checked;
    return 0;
}
