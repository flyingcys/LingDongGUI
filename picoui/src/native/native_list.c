#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_LIST_RENDER_MAX 32

struct picoui_native_list_render_state {
    const struct picoui_list *list;
    int selected_index;
    const char *selected_text;
    int rendered;
};

static struct picoui_native_list_render_state
    g_picoui_native_list_render_states[PICOUI_NATIVE_LIST_RENDER_MAX];

int picoui_native_list_select_index(struct picoui_list *list, int index);

static struct picoui_native_list_render_state *picoui_native_list_find_render_state(
    const struct picoui_list *list)
{
    int i;

    if (list == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_LIST_RENDER_MAX; ++i) {
        if (g_picoui_native_list_render_states[i].list == list) {
            return &g_picoui_native_list_render_states[i];
        }
    }

    return 0;
}

static struct picoui_native_list_render_state *picoui_native_list_alloc_render_state(
    const struct picoui_list *list)
{
    struct picoui_native_list_render_state *state;
    int i;

    state = picoui_native_list_find_render_state(list);
    if (state != 0) {
        return state;
    }

    if (list == 0) {
        return 0;
    }

    for (i = 0; i < PICOUI_NATIVE_LIST_RENDER_MAX; ++i) {
        if (g_picoui_native_list_render_states[i].list == 0) {
            g_picoui_native_list_render_states[i].list = list;
            g_picoui_native_list_render_states[i].selected_index = -1;
            g_picoui_native_list_render_states[i].selected_text = 0;
            g_picoui_native_list_render_states[i].rendered = 0;
            return &g_picoui_native_list_render_states[i];
        }
    }

    return 0;
}

int picoui_native_list_set_selected_index(struct picoui_list *list, int index)
{
    return picoui_native_list_select_index(list, index);
}

int picoui_native_list_select_index(struct picoui_list *list, int index)
{
    struct picoui_backend_widget *backend;
    int old_backend_value;
    unsigned int old_data_model_epoch;
    enum picoui_backend_data_value_source old_last_data_source;

    if (list == 0 || list->widget.backend_widget == 0 || index < 0 || index >= list->item_count) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    old_backend_value = backend->value;
    old_data_model_epoch = backend->data_model_epoch;
    old_last_data_source = backend->last_data_source;

    backend->value = index;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    if (backend->data_model_epoch <= old_data_model_epoch) {
        backend->value = old_backend_value;
        backend->data_model_epoch = old_data_model_epoch;
        backend->last_data_source = old_last_data_source;
        return -1;
    }
    return 0;
}

int picoui_native_list_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_list *list;
    struct picoui_native_list_render_state *state;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_LIST || backend->host_widget == 0) {
        return -1;
    }

    list = (const struct picoui_list *)backend->host_widget;
    state = picoui_native_list_alloc_render_state(list);
    if (state == 0) {
        return -1;
    }

    state->selected_index = list->selected_index;
    state->selected_text = 0;
    if (list->selected_index >= 0 && list->selected_index < list->item_count) {
        state->selected_text = list->items[list->selected_index].text;
    }
    state->rendered = 1;
    return 0;
}

int picoui_native_list_get_rendered_selected_index(const struct picoui_list *list, int *index)
{
    struct picoui_native_list_render_state *state;

    if (index == 0) {
        return -1;
    }

    state = picoui_native_list_find_render_state(list);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *index = state->selected_index;
    return 0;
}

int picoui_native_list_get_rendered_selected_text(const struct picoui_list *list, const char **text)
{
    struct picoui_native_list_render_state *state;

    if (text == 0) {
        return -1;
    }

    state = picoui_native_list_find_render_state(list);
    if (state == 0 || state->rendered == 0) {
        return -1;
    }

    *text = state->selected_text;
    return 0;
}
