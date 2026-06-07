#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include "../../../src/gui/ldScrollSelecter.h"

#define PICOUI_NATIVE_SCROLL_SELECTER_RENDER_READY (1u << 26)
#define PICOUI_NATIVE_SCROLL_SELECTER_FONT_HEIGHT 8

static struct picoui_backend_widget *picoui_native_scroll_selecter_backend(
    const struct picoui_scroll_selecter *scroll_selecter)
{
    if (scroll_selecter == 0 || scroll_selecter->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
}

static int picoui_native_scroll_selecter_item_step(const struct picoui_scroll_selecter *scroll_selecter)
{
    struct picoui_backend_widget *backend;
    const ldScrollSelecter_t *ld_scroll_selecter;
    int height;
    int item_space;

    if (scroll_selecter == 0) {
        return -1;
    }

    backend = picoui_native_scroll_selecter_backend(scroll_selecter);
    if (backend != 0 && backend->ld_widget != 0) {
        ld_scroll_selecter = (const ldScrollSelecter_t *)backend->ld_widget;
        if (ld_scroll_selecter->ptFont != 0) {
            return ld_scroll_selecter->itemSpace + ld_scroll_selecter->ptFont->tCharSize.iHeight;
        }
    }

    height = picoui_widget_get_height((const struct picoui_widget *)scroll_selecter);
    if (height <= 0) {
        return -1;
    }

    if (scroll_selecter->edit_mode != 0) {
        item_space = height - (PICOUI_NATIVE_SCROLL_SELECTER_FONT_HEIGHT * 3);
        if (item_space > 0) {
            item_space /= 2;
        } else {
            item_space = height - PICOUI_NATIVE_SCROLL_SELECTER_FONT_HEIGHT;
            if (item_space > 0) {
                item_space /= 2;
            } else {
                item_space = 0;
            }
        }
        return item_space + PICOUI_NATIVE_SCROLL_SELECTER_FONT_HEIGHT;
    }

    return height > 0 ? height : -1;
}

static int picoui_native_scroll_selecter_round_div_nearest(int numerator, int denominator)
{
    int abs_numerator;
    int rounded;

    if (denominator <= 0) {
        return 0;
    }

    if (numerator >= 0) {
        return (numerator + (denominator / 2)) / denominator;
    }

    abs_numerator = -numerator;
    rounded = (abs_numerator + (denominator / 2)) / denominator;
    return -rounded;
}

void picoui_native_scroll_selecter_reset_render_state(struct picoui_scroll_selecter *scroll_selecter)
{
    struct picoui_backend_widget *backend = picoui_native_scroll_selecter_backend(scroll_selecter);

    if (backend == 0) {
        return;
    }

    backend->runtime_evidence_flags &= ~PICOUI_NATIVE_SCROLL_SELECTER_RENDER_READY;
}

int picoui_native_scroll_selecter_set_selected_index(struct picoui_scroll_selecter *scroll_selecter, int index)
{
    struct picoui_backend_widget *backend;

    if (scroll_selecter == 0 || scroll_selecter->widget.backend_widget == 0
        || index < 0 || index >= scroll_selecter->item_count) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)scroll_selecter->widget.backend_widget;
    if (backend->kind != PICOUI_BACKEND_WIDGET_SCROLL_SELECTER) {
        return -1;
    }

    scroll_selecter->selected_index = index;
    backend->value = index;
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_SETTER;
    picoui_native_scroll_selecter_reset_render_state(scroll_selecter);
    return 0;
}

int picoui_native_scroll_selecter_apply_scroll_delta(struct picoui_scroll_selecter *scroll_selecter, int delta_y)
{
    int item_step;
    int offset_items;
    int next_index;
    struct picoui_backend_widget *backend;

    if (scroll_selecter == 0 || scroll_selecter->item_count <= 0) {
        return -1;
    }

    item_step = picoui_native_scroll_selecter_item_step(scroll_selecter);
    if (item_step <= 0) {
        return -1;
    }

    offset_items = picoui_native_scroll_selecter_round_div_nearest(-delta_y, item_step);
    next_index = scroll_selecter->selected_index;
    if (next_index < 0 || next_index >= scroll_selecter->item_count) {
        next_index = 0;
    }
    next_index += offset_items;
    if (next_index < 0) {
        next_index = 0;
    }
    if (next_index >= scroll_selecter->item_count) {
        next_index = scroll_selecter->item_count - 1;
    }

    if (picoui_native_scroll_selecter_set_selected_index(scroll_selecter, next_index) != 0) {
        return -1;
    }

    backend = picoui_native_scroll_selecter_backend(scroll_selecter);
    if (backend == 0) {
        return -1;
    }
    backend->last_native_value = (uint64_t)(uint32_t)delta_y;
    backend->runtime_evidence_flags |= PICOUI_NATIVE_SCROLL_SELECTER_RENDER_READY;
    return 0;
}

int picoui_native_scroll_selecter_render(const struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *mutable_backend;
    struct picoui_scroll_selecter *scroll_selecter;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_SCROLL_SELECTER
        || backend->host_widget == 0) {
        return -1;
    }

    mutable_backend = (struct picoui_backend_widget *)backend;
    scroll_selecter = (struct picoui_scroll_selecter *)backend->host_widget;

    if (picoui_backend_scroll_selecter_set_edit_mode(mutable_backend, scroll_selecter->edit_mode) != 0) {
        return -1;
    }
    if (scroll_selecter->selected_index >= 0
        && scroll_selecter->selected_index < scroll_selecter->item_count
        && picoui_backend_scroll_selecter_set_selected_index(mutable_backend,
                                                             scroll_selecter->selected_index) != 0) {
        return -1;
    }

    mutable_backend->runtime_evidence_flags |= PICOUI_NATIVE_SCROLL_SELECTER_RENDER_READY;
    return 0;
}

int picoui_native_scroll_selecter_get_rendered_selected_index(const struct picoui_scroll_selecter *scroll_selecter,
                                                              int *index)
{
    const struct picoui_backend_widget *backend = picoui_native_scroll_selecter_backend(scroll_selecter);

    if (index == 0 || backend == 0
        || (backend->runtime_evidence_flags & PICOUI_NATIVE_SCROLL_SELECTER_RENDER_READY) == 0) {
        return -1;
    }

    *index = scroll_selecter->selected_index;
    return 0;
}

int picoui_native_scroll_selecter_get_rendered_selected_text(const struct picoui_scroll_selecter *scroll_selecter,
                                                             const char **text)
{
    int index;

    if (text == 0) {
        return -1;
    }

    if (picoui_native_scroll_selecter_get_rendered_selected_index(scroll_selecter, &index) != 0) {
        return -1;
    }

    if (index < 0 || scroll_selecter == 0 || index >= scroll_selecter->item_count) {
        *text = 0;
        return 0;
    }

    *text = scroll_selecter->items[index].text;
    return 0;
}

int picoui_native_scroll_selecter_get_rendered_scroll_delta(const struct picoui_scroll_selecter *scroll_selecter,
                                                            int *delta_y)
{
    const struct picoui_backend_widget *backend = picoui_native_scroll_selecter_backend(scroll_selecter);

    if (delta_y == 0 || backend == 0
        || (backend->runtime_evidence_flags & PICOUI_NATIVE_SCROLL_SELECTER_RENDER_READY) == 0) {
        return -1;
    }

    *delta_y = (int)(int32_t)(uint32_t)backend->last_native_value;
    return 0;
}
