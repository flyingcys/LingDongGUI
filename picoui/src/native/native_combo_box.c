#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_COMBO_BOX_RENDER_MAX 32

struct picoui_native_combo_box_render_state {
    const struct picoui_combo_box *combo_box;
    int selected_index;
    const char *selected_text;
    int is_open;
    int rendered;
};

static struct picoui_native_combo_box_render_state
    g_picoui_native_combo_box_render_states[PICOUI_NATIVE_COMBO_BOX_RENDER_MAX];

int picoui_combo_box_apply_open_state(struct picoui_combo_box *combo_box, int is_open);
int picoui_combo_box_apply_selected_index_internal(struct picoui_combo_box *combo_box,
                                                   int index,
                                                   int emit_callback);

static struct picoui_native_combo_box_render_state *picoui_native_combo_box_find_render_state(
    const struct picoui_combo_box *combo_box)
{
    int i;

    if (combo_box == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_COMBO_BOX_RENDER_MAX; ++i) {
        if (g_picoui_native_combo_box_render_states[i].combo_box == combo_box) {
            return &g_picoui_native_combo_box_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_combo_box_render_state *picoui_native_combo_box_alloc_render_state(
    const struct picoui_combo_box *combo_box)
{
    struct picoui_native_combo_box_render_state *state;
    int i;

    state = picoui_native_combo_box_find_render_state(combo_box);
    if (state != 0) {
        return state;
    }

    if (combo_box == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_COMBO_BOX_RENDER_MAX; ++i) {
        if (g_picoui_native_combo_box_render_states[i].combo_box == 0) {
            g_picoui_native_combo_box_render_states[i].combo_box = combo_box;
            g_picoui_native_combo_box_render_states[i].selected_index = -1;
            g_picoui_native_combo_box_render_states[i].selected_text = 0;
            g_picoui_native_combo_box_render_states[i].is_open = 0;
            g_picoui_native_combo_box_render_states[i].rendered = 0;
            return &g_picoui_native_combo_box_render_states[i];
        }
    }

    return 0;
}

int picoui_native_combo_box_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_combo_box *combo_box;
    struct picoui_native_combo_box_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_COMBO_BOX
        || backend->host_widget == 0) {
        return -1;
    }

    combo_box = (const struct picoui_combo_box *)backend->host_widget;
    state = picoui_native_combo_box_alloc_render_state(combo_box);
    if (state == 0) {
        return -1;
    }

    state->selected_index = combo_box->selected_index;
    state->selected_text = 0;
    if (combo_box->selected_index >= 0 && combo_box->selected_index < combo_box->item_count) {
        state->selected_text = combo_box->items[combo_box->selected_index].text;
    }
    state->is_open = backend->open ? 1 : 0;
    state->rendered = 1;
    return 0;
}

int picoui_native_combo_box_open(struct picoui_combo_box *combo_box)
{
    return picoui_combo_box_apply_open_state(combo_box, 1);
}

int picoui_native_combo_box_select_index(struct picoui_combo_box *combo_box, int index)
{
    return picoui_combo_box_apply_selected_index_internal(combo_box, index, 1);
}

int picoui_native_combo_box_get_rendered_selected_index(const struct picoui_combo_box *combo_box,
                                                        int *index)
{
    struct picoui_native_combo_box_render_state *state;

    if (index == 0) {
        return -1;
    }

    state = picoui_native_combo_box_find_render_state(combo_box);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *index = state->selected_index;
    return 0;
}

int picoui_native_combo_box_get_rendered_selected_text(const struct picoui_combo_box *combo_box,
                                                       const char **text)
{
    struct picoui_native_combo_box_render_state *state;

    if (text == 0) {
        return -1;
    }

    state = picoui_native_combo_box_find_render_state(combo_box);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *text = state->selected_text;
    return 0;
}

int picoui_native_combo_box_get_rendered_open(const struct picoui_combo_box *combo_box, int *is_open)
{
    struct picoui_native_combo_box_render_state *state;

    if (is_open == 0) {
        return -1;
    }

    state = picoui_native_combo_box_find_render_state(combo_box);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *is_open = state->is_open;
    return 0;
}
