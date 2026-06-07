#include "../backend/ldgui/backend.h"
#include "../core/internal.h"

#include <math.h>

#define PICOUI_NATIVE_RADIAL_MENU_RENDER_READY (1u << 25)

static struct picoui_backend_widget *picoui_native_radial_menu_backend(
    const struct picoui_radial_menu *radial_menu)
{
    if (radial_menu == 0 || radial_menu->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)radial_menu->widget.backend_widget;
}

static int picoui_native_radial_menu_is_render_ready(const struct picoui_radial_menu *radial_menu)
{
    const struct picoui_backend_widget *backend = picoui_native_radial_menu_backend(radial_menu);

    return backend != 0
        && (backend->runtime_evidence_flags & PICOUI_NATIVE_RADIAL_MENU_RENDER_READY) != 0;
}

static double picoui_native_radial_menu_displayed_angle_deg(const struct picoui_radial_menu *radial_menu, int index)
{
    double step;
    int selected_index;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return 90.0;
    }

    step = 360.0 / (double)radial_menu->item_count;
    selected_index = radial_menu->selected_index;
    if (selected_index < 0 || selected_index >= radial_menu->item_count) {
        selected_index = 0;
    }

    return 90.0 + step * (double)(index - selected_index);
}

static int picoui_native_radial_menu_origin(const struct picoui_radial_menu *radial_menu,
                                            int *origin_x,
                                            int *origin_y,
                                            int *radius_x,
                                            int *radius_y)
{
    struct picoui_point origin;
    int width;
    int height;

    if (radial_menu == 0 || origin_x == 0 || origin_y == 0 || radius_x == 0 || radius_y == 0) {
        return -1;
    }

    width = picoui_widget_get_width((const struct picoui_widget *)radial_menu);
    height = picoui_widget_get_height((const struct picoui_widget *)radial_menu);
    if (width <= 0 || height <= 0 || radial_menu->x_axis <= 0 || radial_menu->y_axis <= 0) {
        return -1;
    }

    origin = picoui_widget_get_absolute_pos((const struct picoui_widget *)radial_menu,
                                            (struct picoui_point){0, 0});
    *origin_x = origin.x + (width / 2);
    *origin_y = origin.y + (height / 2);
    *radius_x = radial_menu->x_axis / 2;
    *radius_y = radial_menu->y_axis / 2;
    if (*radius_x <= 0 || *radius_y <= 0) {
        return -1;
    }

    return 0;
}

static int picoui_native_radial_menu_point_to_index(const struct picoui_radial_menu *radial_menu, int x, int y)
{
    double theta_deg;
    double relative_deg;
    double step;
    double norm;
    int origin_x;
    int origin_y;
    int radius_x;
    int radius_y;
    int local_x;
    int local_y;
    int selected_index;
    int offset;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return -1;
    }

    if (picoui_native_radial_menu_origin(radial_menu,
                                         &origin_x,
                                         &origin_y,
                                         &radius_x,
                                         &radius_y) != 0) {
        return -1;
    }

    local_x = x - origin_x;
    local_y = y - origin_y;
    norm = ((double)local_x * (double)local_x) / ((double)radius_x * (double)radius_x)
         + ((double)local_y * (double)local_y) / ((double)radius_y * (double)radius_y);
    if (norm < 0.10 || norm > 2.25) {
        return -1;
    }

    theta_deg = atan2((double)local_y / (double)radius_y,
                      (double)(-local_x) / (double)radius_x) * (180.0 / M_PI);
    if (theta_deg < 0.0) {
        theta_deg += 360.0;
    }

    relative_deg = theta_deg - 90.0;
    while (relative_deg < 0.0) {
        relative_deg += 360.0;
    }
    while (relative_deg >= 360.0) {
        relative_deg -= 360.0;
    }

    step = 360.0 / (double)radial_menu->item_count;
    offset = (int)((relative_deg + (step / 2.0)) / step);
    if (offset >= radial_menu->item_count) {
        offset -= radial_menu->item_count;
    }

    selected_index = radial_menu->selected_index;
    if (selected_index < 0 || selected_index >= radial_menu->item_count) {
        selected_index = 0;
    }

    return (selected_index + offset) % radial_menu->item_count;
}

