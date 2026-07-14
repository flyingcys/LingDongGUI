/*
 * Production-side renamed legacy ABI (Task 7).
 * Old tinyui_app_ and tinyui_widget_ names are only re-exported by v22_demo_bridge.
 */
#ifndef TINYUI_RUNTIME_INTERNAL_LEGACY_API_H
#define TINYUI_RUNTIME_INTERNAL_LEGACY_API_H

#include <stdint.h>

/* Full enum definitions (C++ forbids bare enum forward declarations). */
#include "layout/layout.h"
#include "extensions/ldgui_native.h"
#include "internal/widget_legacy.h"

struct tinyui_app;
struct tinyui_app_timer;
struct tinyui_background;
struct tinyui_window;
struct tinyui_widget;
struct tinyui_point;
struct tinyui_rect;
struct tinyui_arc;
struct tinyui_animation;
struct tinyui_clock;
struct tinyui_date_time;
struct tinyui_gauge;
struct tinyui_icon_slider;
struct tinyui_message_box;
struct tinyui_progress_wheel;
struct tinyui_radial_menu;
struct tinyui_qrcode;

typedef void (*tinyui_app_timer_cb_t)(struct tinyui_app *app,
                                      struct tinyui_app_timer *timer,
                                      void *user_data);

/* app */
struct tinyui_app *tinyui_runtime_internal_app_create(void);
int tinyui_runtime_internal_app_run(struct tinyui_app *app, struct tinyui_window *window);
int tinyui_runtime_internal_app_run_background(struct tinyui_app *app, struct tinyui_background *background);
int tinyui_runtime_internal_app_set_window(struct tinyui_app *app, struct tinyui_window *window);
int tinyui_runtime_internal_app_set_background(struct tinyui_app *app, struct tinyui_background *background);
int tinyui_runtime_internal_app_switch_window(struct tinyui_app *app, struct tinyui_window *window, int mode, unsigned int duration_ms);
int tinyui_runtime_internal_app_switch_background(struct tinyui_app *app, struct tinyui_background *background, int mode, unsigned int duration_ms);
struct tinyui_app_timer *tinyui_runtime_internal_app_timer_create(struct tinyui_app *app);
int tinyui_runtime_internal_app_timer_start(struct tinyui_app_timer *timer, unsigned int interval_ms, int repeat, tinyui_app_timer_cb_t callback, void *user_data);
int tinyui_runtime_internal_app_timer_stop(struct tinyui_app_timer *timer);
int tinyui_runtime_internal_app_timer_is_running(const struct tinyui_app_timer *timer);
void tinyui_runtime_internal_app_timer_destroy(struct tinyui_app_timer *timer);
void tinyui_runtime_internal_app_destroy(struct tinyui_app *app);
void tinyui_runtime_internal_app_pump_timers(struct tinyui_app *app, unsigned int now_ticks);
struct tinyui_app *tinyui_runtime_internal_app_current(void);
void tinyui_runtime_internal_app_register_host(struct tinyui_app *app, struct tinyui_widget *w);
void tinyui_runtime_internal_app_unregister_host(struct tinyui_app *app, struct tinyui_widget *w);
struct tinyui_widget *tinyui_runtime_internal_app_lookup_host(const struct tinyui_app *app, uint16_t name_id);
uint16_t tinyui_runtime_internal_app_alloc_name_id(struct tinyui_app *app);
void tinyui_runtime_internal_app_free_name_id(struct tinyui_app *app, uint16_t id);

