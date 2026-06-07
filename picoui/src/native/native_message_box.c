#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include "../../../src/gui/ldBase.h"

#define PICOUI_NATIVE_MESSAGE_BOX_RENDER_READY (1u << 22)
#define PICOUI_NATIVE_MESSAGE_BOX_PADDING 10
#define PICOUI_NATIVE_MESSAGE_BOX_BUTTON_GAP 2

static struct picoui_backend_widget *picoui_native_message_box_backend(
    const struct picoui_message_box *box)
{
    if (box == 0 || box->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)box->widget.backend_widget;
}

static int picoui_native_message_box_compute_button_rect(const struct picoui_message_box *box,
                                                         int index,
                                                         int *left,
                                                         int *top,
                                                         int *width,
                                                         int *height)
{
    int dialog_width;
    int dialog_height;
    int title_height;
    int message_height;
    int button_width;
    int button_height;
    int button_top;
    int button_left;

    if (box == 0 || index < 0 || index >= box->button_count
        || left == 0 || top == 0 || width == 0 || height == 0) {
        return -1;
    }

    dialog_width = picoui_widget_get_width((const struct picoui_widget *)box);
    dialog_height = picoui_widget_get_height((const struct picoui_widget *)box);
    if (dialog_width <= 0 || dialog_height <= 0 || box->button_count <= 0) {
        return -1;
    }

    title_height = (dialog_height - (PICOUI_NATIVE_MESSAGE_BOX_PADDING * 2)) / 5;
    if (title_height < 0) {
        return -1;
    }
    message_height = title_height * 3;
    button_height = dialog_height - (PICOUI_NATIVE_MESSAGE_BOX_PADDING * 2) - title_height - message_height;
    button_width =
        ((dialog_width - (PICOUI_NATIVE_MESSAGE_BOX_PADDING * 2)) / box->button_count)
        - (PICOUI_NATIVE_MESSAGE_BOX_BUTTON_GAP * 2);
    if (button_height <= 0 || button_width <= 0) {
        return -1;
    }

    button_top = PICOUI_NATIVE_MESSAGE_BOX_PADDING + title_height + message_height;
    button_left =
        PICOUI_NATIVE_MESSAGE_BOX_PADDING
        + index * (button_width + (PICOUI_NATIVE_MESSAGE_BOX_BUTTON_GAP * 2))
        + PICOUI_NATIVE_MESSAGE_BOX_BUTTON_GAP;

    *left = button_left;
    *top = button_top;
    *width = button_width;
    *height = button_height;
    return 0;
}

static int picoui_native_message_box_is_render_ready(const struct picoui_message_box *box)
{
    const struct picoui_backend_widget *backend;

    backend = (const struct picoui_backend_widget *)picoui_native_message_box_backend(box);
    return backend != 0 && (backend->runtime_evidence_flags & PICOUI_NATIVE_MESSAGE_BOX_RENDER_READY) != 0;
}

static int picoui_native_message_box_point_to_button_index(const struct picoui_message_box *box,
                                                           int x,
                                                           int y)
{
    struct picoui_point origin;
    int local_x;
    int local_y;
    int index;

    if (box == 0 || box->button_count <= 0 || !box->widget.visible || !box->is_open) {
        return -1;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)box,
                                            (struct picoui_point){0, 0});
    local_x = x - origin.x;
    local_y = y - origin.y;

    for (index = 0; index < box->button_count; ++index) {
        int left;
        int top;
        int right;
        int bottom;
        int width;
        int height;

        if (picoui_native_message_box_compute_button_rect(box,
                                                          index,
                                                          &left,
                                                          &top,
                                                          &width,
                                                          &height) != 0) {
            return -1;
        }
        right = left + width;
        bottom = top + height;
        if (local_x >= left && local_x < right
            && local_y >= top && local_y < bottom) {
            return index;
        }
    }

    return -1;
}

int picoui_native_message_box_render(const struct picoui_backend_widget *backend)
{
    struct picoui_message_box *box;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_MESSAGE_BOX || backend->host_widget == 0) {
        return -1;
    }

    box = (struct picoui_message_box *)backend->host_widget;
    ((struct picoui_backend_widget *)backend)->open = box->is_open;
    ((struct picoui_backend_widget *)backend)->runtime_evidence_flags |=
        PICOUI_NATIVE_MESSAGE_BOX_RENDER_READY;
    return 0;
}

int picoui_native_message_box_get_rendered_open(const struct picoui_message_box *box, int *is_open)
{
    if (is_open == 0 || !picoui_native_message_box_is_render_ready(box)) {
        return -1;
    }

    *is_open = box->is_open != 0;
    return 0;
}

int picoui_native_message_box_get_rendered_button_count(const struct picoui_message_box *box,
                                                        int *button_count)
{
    if (button_count == 0 || !picoui_native_message_box_is_render_ready(box)) {
        return -1;
    }

    *button_count = box->button_count;
    return 0;
}

int picoui_native_message_box_get_button_center(const struct picoui_message_box *box,
                                                int index,
                                                int *x,
                                                int *y)
{
    struct picoui_point origin;
    int left;
    int top;
    int width;
    int height;

    if (x == 0 || y == 0 || !picoui_native_message_box_is_render_ready(box) || box == 0
        || index < 0 || index >= box->button_count || !box->is_open) {
        return -1;
    }

    if (picoui_native_message_box_compute_button_rect(box,
                                                      index,
                                                      &left,
                                                      &top,
                                                      &width,
                                                      &height) != 0) {
        return -1;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)box,
                                            (struct picoui_point){0, 0});
    *x = origin.x + left + (width / 2);
    *y = origin.y + top + (height / 2);
    return 0;
}

int picoui_native_message_box_handle_press(struct picoui_message_box *box, int x, int y)
{
    int index;

    if (box == 0 || !box->is_open) {
        return -1;
    }

    index = picoui_native_message_box_point_to_button_index(box, x, y);
    box->pressed_button_index = index;
    return 0;
}

int picoui_native_message_box_handle_release(struct picoui_message_box *box, int x, int y)
{
    struct picoui_backend_widget *backend;
    int released_index;

    if (box == 0) {
        return -1;
    }

    backend = picoui_native_message_box_backend(box);
    if (backend == 0) {
        return -1;
    }

    released_index = picoui_native_message_box_point_to_button_index(box, x, y);
    if (released_index < 0 || released_index != box->pressed_button_index) {
        box->pressed_button_index = -1;
        return 0;
    }

    box->pressed_button_index = -1;
    box->is_open = 0;
    backend->open = 0;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_RELEASED;
    backend->dispatch_count++;
    backend->last_native_signal = SIGNAL_RELEASE;
    backend->last_native_value = (uint64_t)released_index;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    if (box->on_confirm_indexed != 0) {
        box->on_confirm_indexed(box, released_index, box->on_confirm_indexed_user_data);
    }
    if (box->on_confirm != 0) {
        box->on_confirm(box, box->on_confirm_user_data);
    }
    return picoui_widget_set_visible(&box->widget, 0);
}
