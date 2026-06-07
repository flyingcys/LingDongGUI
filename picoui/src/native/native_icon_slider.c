#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#define PICOUI_NATIVE_ICON_SLIDER_RENDER_READY (1u << 23)
#define PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_SHIFT 24
#define PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_MASK (0x7Fu << PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_SHIFT)

static struct picoui_backend_widget *picoui_native_icon_slider_backend(
    const struct picoui_icon_slider *icon_slider)
{
    if (icon_slider == 0 || icon_slider->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)icon_slider->widget.backend_widget;
}

static int picoui_native_icon_slider_get_snapshot_index(const struct picoui_icon_slider *icon_slider,
                                                        int *index)
{
    const struct picoui_backend_widget *backend;
    unsigned int packed_index;

    if (index == 0) {
        return -1;
    }

    backend = picoui_native_icon_slider_backend(icon_slider);
    if (backend == 0 || (backend->runtime_evidence_flags & PICOUI_NATIVE_ICON_SLIDER_RENDER_READY) == 0) {
        return -1;
    }

    packed_index =
        (backend->runtime_evidence_flags & PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_MASK)
        >> PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_SHIFT;
    if (packed_index == 0) {
        *index = -1;
        return 0;
    }

    *index = (int)packed_index - 1;
    return 0;
}

static int picoui_native_icon_slider_apply_selected_index(struct picoui_icon_slider *icon_slider,
                                                          int index,
                                                          int emit_callback)
{
    struct picoui_backend_widget *backend;

    if (icon_slider == 0 || icon_slider->widget.backend_widget == 0
        || index < 0 || index >= icon_slider->item_count) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)icon_slider->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER) {
        return -1;
    }

    icon_slider->selected_index = index;
    backend->value = index;
    if (backend->ld_widget != 0) {
        (void)picoui_backend_icon_slider_set_selected_index(backend, index);
    }
    backend->data_model_epoch++;
    backend->last_data_source = emit_callback ? PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT
                                              : PICOUI_BACKEND_DATA_SOURCE_SETTER;
    if (emit_callback) {
        backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
        backend->dispatch_count++;
        backend->last_native_signal = PICOUI_NATIVE_SIGNAL_CLICKED_ITEM;
        backend->last_native_value = (uint64_t)index;
        if (icon_slider->cb != 0) {
            icon_slider->cb(icon_slider, index, icon_slider->user_data);
        }
    }
    return 0;
}

int picoui_native_icon_slider_set_selected_index(struct picoui_icon_slider *icon_slider, int index)
{
    return picoui_native_icon_slider_apply_selected_index(icon_slider, index, 0);
}

int picoui_native_icon_slider_select_index(struct picoui_icon_slider *icon_slider, int index)
{
    return picoui_native_icon_slider_apply_selected_index(icon_slider, index, 1);
}

int picoui_native_icon_slider_render(const struct picoui_backend_widget *backend)
{
    const struct picoui_icon_slider *icon_slider;
    unsigned int packed_index = 0;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_ICON_SLIDER
        || backend->host_widget == 0) {
        return -1;
    }

    icon_slider = (const struct picoui_icon_slider *)backend->host_widget;
    if (icon_slider->selected_index >= icon_slider->item_count) {
        return -1;
    }

    if (icon_slider->selected_index >= 0) {
        packed_index = (unsigned int)(icon_slider->selected_index + 1);
    }
    ((struct picoui_backend_widget *)backend)->runtime_evidence_flags =
        (backend->runtime_evidence_flags & ~PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_MASK)
        | PICOUI_NATIVE_ICON_SLIDER_RENDER_READY
        | (packed_index << PICOUI_NATIVE_ICON_SLIDER_RENDER_INDEX_SHIFT);
    return 0;
}

int picoui_native_icon_slider_get_rendered_selected_index(const struct picoui_icon_slider *icon_slider, int *index)
{
    return picoui_native_icon_slider_get_snapshot_index(icon_slider, index);
}

int picoui_native_icon_slider_get_rendered_selected_text(const struct picoui_icon_slider *icon_slider,
                                                         const char **text)
{
    int index;

    if (text == 0) {
        return -1;
    }

    if (picoui_native_icon_slider_get_snapshot_index(icon_slider, &index) != 0) {
        return -1;
    }

    if (index < 0) {
        *text = 0;
        return 0;
    }

    if (icon_slider == 0 || index >= icon_slider->item_count) {
        return -1;
    }

    *text = icon_slider->items[index].text;
    return 0;
}
