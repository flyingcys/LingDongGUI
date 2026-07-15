/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "legacy_widget_parity/legacy_widget_parity.h"
#include "tinyui.h"

#include <string.h>

struct legacy_widget_sources {
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

static struct legacy_widget_sources s_sources;
static int s_sources_ready;

static tinyui_result_t ensure_legacy_sources(void)
{
    if (s_sources_ready != 0) {
        return TINYUI_OK;
    }
    if (tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &s_sources.paper) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_KEY_RELEASE, &s_sources.button_release) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_KEY_PRESS, &s_sources.button_press) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_PROGRESS_BG, &s_sources.progress_bg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_PROGRESS_FG, &s_sources.progress_fg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_SLIDER_BG, &s_sources.slider_bg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR, &s_sources.slider_indicator) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_WEATHER, &s_sources.weather) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE, &s_sources.note) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_BOOK, &s_sources.book) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_CHART, &s_sources.chart) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_GAUGE_BG, &s_sources.gauge_bg) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_GAUGE_POINTER, &s_sources.gauge_pointer) != TINYUI_OK
        || tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &s_sources.arc_quarter) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    s_sources_ready = 1;
    return TINYUI_OK;
}

static void on_switch_changed(const tinyui_event_t *event)
{
    tinyui_obj_t *status;

    if (event == NULL) {
        return;
    }
    status = (tinyui_obj_t *)event->user_data;
    if (status != NULL) {
        (void)tinyui_label_set_text(status, event->data.value != 0 ? "ON" : "OFF");
    }
}

