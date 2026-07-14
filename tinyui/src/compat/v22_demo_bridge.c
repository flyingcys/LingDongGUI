/*
 * INTERNAL v2.2 demo/test migration bridge implementation.
 * Forwards to renamed production symbols / canonical APIs. M4 deletes this TU.
 */
#include "internal/v22_demo_bridge.h"

#include "internal/runtime_internal_legacy_api.h"
#include "core/runtime.h"
#include "theme/theme.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct tinyui_app * tinyui_app_create(void)
{
    return tinyui_runtime_internal_app_create();
}

struct tinyui_app_timer * tinyui_app_timer_create(struct tinyui_app *app)
{
    return tinyui_runtime_internal_app_timer_create(app);
}

struct tinyui_widget * tinyui_widget_get_parent(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_parent(widget);
}

struct tinyui_widget * tinyui_widget_get_first_child(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_first_child(widget);
}

struct tinyui_widget * tinyui_widget_get_next_sibling(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_next_sibling(widget);
}

struct tinyui_widget * tinyui_widget_get_root(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_root(widget);
}

struct tinyui_widget * tinyui_widget_find_by_name_id(const struct tinyui_widget *root, int name_id)
{
    return tinyui_runtime_internal_widget_find_by_name_id(root, name_id);
}

int tinyui_app_run(struct tinyui_app *app, struct tinyui_window *window)
{
    return tinyui_runtime_internal_app_run(app, window);
}

int tinyui_app_run_background(struct tinyui_app *app, struct tinyui_background *background)
{
    return tinyui_runtime_internal_app_run_background(app, background);
}

int tinyui_app_set_window(struct tinyui_app *app, struct tinyui_window *window)
{
    return tinyui_runtime_internal_app_set_window(app, window);
}

int tinyui_app_set_background(struct tinyui_app *app, struct tinyui_background *background)
{
    return tinyui_runtime_internal_app_set_background(app, background);
}

int tinyui_app_switch_window(struct tinyui_app *app,
                             struct tinyui_window *window,
                             int mode,
                             unsigned int duration_ms)
{
    return tinyui_runtime_internal_app_switch_window(app, window, mode, duration_ms);
}

int tinyui_app_switch_background(struct tinyui_app *app,
                                 struct tinyui_background *background,
                                 int mode,
                                 unsigned int duration_ms)
{
    return tinyui_runtime_internal_app_switch_background(app, background, mode, duration_ms);
}

int tinyui_app_timer_start(struct tinyui_app_timer *timer,
                           unsigned int interval_ms,
                           int repeat,
                           tinyui_app_timer_cb_t callback,
                           void *user_data)
{
    return tinyui_runtime_internal_app_timer_start(timer, interval_ms, repeat, callback, user_data);
}

int tinyui_app_timer_stop(struct tinyui_app_timer *timer)
{
    return tinyui_runtime_internal_app_timer_stop(timer);
}

int tinyui_app_timer_is_running(const struct tinyui_app_timer *timer)
{
    return tinyui_runtime_internal_app_timer_is_running(timer);
}

void tinyui_app_timer_destroy(struct tinyui_app_timer *timer)
{
    tinyui_runtime_internal_app_timer_destroy(timer);
}

void tinyui_app_destroy(struct tinyui_app *app)
{
    tinyui_runtime_internal_app_destroy(app);
}

int tinyui_widget_set_pos(struct tinyui_widget *widget, int x, int y)
{
    return tinyui_runtime_internal_widget_set_pos(widget, x, y);
}

int tinyui_widget_set_size(struct tinyui_widget *widget, int width, int height)
{
    return tinyui_runtime_internal_widget_set_size(widget, width, height);
}

int tinyui_widget_set_text(struct tinyui_widget *widget, const char *text)
{
    return tinyui_runtime_internal_widget_set_text(widget, text);
}

int tinyui_widget_set_style_class(struct tinyui_widget *widget, const char *style_class)
{
    return tinyui_runtime_internal_widget_set_style_class(widget, style_class);
}

int tinyui_widget_set_user_data(struct tinyui_widget *widget, void *user_data)
{
    return tinyui_runtime_internal_widget_set_user_data(widget, user_data);
}

int tinyui_widget_set_bg_color(struct tinyui_widget *widget, unsigned int rgb)
{
    return tinyui_runtime_internal_widget_set_bg_color(widget, rgb);
}

