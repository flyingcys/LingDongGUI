/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "legacy_demo0_parity/legacy_demo0_parity.h"
#include "tinyui.h"

#include <stdlib.h>
#include <string.h>

struct legacy_demo0_runtime {
    tinyui_obj_t *image;
    tinyui_obj_t *switch_label;
    tinyui_obj_t *gauge;
    tinyui_obj_t *arc;
    float angle;
};

struct legacy_demo0_sources {
    tinyui_image_source_t paper;
    tinyui_image_source_t button_release;
    tinyui_image_source_t button_press;
    tinyui_image_source_t progress_bg;
    tinyui_image_source_t progress_fg;
    tinyui_image_source_t slider_bg;
    tinyui_image_source_t slider_indicator;
    tinyui_image_source_t weather;
    tinyui_image_source_t note;
    tinyui_image_source_t book;
    tinyui_image_source_t chart;
    tinyui_image_source_t gauge_bg;
    tinyui_image_source_t gauge_pointer;
    tinyui_image_source_t arc_quarter;
};

static const char g_legacy_qrcode_truth[] = {'l', 'd', 'g', 'u', 'i', '\0'};
static tinyui_font_t g_font_arial_12;
static tinyui_font_t g_font_arial_16;
static int g_fonts_ready;
static const char *const g_scroll_ids[] = {"1", "10", "123", "99", "7"};
static const char *const g_icon_ids[] = {"11", "22", "33", "44", "55"};
static const char *const g_combo_ids[] = {"11", "22", "00"};
static const char *const g_message_buttons[] = {"11", "22", "33"};
static const char *const g_calendar_day_names[7] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fir", "Sat",
};

static struct legacy_demo0_sources g_sources;
static struct legacy_demo0_runtime g_runtime;
static int g_sources_ready;

enum {
    LEGACY_DEMO0_FRAME_INTERVAL_MS = 100,
};

static tinyui_result_t ensure_sources(void)
{
    if (g_sources_ready != 0) {
        return TINYUI_OK;
    }
    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &g_sources.paper) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_KEY_RELEASE, &g_sources.button_release) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_KEY_PRESS, &g_sources.button_press) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_PROGRESS_BG, &g_sources.progress_bg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_PROGRESS_FG, &g_sources.progress_fg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_SLIDER_BG, &g_sources.slider_bg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR, &g_sources.slider_indicator) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_WEATHER, &g_sources.weather) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE, &g_sources.note) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_BOOK, &g_sources.book) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_CHART, &g_sources.chart) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_GAUGE_BG, &g_sources.gauge_bg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_GAUGE_POINTER, &g_sources.gauge_pointer) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &g_sources.arc_quarter) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    g_sources_ready = 1;
    return TINYUI_OK;
}