static tinyui_result_t seed_graph(tinyui_obj_t *graph)
{
    int cpu_series = tinyui_graph_add_series(graph, 0xD62828U, 2, 6);
    int mem_series = tinyui_graph_add_series(graph, 0x457B9DU, 2, 6);

    if (cpu_series < 0 || mem_series < 0) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_graph_set_value(graph, cpu_series, 0, 18) != 0
        || tinyui_graph_set_value(graph, cpu_series, 1, 26) != 0
        || tinyui_graph_set_value(graph, cpu_series, 2, 35) != 0
        || tinyui_graph_set_value(graph, cpu_series, 3, 48) != 0
        || tinyui_graph_set_value(graph, cpu_series, 4, 38) != 0
        || tinyui_graph_set_value(graph, cpu_series, 5, 44) != 0
        || tinyui_graph_set_value(graph, mem_series, 0, 12) != 0
        || tinyui_graph_set_value(graph, mem_series, 1, 19) != 0
        || tinyui_graph_set_value(graph, mem_series, 2, 25) != 0
        || tinyui_graph_set_value(graph, mem_series, 3, 28) != 0
        || tinyui_graph_set_value(graph, mem_series, 4, 32) != 0
        || tinyui_graph_set_value(graph, mem_series, 5, 30) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

static tinyui_result_t seed_table(tinyui_obj_t *table)
{
    if (tinyui_table_set_cell_text(table, 0, 0, "Mon") != 0
        || tinyui_table_set_cell_text(table, 0, 1, "Tue") != 0
        || tinyui_table_set_cell_text(table, 0, 2, "Wed") != 0
        || tinyui_table_set_cell_text(table, 1, 0, "09:00") != 0
        || tinyui_table_set_cell_text(table, 1, 1, "Build") != 0
        || tinyui_table_set_cell_text(table, 1, 2, "Review") != 0
        || tinyui_table_set_cell_text(table, 2, 0, "13:30") != 0
        || tinyui_table_set_cell_text(table, 2, 1, "Demo") != 0
        || tinyui_table_set_cell_text(table, 2, 2, "Ship") != 0
        || tinyui_table_set_current_cell(table, 1, 1) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_demo_legacy_widget_parity_build(tinyui_obj_t *screen)
{
    static const char *combo_ids[] = {"low", "mid", "high"};
    static const char *combo_texts[] = {"Low", "Medium", "High"};
    static const char *scroll_ids[] = {"one", "two", "three", "four", "five"};
    static const char *scroll_texts[] = {"One", "Two", "Three", "Four", "Five"};
    static const char *message_buttons[] = {"Later", "Apply"};
    static const char *calendar_day_names[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    tinyui_line_edit_props_t line_edit_props;
    tinyui_keyboard_props_t keyboard_props;
    tinyui_arc_props_t arc_props;
    tinyui_graph_props_t graph_props;
    tinyui_table_props_t table_props;
    tinyui_obj_t *objs[32];
    tinyui_obj_t *list_item_button;
    tinyui_obj_t *nested_button;
    int i;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (ensure_legacy_sources() != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_window_set_color(screen, 0xF5F6F8U) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    memset(&line_edit_props, 0, sizeof(line_edit_props));
    line_edit_props.fields = TINYUI_LINE_EDIT_FIELD_TEXT
        | TINYUI_LINE_EDIT_FIELD_TYPE
        | TINYUI_LINE_EDIT_FIELD_KEYBOARD_BINDING
        | TINYUI_LINE_EDIT_FIELD_WIDTH
        | TINYUI_LINE_EDIT_FIELD_HEIGHT;
    line_edit_props.text = "123";
    line_edit_props.type = TINYUI_LINE_EDIT_TYPE_STRING;
    line_edit_props.keyboard_binding = 1U;
    line_edit_props.width = 100;
    line_edit_props.height = 32;

    memset(&keyboard_props, 0, sizeof(keyboard_props));
    keyboard_props.fields = TINYUI_KEYBOARD_FIELD_WIDTH | TINYUI_KEYBOARD_FIELD_HEIGHT;
    keyboard_props.width = 180;
    keyboard_props.height = 112;

    memset(&arc_props, 0, sizeof(arc_props));
    arc_props.fields = TINYUI_ARC_FIELD_BG_START_ANGLE
        | TINYUI_ARC_FIELD_BG_END_ANGLE
        | TINYUI_ARC_FIELD_FG_END_ANGLE
        | TINYUI_ARC_FIELD_ROTATION_ANGLE
        | TINYUI_ARC_FIELD_BG_COLOR
        | TINYUI_ARC_FIELD_FG_COLOR;
    arc_props.bg_start_angle = 0.0f;
    arc_props.bg_end_angle = 350.0f;
    arc_props.fg_end_angle = 30.0f;
    arc_props.rotation_angle = 120.0f;
    arc_props.bg_color = 0xADD8E6U;
    arc_props.fg_color = 0x90EE90U;

    memset(&graph_props, 0, sizeof(graph_props));
    graph_props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX
        | TINYUI_GRAPH_FIELD_WIDTH
        | TINYUI_GRAPH_FIELD_HEIGHT;
    graph_props.series_max = 2;
    graph_props.width = 130;
    graph_props.height = 100;

    memset(&table_props, 0, sizeof(table_props));
    table_props.fields = TINYUI_TABLE_FIELD_ROWS
        | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_WIDTH
        | TINYUI_TABLE_FIELD_HEIGHT;
    table_props.rows = 3;
    table_props.columns = 3;
    table_props.width = 150;
    table_props.height = 90;

    objs[0] = tinyui_image_create(screen);
    objs[1] = tinyui_button_create(screen);
    objs[2] = tinyui_label_create(screen);
    objs[3] = tinyui_label_create(screen);
    objs[4] = tinyui_checkbox_create(screen);
    objs[5] = tinyui_checkbox_create(screen);
    objs[6] = tinyui_checkbox_create(screen);
    objs[7] = tinyui_switch_create(screen);
    objs[8] = tinyui_label_create(screen);
    objs[9] = tinyui_progress_bar_create(screen);
    objs[10] = tinyui_text_create(screen);
    objs[11] = tinyui_slider_create(screen);
    objs[12] = tinyui_slider_create(screen);
    objs[13] = tinyui_list_create(screen);
    objs[14] = tinyui_combo_box_create(screen);
    objs[15] = tinyui_calendar_create(screen);
    objs[16] = tinyui_scroll_selector_create(screen);
    objs[17] = tinyui_date_time_create(screen);
    objs[18] = tinyui_message_box_create(screen);
    objs[19] = tinyui_graph_create_with_props(screen, &graph_props);
    objs[20] = tinyui_table_create_with_props(screen, &table_props);
    objs[21] = tinyui_radial_menu_create(screen);
    objs[22] = tinyui_icon_slider_create(screen);
    objs[23] = tinyui_qrcode_create(screen);
    objs[24] = tinyui_gauge_create(screen);
    objs[25] = tinyui_line_edit_create_with_props(screen, &line_edit_props);
    objs[26] = tinyui_keyboard_create_with_props(screen, &keyboard_props);
    objs[27] = tinyui_arc_create_with_props(screen, &arc_props);
    objs[28] = tinyui_window_create(screen);

    for (i = 0; i <= 28; ++i) {
        if (objs[i] == NULL) {
            return TINYUI_ERROR_NO_MEMORY;
        }
    }

    if (tinyui_image_set_source(objs[0], &s_sources.paper) != 0
        || tinyui_obj_set_pos(objs[0], 100, 120) != TINYUI_OK
        || tinyui_obj_set_size(objs[0], 80, 50) != TINYUI_OK
        || tinyui_button_set_text(objs[1], "123") != 0
        || tinyui_button_set_image(objs[1], &s_sources.button_release, &s_sources.button_press) != 0
        || tinyui_obj_set_pos(objs[1], 10, 10) != TINYUI_OK
        || tinyui_obj_set_size(objs[1], 88, 56) != TINYUI_OK
        || tinyui_button_set_text_color(objs[1], 0xFFFFFFU) != 0
        || tinyui_label_set_text(objs[2], "") != 0
        || tinyui_label_set_bg_color(objs[2], 0x2A9D8FU) != 0
        || tinyui_obj_set_pos(objs[2], 200, 95) != TINYUI_OK
        || tinyui_obj_set_size(objs[2], 24, 24) != TINYUI_OK
        || tinyui_label_set_text(objs[3], "123") != 0
        || tinyui_label_set_bg_color(objs[3], 0xD9D9D9U) != 0
        || tinyui_label_set_align(objs[3], TINYUI_ALIGN_END) != 0
        || tinyui_obj_set_pos(objs[3], 100, 50) != TINYUI_OK
        || tinyui_obj_set_size(objs[3], 100, 50) != TINYUI_OK
        || tinyui_checkbox_set_text(objs[4], "999") != 0
        || tinyui_checkbox_set_radio_group(objs[4], 1) != 0
        || tinyui_checkbox_set_checked(objs[4], 1) != 0
        || tinyui_obj_set_pos(objs[4], 220, 10) != TINYUI_OK
        || tinyui_obj_set_size(objs[4], 72, 20) != TINYUI_OK
        || tinyui_checkbox_set_text(objs[5], "radio") != 0
        || tinyui_checkbox_set_radio_group(objs[5], 1) != 0
        || tinyui_obj_set_pos(objs[5], 220, 40) != TINYUI_OK
        || tinyui_obj_set_size(objs[5], 72, 20) != TINYUI_OK
        || tinyui_checkbox_set_text(objs[6], "check") != 0
        || tinyui_checkbox_set_checked(objs[6], 1) != 0
        || tinyui_obj_set_pos(objs[6], 220, 70) != TINYUI_OK
        || tinyui_obj_set_size(objs[6], 72, 20) != TINYUI_OK
        || tinyui_switch_set_checked(objs[7], 0) != 0
        || tinyui_obj_set_pos(objs[7], 310, 116) != TINYUI_OK
        || tinyui_obj_set_size(objs[7], 56, 28) != TINYUI_OK
        || tinyui_label_set_text(objs[8], "OFF") != 0
        || tinyui_obj_set_pos(objs[8], 374, 114) != TINYUI_OK
        || tinyui_obj_set_size(objs[8], 48, 24) != TINYUI_OK
        || tinyui_switch_set_on_toggled(objs[7], on_switch_changed, objs[8]) != 0
        || tinyui_progress_bar_set_percent(objs[9], 45) != 0
        || tinyui_progress_bar_set_image(objs[9], &s_sources.progress_bg, &s_sources.progress_fg) != 0
        || tinyui_progress_bar_set_color(objs[9], 0xCED4DAU, 0x457B9DU) != 0
        || tinyui_progress_bar_set_frame_color(objs[9], 0x6C757DU, 1) != 0
        || tinyui_obj_set_pos(objs[9], 10, 280) != TINYUI_OK
        || tinyui_obj_set_size(objs[9], 300, 30) != TINYUI_OK
        || tinyui_text_set_text(objs[10], "123\n12333") != 0
        || tinyui_text_set_background_source(objs[10], &s_sources.paper) != 0
        || tinyui_text_set_bg_color(objs[10], 0xF2E8CFU) != 0
        || tinyui_obj_set_pos(objs[10], 300, 10) != TINYUI_OK
        || tinyui_obj_set_size(objs[10], 150, 100) != TINYUI_OK
        || tinyui_slider_set_percent(objs[11], 42) != 0
        || tinyui_slider_set_image(objs[11], &s_sources.slider_bg, &s_sources.slider_indicator) != 0
        || tinyui_slider_set_color(objs[11], 0xCED4DAU, 0xADB5BDU, 0xD62828U) != 0
        || tinyui_obj_set_pos(objs[11], 10, 220) != TINYUI_OK
        || tinyui_obj_set_size(objs[11], 280, 24) != TINYUI_OK
        || tinyui_slider_set_horizontal(objs[12], 0) != 0
        || tinyui_slider_set_percent(objs[12], 42) != 0
        || tinyui_slider_set_image(objs[12], &s_sources.slider_bg, &s_sources.slider_indicator) != 0
        || tinyui_slider_set_color(objs[12], 0xCED4DAU, 0xADB5BDU, 0x2A9D8FU) != 0
        || tinyui_obj_set_pos(objs[12], 300, 220) != TINYUI_OK
        || tinyui_obj_set_size(objs[12], 30, 50) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    if (tinyui_list_add_item(objs[13], "weather", "Weather") != 0
        || tinyui_list_add_item(objs[13], "note", "Note") != 0
        || tinyui_list_add_item(objs[13], "chart", "Chart") != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    list_item_button = tinyui_button_create(screen);
    if (list_item_button == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_button_set_text(list_item_button, "1") != 0
        || tinyui_obj_set_size(list_item_button, 20, 20) != TINYUI_OK
        || tinyui_list_set_item_widget(objs[13], 1, list_item_button) != 0
        || tinyui_list_set_selected_index(objs[13], 1) != 0
        || tinyui_obj_set_pos(objs[13], 360, 10) != TINYUI_OK
        || tinyui_obj_set_size(objs[13], 110, 90) != TINYUI_OK
        || tinyui_combo_box_set_static_items(objs[14], combo_ids, combo_texts, 3) != 0
        || tinyui_combo_box_set_selected_index(objs[14], 1) != 0
        || tinyui_obj_set_pos(objs[14], 360, 110) != TINYUI_OK
        || tinyui_obj_set_size(objs[14], 100, 30) != TINYUI_OK
        || tinyui_calendar_set_date(objs[15], 2026, 6, 4) != 0
        || tinyui_calendar_set_day_names(objs[15], calendar_day_names) != 0
        || tinyui_calendar_set_header_visible(objs[15], 1) != 0
        || tinyui_calendar_set_header_format(objs[15], "yyyy/mm/dd") != 0
        || tinyui_obj_set_pos(objs[15], 10, 150) != TINYUI_OK
        || tinyui_obj_set_size(objs[15], 180, 60) != TINYUI_OK
        || tinyui_scroll_selector_set_items(objs[16], scroll_ids, scroll_texts, 5) != 0
        || tinyui_scroll_selector_set_selected_index(objs[16], 2) != 0
        || tinyui_scroll_selector_set_background_color(objs[16], 0xFFFFFFU) != 0
        || tinyui_obj_set_pos(objs[16], 200, 150) != TINYUI_OK
        || tinyui_obj_set_size(objs[16], 60, 60) != TINYUI_OK
        || tinyui_date_time_set_format(objs[17], "yyyy-mm-dd hh:nn") != 0
        || tinyui_date_time_set_date(objs[17], 2026, 6, 4) != 0
        || tinyui_date_time_set_time(objs[17], 13, 45, 0) != 0
        || tinyui_obj_set_pos(objs[17], 270, 150) != TINYUI_OK
        || tinyui_obj_set_size(objs[17], 180, 32) != TINYUI_OK
        || tinyui_message_box_set_title(objs[18], "Update") != 0
        || tinyui_message_box_set_message(objs[18], "Apply settings?") != 0
        || tinyui_message_box_set_confirm_text(objs[18], "OK") != 0
        || tinyui_message_box_set_buttons(objs[18], message_buttons, 2) != 0
        || tinyui_obj_set_pos(objs[18], 270, 190) != TINYUI_OK
        || tinyui_graph_set_axis(objs[19], 80, 80) != 0
        || tinyui_graph_set_grid_offset(objs[19], 4) != 0
        || seed_graph(objs[19]) != TINYUI_OK
        || tinyui_obj_set_pos(objs[19], 340, 190) != TINYUI_OK
        || tinyui_obj_set_size(objs[19], 130, 100) != TINYUI_OK
        || seed_table(objs[20]) != TINYUI_OK
        || tinyui_obj_set_pos(objs[20], 200, 10) != TINYUI_OK
        || tinyui_obj_set_size(objs[20], 150, 90) != TINYUI_OK
        || tinyui_radial_menu_add_item_with_source(objs[21], "weather", &s_sources.weather) != 0
        || tinyui_radial_menu_add_item_with_source(objs[21], "note", &s_sources.note) != 0
        || tinyui_radial_menu_add_item_with_source(objs[21], "weather2", &s_sources.weather) != 0
        || tinyui_radial_menu_add_item_with_source(objs[21], "note2", &s_sources.note) != 0
        || tinyui_radial_menu_set_selected_index(objs[21], 1) != 0
        || tinyui_obj_set_pos(objs[21], 10, 70) != TINYUI_OK
        || tinyui_obj_set_size(objs[21], 80, 70) != TINYUI_OK
        || tinyui_icon_slider_add_item_with_source(objs[22], "11", "11", &s_sources.note) != 0
        || tinyui_icon_slider_add_item_with_source(objs[22], "22", "22", &s_sources.book) != 0
        || tinyui_icon_slider_add_item_with_source(objs[22], "33", "33", &s_sources.weather) != 0
        || tinyui_icon_slider_add_item_with_source(objs[22], "44", "44", &s_sources.chart) != 0
        || tinyui_icon_slider_add_item_with_source(objs[22], "55", "55", &s_sources.note) != 0
        || tinyui_icon_slider_set_selected_index(objs[22], 1) != 0
        || tinyui_obj_set_pos(objs[22], 120, 250) != TINYUI_OK
        || tinyui_obj_set_size(objs[22], 170, 30) != TINYUI_OK
        || tinyui_qrcode_set_text(objs[23], "gui-demo") != 0
        || tinyui_qrcode_set_qr_color(objs[23], 0x0000FFU) != 0
        || tinyui_qrcode_set_bg_color(objs[23], 0xFFFFFFU) != 0
        || tinyui_qrcode_set_max_version(objs[23], 2) != 0
        || tinyui_qrcode_set_zoom(objs[23], 3) != 0
        || tinyui_obj_set_pos(objs[23], 360, 150) != TINYUI_OK
        || tinyui_obj_set_size(objs[23], 80, 80) != TINYUI_OK
        || tinyui_gauge_set_angle(objs[24], 120.0f) != 0
        || tinyui_gauge_set_bg_source(objs[24], &s_sources.gauge_bg) != 0
        || tinyui_gauge_set_pointer_source(objs[24], &s_sources.gauge_pointer) != 0
        || tinyui_gauge_set_pointer_color(objs[24], 0x0000FFU) != 0
        || tinyui_obj_set_pos(objs[24], 360, 70) != TINYUI_OK
        || tinyui_obj_set_size(objs[24], 100, 70) != TINYUI_OK
        || tinyui_obj_set_pos(objs[25], 10, 290) != TINYUI_OK
        || tinyui_obj_set_pos(objs[26], 120, 290) != TINYUI_OK
        || tinyui_arc_set_quarter_source(objs[27], &s_sources.arc_quarter) != 0
        || tinyui_obj_set_pos(objs[27], 280, 250) != TINYUI_OK
        || tinyui_obj_set_size(objs[27], 70, 70) != TINYUI_OK
        || tinyui_window_set_color(objs[28], 0xFFFFFFU) != 0
        || tinyui_obj_set_pos(objs[28], 360, 250) != TINYUI_OK
        || tinyui_obj_set_size(objs[28], 100, 60) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    nested_button = tinyui_button_create(objs[28]);
    if (nested_button == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }
    if (tinyui_button_set_text(nested_button, "123") != 0
        || tinyui_obj_set_pos(nested_button, 8, 3) != TINYUI_OK
        || tinyui_obj_set_size(nested_button, 30, 30) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