int tinyui_widget_set_text_color(struct tinyui_widget *widget, unsigned int rgb)
{
    return tinyui_runtime_internal_widget_set_text_color(widget, rgb);
}

int tinyui_widget_set_border_color(struct tinyui_widget *widget, unsigned int rgb)
{
    return tinyui_runtime_internal_widget_set_border_color(widget, rgb);
}

int tinyui_widget_set_radius(struct tinyui_widget *widget, int radius)
{
    return tinyui_runtime_internal_widget_set_radius(widget, radius);
}

int tinyui_widget_set_padding(struct tinyui_widget *widget, int padding)
{
    return tinyui_runtime_internal_widget_set_padding(widget, padding);
}

int tinyui_widget_set_center(struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_set_center(widget);
}

int tinyui_widget_set_visible(struct tinyui_widget *widget, int visible)
{
    return tinyui_runtime_internal_widget_set_visible(widget, visible);
}

int tinyui_widget_is_hidden(struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_is_hidden(widget);
}

int tinyui_widget_set_opacity(struct tinyui_widget *widget, int opacity)
{
    return tinyui_runtime_internal_widget_set_opacity(widget, opacity);
}

int tinyui_widget_set_selectable(struct tinyui_widget *widget, int selectable)
{
    return tinyui_runtime_internal_widget_set_selectable(widget, selectable);
}

int tinyui_widget_set_selected(struct tinyui_widget *widget, int selected)
{
    return tinyui_runtime_internal_widget_set_selected(widget, selected);
}

int tinyui_widget_set_corner(struct tinyui_widget *widget, int corner)
{
    return tinyui_runtime_internal_widget_set_corner(widget, corner);
}

int tinyui_widget_set_enabled(struct tinyui_widget *widget, int enabled)
{
    return tinyui_runtime_internal_widget_set_enabled(widget, enabled);
}

int tinyui_widget_set_flex_grow(struct tinyui_widget *widget, int grow)
{
    return tinyui_runtime_internal_widget_set_flex_grow(widget, grow);
}

int tinyui_widget_set_flex_new_track(struct tinyui_widget *widget, int new_track)
{
    return tinyui_runtime_internal_widget_set_flex_new_track(widget, new_track);
}

int tinyui_widget_set_flex_min_width(struct tinyui_widget *widget, int min_width)
{
    return tinyui_runtime_internal_widget_set_flex_min_width(widget, min_width);
}

int tinyui_widget_set_flex_min_height(struct tinyui_widget *widget, int min_height)
{
    return tinyui_runtime_internal_widget_set_flex_min_height(widget, min_height);
}

int tinyui_widget_set_flex_max_width(struct tinyui_widget *widget, int max_width)
{
    return tinyui_runtime_internal_widget_set_flex_max_width(widget, max_width);
}

int tinyui_widget_set_flex_max_height(struct tinyui_widget *widget, int max_height)
{
    return tinyui_runtime_internal_widget_set_flex_max_height(widget, max_height);
}

int tinyui_widget_set_ignore_layout(struct tinyui_widget *widget, int ignore_layout)
{
    return tinyui_runtime_internal_widget_set_ignore_layout(widget, ignore_layout);
}

int tinyui_widget_set_grid_cell(struct tinyui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum tinyui_align x_align,
                                enum tinyui_align y_align)
{
    return tinyui_runtime_internal_widget_set_grid_cell(widget, col, row, col_span, row_span, x_align, y_align);
}

int tinyui_widget_remove_from_parent(struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_remove_from_parent(widget);
}

int tinyui_widget_destroy(struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_destroy(widget);
}

int tinyui_widget_get_x(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_x(widget);
}

int tinyui_widget_get_y(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_y(widget);
}

int tinyui_widget_get_width(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_width(widget);
}

int tinyui_widget_get_height(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_height(widget);
}

int tinyui_widget_get_visible(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_visible(widget);
}

int tinyui_widget_get_opacity(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_opacity(widget);
}

int tinyui_widget_get_selectable(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_selectable(widget);
}

int tinyui_widget_get_selected(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_selected(widget);
}

int tinyui_widget_get_corner(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_corner(widget);
}

int tinyui_widget_get_child_count(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_child_count(widget);
}

int tinyui_widget_get_name_id(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_name_id(widget);
}

enum tinyui_widget_type tinyui_widget_get_type(const struct tinyui_widget *widget)
{
    return tinyui_runtime_internal_widget_get_type(widget);
}

