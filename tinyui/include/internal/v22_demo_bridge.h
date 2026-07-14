/*
 * INTERNAL v2.2 demo/test migration bridge - not installed, not canonical.
 * Enabled only with TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE=1 (PRIVATE).
 * Scheduled for deletion in M4.
 */
#ifndef TINYUI_INTERNAL_V22_DEMO_BRIDGE_H
#define TINYUI_INTERNAL_V22_DEMO_BRIDGE_H

#if !defined(TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE) || !(TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE)
#error "internal/v22_demo_bridge.h requires TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE=1"
#endif

#include "internal/app_legacy.h"
#include "internal/widget_legacy.h"
#include "widgets/scroll_selector.h"
#include "theme/theme.h"

#include <stddef.h>

struct tinyui_app *tinyui_app_create(void);
int tinyui_app_run(struct tinyui_app *app, struct tinyui_window *window);
int tinyui_app_run_background(struct tinyui_app *app, struct tinyui_background *background);
int tinyui_app_set_window(struct tinyui_app *app, struct tinyui_window *window);
int tinyui_app_set_background(struct tinyui_app *app, struct tinyui_background *background);
struct tinyui_app_timer *tinyui_app_timer_create(struct tinyui_app *app);
int tinyui_app_timer_stop(struct tinyui_app_timer *timer);
int tinyui_app_timer_is_running(const struct tinyui_app_timer *timer);
void tinyui_app_timer_destroy(struct tinyui_app_timer *timer);
void tinyui_app_destroy(struct tinyui_app *app);
int tinyui_widget_set_pos(struct tinyui_widget *widget, int x, int y);
int tinyui_widget_set_size(struct tinyui_widget *widget, int width, int height);
int tinyui_widget_set_text(struct tinyui_widget *widget, const char *text);
int tinyui_widget_set_style_class(struct tinyui_widget *widget, const char *style_class);
int tinyui_widget_set_user_data(struct tinyui_widget *widget, void *user_data);
int tinyui_widget_set_bg_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_widget_set_text_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_widget_set_border_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_widget_set_radius(struct tinyui_widget *widget, int radius);
int tinyui_widget_set_padding(struct tinyui_widget *widget, int padding);
int tinyui_widget_set_center(struct tinyui_widget *widget);
int tinyui_widget_set_visible(struct tinyui_widget *widget, int visible);
int tinyui_widget_is_hidden(struct tinyui_widget *widget);
int tinyui_widget_set_opacity(struct tinyui_widget *widget, int opacity);
int tinyui_widget_set_selectable(struct tinyui_widget *widget, int selectable);
int tinyui_widget_set_selected(struct tinyui_widget *widget, int selected);
int tinyui_widget_set_corner(struct tinyui_widget *widget, int corner);
int tinyui_widget_set_enabled(struct tinyui_widget *widget, int enabled);
int tinyui_widget_set_flex_grow(struct tinyui_widget *widget, int grow);
int tinyui_widget_set_flex_new_track(struct tinyui_widget *widget, int new_track);
int tinyui_widget_set_flex_min_width(struct tinyui_widget *widget, int min_width);
int tinyui_widget_set_flex_min_height(struct tinyui_widget *widget, int min_height);
int tinyui_widget_set_flex_max_width(struct tinyui_widget *widget, int max_width);
int tinyui_widget_set_flex_max_height(struct tinyui_widget *widget, int max_height);
int tinyui_widget_set_ignore_layout(struct tinyui_widget *widget, int ignore_layout);
int tinyui_widget_remove_from_parent(struct tinyui_widget *widget);
int tinyui_widget_destroy(struct tinyui_widget *widget);
int tinyui_widget_get_x(const struct tinyui_widget *widget);
int tinyui_widget_get_y(const struct tinyui_widget *widget);
int tinyui_widget_get_width(const struct tinyui_widget *widget);
int tinyui_widget_get_height(const struct tinyui_widget *widget);
int tinyui_widget_get_visible(const struct tinyui_widget *widget);
int tinyui_widget_get_opacity(const struct tinyui_widget *widget);
int tinyui_widget_get_selectable(const struct tinyui_widget *widget);
int tinyui_widget_get_selected(const struct tinyui_widget *widget);
int tinyui_widget_get_corner(const struct tinyui_widget *widget);
int tinyui_widget_get_child_count(const struct tinyui_widget *widget);
int tinyui_widget_get_name_id(const struct tinyui_widget *widget);
enum tinyui_widget_type tinyui_widget_get_type(const struct tinyui_widget *widget);
struct tinyui_widget * tinyui_widget_get_parent(const struct tinyui_widget *widget);
struct tinyui_widget * tinyui_widget_get_first_child(const struct tinyui_widget *widget);
struct tinyui_widget * tinyui_widget_get_next_sibling(const struct tinyui_widget *widget);
struct tinyui_widget * tinyui_widget_get_root(const struct tinyui_widget *widget);
struct tinyui_widget * tinyui_widget_find_by_name_id(const struct tinyui_widget *root, int name_id);
int tinyui_focus_reset(struct tinyui_app *app);
int tinyui_focus_navigate(struct tinyui_app *app, enum tinyui_native_nav_dir dir);
int tinyui_app_switch_window(struct tinyui_app *app, struct tinyui_window *window, int mode, unsigned int duration_ms);
int tinyui_app_switch_background(struct tinyui_app *app, struct tinyui_background *background, int mode, unsigned int duration_ms);
int tinyui_app_timer_start(struct tinyui_app_timer *timer, unsigned int interval_ms, int repeat, tinyui_app_timer_cb_t callback, void *user_data);
int tinyui_widget_set_grid_cell(struct tinyui_widget *widget, int col, int row, int col_span, int row_span, enum tinyui_align x_align, enum tinyui_align y_align);
struct tinyui_point tinyui_widget_get_absolute_pos(const struct tinyui_widget *widget, struct tinyui_point point);
struct tinyui_point tinyui_widget_get_relative_pos(const struct tinyui_widget *widget, struct tinyui_point point);
struct tinyui_rect tinyui_rect_align(struct tinyui_rect parent, struct tinyui_rect child, enum tinyui_align x_align, enum tinyui_align y_align);
struct tinyui_rect tinyui_rect_center(struct tinyui_rect parent, struct tinyui_rect child);
int tinyui_vertical_grid_align_offset(struct tinyui_rect widget, int current_offset, int item_count, int item_height, int space);