static tinyui_result_t ensure_fonts(void)
{
    if (g_fonts_ready != 0) {
        return TINYUI_OK;
    }
    if (tinyui_font_from_builtin(TINYUI_FONT_ARIAL_12, &g_font_arial_12) != TINYUI_OK
        || tinyui_font_from_builtin(TINYUI_FONT_ARIAL_16_A8, &g_font_arial_16) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    g_fonts_ready = 1;
    return TINYUI_OK;
}

static void on_button_released(const tinyui_event_t *event)
{
    struct legacy_demo0_runtime *runtime;

    if (event == NULL) {
        return;
    }
    runtime = (struct legacy_demo0_runtime *)event->user_data;
    if (runtime != NULL && runtime->image != NULL) {
        (void)tinyui_obj_set_opacity(runtime->image, 128);
    }
}

static void on_switch_toggled(const tinyui_event_t *event)
{
    tinyui_obj_t *switch_label;

    if (event == NULL) {
        return;
    }
    switch_label = (tinyui_obj_t *)event->user_data;
    if (switch_label != NULL) {
        (void)tinyui_label_set_text(switch_label, event->data.value != 0 ? "ON" : "OFF");
    }
}

static void on_angle_timer(tinyui_timer_t *timer, void *user_data)
{
    struct legacy_demo0_runtime *runtime = (struct legacy_demo0_runtime *)user_data;

    (void)timer;
    if (runtime == NULL || runtime->gauge == NULL || runtime->arc == NULL) {
        return;
    }
    (void)tinyui_arc_set_rotation_angle(runtime->arc, runtime->angle);
    (void)tinyui_gauge_set_angle(runtime->gauge, runtime->angle);
    runtime->angle += 1.0f;
    if (runtime->angle >= 360.0f) {
        runtime->angle = 0.0f;
    }
}

static tinyui_result_t seed_graph(tinyui_obj_t *graph)
{
    int a;
    int b;
    int i;

    if (tinyui_graph_set_axis(graph, 80, 80) != 0
        || tinyui_graph_set_axis_offset(graph, 5) != 0
        || tinyui_graph_set_grid_offset(graph, 4) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    a = tinyui_graph_add_series(graph, 0xFF0000U, 2, 16);
    b = tinyui_graph_add_series(graph, 0xC0C0C0U, 2, 16);
    if (a < 0 || b < 0) {
        return TINYUI_ERROR_BACKEND;
    }
    srand(10);
    for (i = 0; i < 16; ++i) {
        if (tinyui_graph_set_value(graph, a, i, rand() % 81) != 0
            || tinyui_graph_set_value(graph, b, i, rand() % 81) != 0) {
            return TINYUI_ERROR_BACKEND;
        }
    }
    return TINYUI_OK;
}

static tinyui_result_t seed_table(tinyui_obj_t *table, tinyui_obj_t *keyboard)
{
    if (tinyui_table_set_excel_type(table) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    if (keyboard != NULL && tinyui_table_set_keyboard_widget(table, keyboard) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_table_set_cell_text(table, 1, 1, "id") != 0
        || tinyui_table_set_cell_text(table, 1, 2, "name") != 0
        || tinyui_table_set_cell_text(table, 1, 3, "size") != 0
        || tinyui_table_set_cell_text(table, 2, 1, "1") != 0
        || tinyui_table_set_cell_text(table, 2, 2, "button") != 0
        || tinyui_table_set_cell_text(table, 2, 3, "30*20") != 0
        || tinyui_table_set_cell_text(table, 3, 1, "2") != 0
        || tinyui_table_set_cell_text(table, 3, 2, "image") != 0
        || tinyui_table_set_cell_text(table, 3, 3, "100*100") != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_demo_legacy_demo0_parity_build(tinyui_obj_t *screen)
{
    tinyui_graph_props_t graph_props;
    tinyui_table_props_t table_props;
    tinyui_obj_t *image;
    tinyui_obj_t *button;
    tinyui_obj_t *list_item_button;
    tinyui_obj_t *nested_button;
    tinyui_obj_t *panel;
    tinyui_obj_t *label;
    tinyui_obj_t *radio_a;
    tinyui_obj_t *radio_b;
    tinyui_obj_t *check;
    tinyui_obj_t *sw;
    tinyui_obj_t *switch_label;
    tinyui_obj_t *bar;
    tinyui_obj_t *text;
    tinyui_obj_t *slider_h;
    tinyui_obj_t *slider_v;
    tinyui_obj_t *radial_menu;
    tinyui_obj_t *date_time;
    tinyui_obj_t *icon_slider;
    tinyui_obj_t *qrcode;
    tinyui_obj_t *scroll_selector;
    tinyui_obj_t *gauge;
    tinyui_obj_t *combo_box;
    tinyui_obj_t *graph;
    tinyui_obj_t *table;
    tinyui_obj_t *line_edit;
    tinyui_obj_t *child_window;
    tinyui_obj_t *keyboard;
    tinyui_obj_t *arc;
    tinyui_obj_t *list;
    tinyui_obj_t *message_box;
    tinyui_obj_t *calendar;
    tinyui_timer_t *timer;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (ensure_sources() != TINYUI_OK || ensure_fonts() != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_window_set_color(screen, 0xF0F0F0U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    memset(&g_runtime, 0, sizeof(g_runtime));
    g_runtime.angle = 120.0f;

    memset(&graph_props, 0, sizeof(graph_props));
    graph_props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX
        | TINYUI_GRAPH_FIELD_WIDTH
        | TINYUI_GRAPH_FIELD_HEIGHT;
    graph_props.series_max = 2;
    graph_props.width = 100;
    graph_props.height = 100;

    memset(&table_props, 0, sizeof(table_props));
    table_props.fields = TINYUI_TABLE_FIELD_ROWS
        | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_WIDTH
        | TINYUI_TABLE_FIELD_HEIGHT;
    table_props.rows = 10;
    table_props.columns = 6;
    table_props.width = 200;
    table_props.height = 100;

    image = tinyui_image_create(screen);
    button = tinyui_button_create(screen);
    panel = tinyui_window_create(screen);
    label = tinyui_label_create(screen);
    radio_a = tinyui_checkbox_create(screen);
    radio_b = tinyui_checkbox_create(screen);
    check = tinyui_checkbox_create(screen);
    sw = tinyui_switch_create(screen);
    switch_label = tinyui_label_create(screen);
    bar = tinyui_progress_bar_create(screen);
    text = tinyui_text_create(screen);
    slider_h = tinyui_slider_create(screen);
    slider_v = tinyui_slider_create(screen);
    radial_menu = tinyui_radial_menu_create(screen);
    date_time = tinyui_date_time_create(screen);
    icon_slider = tinyui_icon_slider_create(screen);
    qrcode = tinyui_qrcode_create(screen);
    scroll_selector = tinyui_scroll_selector_create(screen);
    gauge = tinyui_gauge_create(screen);
    combo_box = tinyui_combo_box_create(screen);
    graph = tinyui_graph_create_with_props(screen, &graph_props);
    table = tinyui_table_create_with_props(screen, &table_props);
    line_edit = tinyui_line_edit_create(screen);
    child_window = tinyui_window_create(screen);
    keyboard = tinyui_keyboard_create(screen);
    arc = tinyui_arc_create(screen);
    list = tinyui_list_create(screen);
    message_box = tinyui_message_box_create(screen);
    calendar = tinyui_calendar_create(screen);

    if (image == NULL || button == NULL || panel == NULL || label == NULL
        || radio_a == NULL || radio_b == NULL || check == NULL || sw == NULL
        || switch_label == NULL || bar == NULL || text == NULL
        || slider_h == NULL || slider_v == NULL || radial_menu == NULL
        || date_time == NULL || icon_slider == NULL || qrcode == NULL
        || scroll_selector == NULL || gauge == NULL || combo_box == NULL
        || graph == NULL || table == NULL || line_edit == NULL
        || child_window == NULL || keyboard == NULL || arc == NULL
        || list == NULL || message_box == NULL || calendar == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    g_runtime.image = image;
    g_runtime.switch_label = switch_label;
    g_runtime.gauge = gauge;
    g_runtime.arc = arc;

    if (tinyui_image_set_source(image, &g_sources.paper) != 0
        || tinyui_obj_set_pos(image, 100, 120) != TINYUI_OK
        || tinyui_obj_set_size(image, 50, 80) != TINYUI_OK
        || tinyui_obj_set_selectable(image, 1) != TINYUI_OK
        || tinyui_button_set_text(button, "123") != 0
        || tinyui_button_set_font(button, &g_font_arial_16) != 0
        || tinyui_button_set_image(button, &g_sources.button_release, &g_sources.button_press) != 0
        || tinyui_button_set_text_color(button, 0xFFFFFFU) != 0
        || tinyui_button_set_on_released(button, on_button_released, &g_runtime) != 0
        || tinyui_obj_set_pos(button, 10, 10) != TINYUI_OK
        || tinyui_obj_set_size(button, 79, 53) != TINYUI_OK
        || tinyui_obj_set_selectable(button, 1) != TINYUI_OK
        || tinyui_window_set_color(panel, 0x00FF00U) != 0
        || tinyui_obj_set_pos(panel, 200, 95) != TINYUI_OK
        || tinyui_obj_set_size(panel, 20, 20) != TINYUI_OK
        || tinyui_obj_set_selectable(panel, 1) != TINYUI_OK
        || tinyui_label_set_text(label, "123") != 0
        || tinyui_label_set_font(label, &g_font_arial_12) != 0
        || tinyui_label_set_bg_color(label, 0xC0C0C0U) != 0
        || tinyui_label_set_text_align(label, TINYUI_ALIGN_START, TINYUI_ALIGN_END) != 0
        || tinyui_obj_set_pos(label, 100, 50) != TINYUI_OK
        || tinyui_obj_set_size(label, 100, 50) != TINYUI_OK
        || tinyui_obj_set_selectable(label, 1) != TINYUI_OK
        || tinyui_checkbox_set_text(radio_a, "999") != 0
        || tinyui_checkbox_set_radio_group(radio_a, 0) != 0
        || tinyui_obj_set_pos(radio_a, 220, 10) != TINYUI_OK
        || tinyui_obj_set_size(radio_a, 50, 20) != TINYUI_OK
        || tinyui_obj_set_selectable(radio_a, 1) != TINYUI_OK
        || tinyui_checkbox_set_radio_group(radio_b, 0) != 0
        || tinyui_obj_set_pos(radio_b, 220, 40) != TINYUI_OK
        || tinyui_obj_set_size(radio_b, 50, 20) != TINYUI_OK
        || tinyui_obj_set_selectable(radio_b, 1) != TINYUI_OK
        || tinyui_obj_set_pos(check, 220, 70) != TINYUI_OK
        || tinyui_obj_set_size(check, 50, 20) != TINYUI_OK
        || tinyui_obj_set_selectable(check, 1) != TINYUI_OK
        || tinyui_switch_set_checked(sw, 0) != 0
        || tinyui_switch_set_on_toggled(sw, on_switch_toggled, switch_label) != 0
        || tinyui_obj_set_pos(sw, 300, 226) != TINYUI_OK
        || tinyui_obj_set_size(sw, 48, 24) != TINYUI_OK
        || tinyui_obj_set_selectable(sw, 1) != TINYUI_OK
        || tinyui_label_set_text(switch_label, "OFF") != 0
        || tinyui_label_set_font(switch_label, &g_font_arial_16) != 0
        || tinyui_label_set_text_align(switch_label, TINYUI_ALIGN_START, TINYUI_ALIGN_CENTER) != 0
        || tinyui_obj_set_pos(switch_label, 356, 218) != TINYUI_OK
        || tinyui_obj_set_size(switch_label, 60, 40) != TINYUI_OK
        || tinyui_obj_set_selectable(switch_label, 1) != TINYUI_OK
        || tinyui_progress_bar_set_horizontal(bar, 1) != 0
        || tinyui_progress_bar_set_percent(bar, 45) != 0
        || tinyui_progress_bar_set_image(bar, &g_sources.progress_bg, &g_sources.progress_fg) != 0
        || tinyui_obj_set_pos(bar, 10, 500) != TINYUI_OK
        || tinyui_obj_set_size(bar, 300, 30) != TINYUI_OK
        || tinyui_obj_set_selectable(bar, 1) != TINYUI_OK
        || tinyui_text_set_background_source(text, &g_sources.paper) != 0
        || tinyui_text_set_font(text, &g_font_arial_12) != 0
        || tinyui_text_set_text(text, "123\n12333") != 0
        || tinyui_text_set_scroll_enabled(text, 1) != 0
        || tinyui_obj_set_pos(text, 300, 10) != TINYUI_OK
        || tinyui_obj_set_size(text, 150, 200) != TINYUI_OK
        || tinyui_obj_set_selectable(text, 1) != TINYUI_OK
        || tinyui_slider_set_percent(slider_h, 42) != 0
        || tinyui_slider_set_image(slider_h, &g_sources.slider_bg, &g_sources.slider_indicator) != 0
        || tinyui_obj_set_pos(slider_h, 50, 300) != TINYUI_OK
        || tinyui_obj_set_size(slider_h, 317, 34) != TINYUI_OK
        || tinyui_slider_set_horizontal(slider_v, 0) != 0
        || tinyui_slider_set_percent(slider_v, 42) != 0
        || tinyui_obj_set_pos(slider_v, 400, 300) != TINYUI_OK
        || tinyui_obj_set_size(slider_v, 30, 100) != TINYUI_OK
        || tinyui_obj_set_selectable(slider_v, 1) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_radial_menu_set_geometry(radial_menu, 150, 100, 100, 80, 5) != 0
        || tinyui_radial_menu_add_item_with_source(radial_menu, "weather", &g_sources.weather) != 0
        || tinyui_radial_menu_add_item_with_source(radial_menu, "note", &g_sources.note) != 0
        || tinyui_radial_menu_add_item_with_source(radial_menu, "weather2", &g_sources.weather) != 0
        || tinyui_radial_menu_add_item_with_source(radial_menu, "note2", &g_sources.note) != 0
        || tinyui_obj_set_pos(radial_menu, 500, 200) != TINYUI_OK
        || tinyui_obj_set_selectable(radial_menu, 1) != TINYUI_OK
        || tinyui_date_time_set_font(date_time, &g_font_arial_12) != 0
        || tinyui_date_time_set_use_system_time(date_time, 1) != 0
        || tinyui_obj_set_pos(date_time, 600, 100) != TINYUI_OK
        || tinyui_obj_set_size(date_time, 200, 50) != TINYUI_OK
        || tinyui_obj_set_selectable(date_time, 1) != TINYUI_OK
        || tinyui_icon_slider_set_layout(icon_slider, 150, 65, 48, 2, 5, 1, 1) != 0
        || tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[0], g_icon_ids[0], &g_sources.note) != 0
        || tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[1], g_icon_ids[1], &g_sources.book) != 0
        || tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[2], g_icon_ids[2], &g_sources.weather) != 0
        || tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[3], g_icon_ids[3], &g_sources.chart) != 0
        || tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[4], g_icon_ids[4], &g_sources.note) != 0
        || tinyui_obj_set_pos(icon_slider, 500, 350) != TINYUI_OK
        || tinyui_obj_set_selectable(icon_slider, 1) != TINYUI_OK
        || tinyui_qrcode_set_text(qrcode, g_legacy_qrcode_truth) != 0
        || tinyui_qrcode_set_qr_color(qrcode, 0x0000FFU) != 0
        || tinyui_qrcode_set_bg_color(qrcode, 0xFFFFFFU) != 0
        || tinyui_qrcode_set_max_version(qrcode, 2) != 0
        || tinyui_qrcode_set_zoom(qrcode, 5) != 0
        || tinyui_obj_set_opacity(qrcode, 100) != TINYUI_OK
        || tinyui_obj_set_pos(qrcode, 500, 10) != TINYUI_OK
        || tinyui_obj_set_size(qrcode, 200, 200) != TINYUI_OK
        || tinyui_obj_set_selectable(qrcode, 1) != TINYUI_OK
        || tinyui_scroll_selector_set_items(scroll_selector, g_scroll_ids, g_scroll_ids, 5) != 0
        || tinyui_scroll_selector_set_background_color(scroll_selector, 0xFFFFFFU) != 0
        || tinyui_obj_set_pos(scroll_selector, 700, 200) != TINYUI_OK
        || tinyui_obj_set_size(scroll_selector, 30, 50) != TINYUI_OK
        || tinyui_obj_set_selectable(scroll_selector, 1) != TINYUI_OK
        || tinyui_gauge_set_bg_source(gauge, &g_sources.gauge_bg) != 0
        || tinyui_gauge_set_centre_offset(gauge, 0, 10) != 0
        || tinyui_gauge_set_pointer_mask_source(gauge, &g_sources.gauge_pointer, 5, 45) != 0
        || tinyui_gauge_set_pointer_color(gauge, 0x0000FFU) != 0
        || tinyui_gauge_set_angle(gauge, 120.0f) != 0
        || tinyui_obj_set_pos(gauge, 700, 300) != TINYUI_OK
        || tinyui_obj_set_size(gauge, 120, 98) != TINYUI_OK
        || tinyui_obj_set_selectable(gauge, 1) != TINYUI_OK
        || tinyui_combo_box_set_static_items(combo_box, g_combo_ids, g_combo_ids, 3) != 0
        || tinyui_obj_set_pos(combo_box, 700, 420) != TINYUI_OK
        || tinyui_obj_set_size(combo_box, 100, 30) != TINYUI_OK
        || tinyui_obj_set_selectable(combo_box, 1) != TINYUI_OK
        || tinyui_obj_set_pos(graph, 830, 10) != TINYUI_OK
        || tinyui_obj_set_size(graph, 100, 100) != TINYUI_OK
        || seed_graph(graph) != TINYUI_OK
        || tinyui_obj_set_selectable(graph, 1) != TINYUI_OK
        || tinyui_table_set_item_space(table, 1) != 0
        || seed_table(table, keyboard) != TINYUI_OK
        || tinyui_obj_set_pos(table, 780, 150) != TINYUI_OK
        || tinyui_obj_set_size(table, 200, 100) != TINYUI_OK
        || tinyui_obj_set_selectable(table, 1) != TINYUI_OK
        || tinyui_line_edit_set_text(line_edit, "123") != 0
        || tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) != 0
        || tinyui_obj_set_pos(line_edit, 850, 400) != TINYUI_OK
        || tinyui_obj_set_size(line_edit, 100, 50) != TINYUI_OK
        || tinyui_obj_set_selectable(line_edit, 1) != TINYUI_OK
        || tinyui_obj_set_pos(child_window, 850, 450) != TINYUI_OK
        || tinyui_obj_set_size(child_window, 100, 100) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    nested_button = tinyui_button_create(child_window);
    if (nested_button == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_button_set_text(nested_button, "123") != 0
        || tinyui_button_set_font(nested_button, &g_font_arial_16) != 0
        || tinyui_obj_set_pos(nested_button, 8, 3) != TINYUI_OK
        || tinyui_obj_set_size(nested_button, 30, 30) != TINYUI_OK
        || tinyui_obj_set_pos(keyboard, 780, 450) != TINYUI_OK
        || tinyui_arc_set_quarter_source(arc, &g_sources.arc_quarter) != 0
        || tinyui_arc_set_background_angle(arc, 0.0f, 270.0f) != 0
        || tinyui_arc_set_foreground_angle(arc, 0.0f) != 0
        || tinyui_arc_set_parent_color(arc, 0xF0F0F0U) != 0
        || tinyui_arc_set_color(arc, 0xADD8E6U, 0x90EE90U) != 0
        || tinyui_obj_set_pos(arc, 450, 450) != TINYUI_OK
        || tinyui_obj_set_size(arc, 103, 103) != TINYUI_OK
        || tinyui_list_set_item_height(list, 30) != 0
        || tinyui_list_add_item(list, g_scroll_ids[0], g_scroll_ids[0]) != 0
        || tinyui_list_add_item(list, g_scroll_ids[1], g_scroll_ids[1]) != 0
        || tinyui_list_add_item(list, g_scroll_ids[2], g_scroll_ids[2]) != 0
        || tinyui_list_add_item(list, g_scroll_ids[3], g_scroll_ids[3]) != 0
        || tinyui_list_add_item(list, g_scroll_ids[4], g_scroll_ids[4]) != 0
        || tinyui_list_set_align(list, TINYUI_ALIGN_START) != 0
        || tinyui_list_set_selected_index(list, 0) != 0
        || tinyui_obj_set_pos(list, 850, 280) != TINYUI_OK
        || tinyui_obj_set_size(list, 100, 100) != TINYUI_OK
        || tinyui_obj_set_selectable(list, 1) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    list_item_button = tinyui_button_create(screen);
    if (list_item_button == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_obj_set_pos(list_item_button, 10, 3) != TINYUI_OK
        || tinyui_obj_set_size(list_item_button, 20, 20) != TINYUI_OK
        || tinyui_list_set_item_widget(list, 1, list_item_button) != 0
        || tinyui_message_box_set_layout(message_box, 200, 150) != 0
        || tinyui_message_box_set_title(message_box, "title") != 0
        || tinyui_message_box_set_message(message_box, "12345678abcdefg\n99556") != 0
        || tinyui_message_box_set_buttons(message_box, g_message_buttons, 3) != 0
        || tinyui_obj_set_pos(message_box, 200, 150) != TINYUI_OK
        || tinyui_calendar_set_date(calendar, 2026, 1, 1) != 0
        || tinyui_calendar_set_day_names(calendar, g_calendar_day_names) != 0
        || tinyui_calendar_set_header_visible(calendar, 1) != 0
        || tinyui_calendar_set_header_format(calendar, "yyyy - mm - dd") != 0
        || tinyui_obj_set_pos(calendar, 50, 340) != TINYUI_OK
        || tinyui_obj_set_size(calendar, 300, 150) != TINYUI_OK
        || tinyui_obj_set_selectable(calendar, 1) != TINYUI_OK
        || tinyui_obj_set_selected(calendar, 1) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    timer = tinyui_timer_create((uint32_t)LEGACY_DEMO0_FRAME_INTERVAL_MS,
                                true,
                                on_angle_timer,
                                &g_runtime);
    if (timer == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_timer_start(timer) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