struct tinyui_point tinyui_widget_get_absolute_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point)
{
    return tinyui_runtime_internal_widget_get_absolute_pos(widget, point);
}

struct tinyui_point tinyui_widget_get_relative_pos(const struct tinyui_widget *widget,
                                                   struct tinyui_point point)
{
    return tinyui_runtime_internal_widget_get_relative_pos(widget, point);
}

struct tinyui_rect tinyui_rect_align(struct tinyui_rect parent,
                                     struct tinyui_rect child,
                                     enum tinyui_align x_align,
                                     enum tinyui_align y_align)
{
    return tinyui_rect_align(parent, child, x_align, y_align);
}

struct tinyui_rect tinyui_rect_center(struct tinyui_rect parent,
                                      struct tinyui_rect child)
{
    return tinyui_rect_center(parent, child);
}

int tinyui_vertical_grid_align_offset(struct tinyui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space)
{
    return tinyui_vertical_grid_align_offset(widget, current_offset, item_count, item_height, space);
}

int tinyui_timer_handler(void)
{
    return tinyui_runtime_internal_timer_handler();
}

int tinyui_button_set_press(tinyui_obj_t *button, int pressed)
{
    return tinyui_runtime_internal_button_set_press(button, pressed);
}

int tinyui_button_get_press(tinyui_obj_t *button, int *pressed)
{
    return tinyui_runtime_internal_button_get_press(button, pressed);
}

int tinyui_tabel_show_keyboard(tinyui_obj_t *table)
{
    return tinyui_runtime_internal_tabel_show_keyboard(table);
}

int tinyui_q_r_code_set_text(tinyui_obj_t *qrcode, const char *text)
{
    return tinyui_runtime_internal_q_r_code_set_text(qrcode, text);
}

struct tinyui_qrcode *tinyui_q_r_code_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_q_r_code_init(parent, id);
}

struct tinyui_arc *tinyui_arc_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_arc_init(parent, id);
}

struct tinyui_animation *tinyui_animation_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_animation_init(parent, id);
}

struct tinyui_clock *tinyui_clock_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_clock_init(parent, id);
}

struct tinyui_date_time *tinyui_date_time_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_date_time_init(parent, id);
}

struct tinyui_gauge *tinyui_gauge_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_gauge_init(parent, id);
}

struct tinyui_icon_slider *tinyui_icon_slider_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_icon_slider_init(parent, id);
}

struct tinyui_message_box *tinyui_message_box_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_message_box_init(parent, id);
}

struct tinyui_progress_wheel *tinyui_progress_wheel_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_progress_wheel_init(parent, id);
}

struct tinyui_radial_menu *tinyui_radial_menu_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_runtime_internal_radial_menu_init(parent, id);
}

struct tinyui_theme *tinyui_theme_create(void)
{
    struct tinyui_theme *theme = (struct tinyui_theme *)calloc(1, sizeof(*theme));
    return theme;
}

void tinyui_theme_destroy(struct tinyui_theme *theme)
{
    free(theme);
}

int tinyui_theme_apply_to_widget(struct tinyui_theme *theme, struct tinyui_widget *widget)
{
    if (theme == NULL || widget == NULL) {
        return -1;
    }
    if (tinyui_theme_set(theme) != TINYUI_OK) {
        return -1;
    }
    return tinyui_theme_apply((tinyui_obj_t *)widget) == TINYUI_OK ? 0 : -1;
}

int tinyui_app_set_theme(struct tinyui_app *app, struct tinyui_theme *theme)
{
    if (app == NULL || theme == NULL) {
        return -1;
    }
    (void)app;
    return tinyui_theme_set(theme) == TINYUI_OK ? 0 : -1;
}

int tinyui_theme_set_color(struct tinyui_theme *theme, int color_id, unsigned int rgb)
{
    if (theme == NULL || color_id < 0 || color_id >= (int)TINYUI_COLOR_COUNT) {
        return -1;
    }
    theme->colors[color_id] = rgb;
    return 0;
}

int tinyui_theme_set_metric(struct tinyui_theme *theme, int metric_id, int value)
{
    if (theme == NULL || metric_id < 0 || metric_id >= (int)TINYUI_METRIC_COUNT) {
        return -1;
    }
    theme->metrics[metric_id] = (int16_t)value;
    return 0;
}