int picoui_native_radial_menu_render(const struct picoui_backend_widget *backend)
{
    struct picoui_backend_widget *mutable_backend;
    struct picoui_radial_menu *radial_menu;

    if (backend == 0 || backend->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU || backend->host_widget == 0) {
        return -1;
    }

    mutable_backend = (struct picoui_backend_widget *)backend;
    radial_menu = (struct picoui_radial_menu *)backend->host_widget;
    if (radial_menu->item_count > 0
        && radial_menu->selected_index >= 0
        && radial_menu->selected_index < radial_menu->item_count
        && picoui_backend_radial_menu_set_selected_index(mutable_backend,
                                                         radial_menu->selected_index) != 0) {
        return -1;
    }

    mutable_backend->value = radial_menu->selected_index;
    mutable_backend->runtime_evidence_flags |= PICOUI_NATIVE_RADIAL_MENU_RENDER_READY;
    return 0;
}

int picoui_native_radial_menu_get_rendered_state(const struct picoui_radial_menu *radial_menu,
                                                 int *item_count,
                                                 int *selected_index)
{
    if (radial_menu == 0 || item_count == 0 || selected_index == 0
        || !picoui_native_radial_menu_is_render_ready(radial_menu)) {
        return -1;
    }

    *item_count = radial_menu->item_count;
    *selected_index = radial_menu->selected_index;
    return 0;
}

int picoui_native_radial_menu_get_item_center(const struct picoui_radial_menu *radial_menu,
                                              int index,
                                              int *x,
                                              int *y)
{
    double angle_deg;
    double angle_rad;
    int origin_x;
    int origin_y;
    int radius_x;
    int radius_y;

    if (radial_menu == 0 || x == 0 || y == 0 || index < 0 || index >= radial_menu->item_count
        || !picoui_native_radial_menu_is_render_ready(radial_menu)) {
        return -1;
    }

    if (picoui_native_radial_menu_origin(radial_menu,
                                         &origin_x,
                                         &origin_y,
                                         &radius_x,
                                         &radius_y) != 0) {
        return -1;
    }

    angle_deg = picoui_native_radial_menu_displayed_angle_deg(radial_menu, index);
    angle_rad = angle_deg * (M_PI / 180.0);

    *x = origin_x - (int)lround(cos(angle_rad) * (double)radius_x);
    *y = origin_y + (int)lround(sin(angle_rad) * (double)radius_y);
    return 0;
}

int picoui_native_radial_menu_select_point(struct picoui_radial_menu *radial_menu, int x, int y)
{
    struct picoui_backend_widget *backend;
    int next_index;
    int previous_index;

    if (radial_menu == 0) {
        return -1;
    }

    backend = picoui_native_radial_menu_backend(radial_menu);
    if (backend == 0) {
        return -1;
    }

    next_index = picoui_native_radial_menu_point_to_index(radial_menu, x, y);
    if (next_index < 0 || next_index >= radial_menu->item_count) {
        return -1;
    }

    previous_index = radial_menu->selected_index;
    if (picoui_backend_widget_claim_focus(backend) != 0) {
        return -1;
    }
    if (picoui_radial_menu_set_selected_index(radial_menu, next_index) != 0) {
        return -1;
    }

    if (previous_index == next_index) {
        return 0;
    }

    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    backend->last_native_signal = PICOUI_NATIVE_SIGNAL_CLICKED_ITEM;
    backend->last_native_value = (uint64_t)next_index;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    if (radial_menu->cb != 0) {
        radial_menu->cb(radial_menu, next_index, radial_menu->user_data);
    }
    return 0;
}