/* widget public-legacy surface */
int tinyui_runtime_internal_widget_set_pos(struct tinyui_widget *widget, int x, int y);
int tinyui_runtime_internal_widget_set_size(struct tinyui_widget *widget, int width, int height);
int tinyui_runtime_internal_widget_set_text(struct tinyui_widget *widget, const char *text);
int tinyui_runtime_internal_widget_set_style_class(struct tinyui_widget *widget, const char *style_class);
int tinyui_runtime_internal_widget_set_user_data(struct tinyui_widget *widget, void *user_data);
int tinyui_runtime_internal_widget_set_bg_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_runtime_internal_widget_set_text_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_runtime_internal_widget_set_border_color(struct tinyui_widget *widget, unsigned int rgb);
int tinyui_runtime_internal_widget_set_radius(struct tinyui_widget *widget, int radius);
int tinyui_runtime_internal_widget_set_padding(struct tinyui_widget *widget, int padding);
int tinyui_runtime_internal_widget_set_center(struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_set_visible(struct tinyui_widget *widget, int visible);
int tinyui_runtime_internal_widget_is_hidden(struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_set_opacity(struct tinyui_widget *widget, int opacity);
int tinyui_runtime_internal_widget_set_selectable(struct tinyui_widget *widget, int selectable);
int tinyui_runtime_internal_widget_set_selected(struct tinyui_widget *widget, int selected);
int tinyui_runtime_internal_widget_set_corner(struct tinyui_widget *widget, int corner);
int tinyui_runtime_internal_widget_set_enabled(struct tinyui_widget *widget, int enabled);
int tinyui_runtime_internal_widget_set_flex_grow(struct tinyui_widget *widget, int grow);
int tinyui_runtime_internal_widget_set_flex_new_track(struct tinyui_widget *widget, int new_track);
int tinyui_runtime_internal_widget_set_flex_min_width(struct tinyui_widget *widget, int min_width);
int tinyui_runtime_internal_widget_set_flex_min_height(struct tinyui_widget *widget, int min_height);
int tinyui_runtime_internal_widget_set_flex_max_width(struct tinyui_widget *widget, int max_width);
int tinyui_runtime_internal_widget_set_flex_max_height(struct tinyui_widget *widget, int max_height);
int tinyui_runtime_internal_widget_set_ignore_layout(struct tinyui_widget *widget, int ignore_layout);
int tinyui_runtime_internal_widget_set_grid_cell(struct tinyui_widget *widget, int col, int row, int col_span, int row_span, enum tinyui_align x_align, enum tinyui_align y_align);
int tinyui_runtime_internal_widget_remove_from_parent(struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_destroy(struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_x(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_y(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_width(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_height(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_visible(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_opacity(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_selectable(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_selected(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_corner(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_child_count(const struct tinyui_widget *widget);
int tinyui_runtime_internal_widget_get_name_id(const struct tinyui_widget *widget);
enum tinyui_widget_type tinyui_runtime_internal_widget_get_type(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_runtime_internal_widget_get_parent(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_runtime_internal_widget_get_first_child(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_runtime_internal_widget_get_next_sibling(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_runtime_internal_widget_get_root(const struct tinyui_widget *widget);
struct tinyui_widget *tinyui_runtime_internal_widget_find_by_name_id(const struct tinyui_widget *root, int name_id);
struct tinyui_point tinyui_runtime_internal_widget_get_absolute_pos(const struct tinyui_widget *widget, struct tinyui_point point);
struct tinyui_point tinyui_runtime_internal_widget_get_relative_pos(const struct tinyui_widget *widget, struct tinyui_point point);

/* helpers that lived in widget.h but are not tinyui_widget_* names — keep original symbols */
struct tinyui_rect tinyui_rect_align(struct tinyui_rect parent, struct tinyui_rect child, enum tinyui_align x_align, enum tinyui_align y_align);
struct tinyui_rect tinyui_rect_center(struct tinyui_rect parent, struct tinyui_rect child);
int tinyui_vertical_grid_align_offset(struct tinyui_rect widget, int current_offset, int item_count, int item_height, int space);
int tinyui_focus_reset(struct tinyui_app *app);
int tinyui_focus_navigate(struct tinyui_app *app, enum tinyui_native_nav_dir dir);

/* pump / typos / inits */
int tinyui_runtime_internal_timer_handler(void);
int tinyui_runtime_internal_button_set_press(tinyui_obj_t *button, int pressed);
int tinyui_runtime_internal_button_get_press(tinyui_obj_t *button, int *pressed);
int tinyui_runtime_internal_tabel_show_keyboard(tinyui_obj_t *table);
struct tinyui_qrcode *tinyui_runtime_internal_q_r_code_init(struct tinyui_widget *parent, const char *id);
int tinyui_runtime_internal_q_r_code_set_text(tinyui_obj_t *qrcode, const char *text);
struct tinyui_arc *tinyui_runtime_internal_arc_init(struct tinyui_widget *parent, const char *id);
struct tinyui_animation *tinyui_runtime_internal_animation_init(struct tinyui_widget *parent, const char *id);
struct tinyui_clock *tinyui_runtime_internal_clock_init(struct tinyui_widget *parent, const char *id);
struct tinyui_date_time *tinyui_runtime_internal_date_time_init(struct tinyui_widget *parent, const char *id);
struct tinyui_gauge *tinyui_runtime_internal_gauge_init(struct tinyui_widget *parent, const char *id);
struct tinyui_icon_slider *tinyui_runtime_internal_icon_slider_init(struct tinyui_widget *parent, const char *id);
struct tinyui_message_box *tinyui_runtime_internal_message_box_init(struct tinyui_widget *parent, const char *id);
struct tinyui_progress_wheel *tinyui_runtime_internal_progress_wheel_init(struct tinyui_widget *parent, const char *id);
struct tinyui_radial_menu *tinyui_runtime_internal_radial_menu_init(struct tinyui_widget *parent, const char *id);

#endif /* TINYUI_RUNTIME_INTERNAL_LEGACY_API_H */