int tinyui_timer_handler(void);
int tinyui_button_set_press(tinyui_obj_t *button, int pressed);
int tinyui_button_get_press(tinyui_obj_t *button, int *pressed);
int tinyui_tabel_show_keyboard(tinyui_obj_t *table);
int tinyui_q_r_code_set_text(tinyui_obj_t *qrcode, const char *text);
struct tinyui_qrcode *tinyui_q_r_code_init(struct tinyui_widget *parent, const char *id);
struct tinyui_arc *tinyui_arc_init(struct tinyui_widget *parent, const char *id);
struct tinyui_animation *tinyui_animation_init(struct tinyui_widget *parent, const char *id);
struct tinyui_clock *tinyui_clock_init(struct tinyui_widget *parent, const char *id);
struct tinyui_date_time *tinyui_date_time_init(struct tinyui_widget *parent, const char *id);
struct tinyui_gauge *tinyui_gauge_init(struct tinyui_widget *parent, const char *id);
struct tinyui_icon_slider *tinyui_icon_slider_init(struct tinyui_widget *parent, const char *id);
struct tinyui_message_box *tinyui_message_box_init(struct tinyui_widget *parent, const char *id);
struct tinyui_progress_wheel *tinyui_progress_wheel_init(struct tinyui_widget *parent, const char *id);
struct tinyui_radial_menu *tinyui_radial_menu_init(struct tinyui_widget *parent, const char *id);

#define tinyui_scroll_selecter tinyui_scroll_selector
#define tinyui_scroll_selecter_props tinyui_scroll_selector_props
#define tinyui_scroll_selecter_props_t tinyui_scroll_selector_props_t
#define tinyui_scroll_selecter_create tinyui_scroll_selector_create
#define tinyui_scroll_selecter_create_with_props tinyui_scroll_selector_create_with_props
#define tinyui_scroll_selecter_set_items tinyui_scroll_selector_set_items
#define tinyui_scroll_selecter_add_item tinyui_scroll_selector_add_item
#define tinyui_scroll_selecter_set_select_item_num tinyui_scroll_selector_set_select_item_num
#define tinyui_scroll_selecter_set_selected_index tinyui_scroll_selector_set_selected_index
#define tinyui_scroll_selecter_get_select_item_num tinyui_scroll_selector_get_select_item_num
#define tinyui_scroll_selecter_get_selected_index tinyui_scroll_selector_get_selected_index
#define tinyui_scroll_selecter_get_select_text tinyui_scroll_selector_get_select_text
#define tinyui_scroll_selecter_set_text_color tinyui_scroll_selector_set_text_color
#define tinyui_scroll_selecter_set_background_color tinyui_scroll_selector_set_background_color
#define tinyui_scroll_selecter_set_bg_color tinyui_scroll_selector_set_bg_color
#define tinyui_scroll_selecter_set_indicator_color tinyui_scroll_selector_set_indicator_color

struct tinyui_theme *tinyui_theme_create(void);
void tinyui_theme_destroy(struct tinyui_theme *theme);
int tinyui_theme_apply_to_widget(struct tinyui_theme *theme, struct tinyui_widget *widget);
int tinyui_app_set_theme(struct tinyui_app *app, struct tinyui_theme *theme);
int tinyui_theme_set_color(struct tinyui_theme *theme, int color_id, unsigned int rgb);
int tinyui_theme_set_metric(struct tinyui_theme *theme, int metric_id, int value);

#endif /* TINYUI_INTERNAL_V22_DEMO_BRIDGE_H */
