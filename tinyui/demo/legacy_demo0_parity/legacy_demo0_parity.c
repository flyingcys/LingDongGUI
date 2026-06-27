/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "legacy_demo0_parity/legacy_demo0_parity.h"
#include "tinyui.h"

struct legacy_demo0_runtime {
    struct tinyui_image *image;
    struct tinyui_label *switch_label;
    struct tinyui_gauge *gauge;
    struct tinyui_arc *arc;
    float angle;
    unsigned int frame_accumulated_ms;
};

struct legacy_demo0_sources {
    struct tinyui_image_source paper;
    struct tinyui_image_source button_release;
    struct tinyui_image_source button_press;
    struct tinyui_image_source progress_bg;
    struct tinyui_image_source progress_fg;
    struct tinyui_image_source slider_bg;
    struct tinyui_image_source slider_indicator;
    struct tinyui_image_source weather;
    struct tinyui_image_source note;
    struct tinyui_image_source book;
    struct tinyui_image_source chart;
    struct tinyui_image_source gauge_bg;
    struct tinyui_image_source gauge_pointer;
    struct tinyui_image_source arc_quarter;
};

static const char *const g_legacy_demo0_truth_fields[] = {
    "button@10, 10:123",
    "\"123\"",
    "text@300, 10:123\\n12333",
    "\"123\\n12333\"",
    "switch@300, 226:OFF",
    "\"OFF\"",
    "switch_label@356, 218:OFF",
    "qrcode@500, 10:legacy token",
    "g_legacy_qrcode_truth",
    "{'l', 'd', 'g', 'u', 'i', '\\0'}",
    "list@850, 280:1/10/123/99/7",
    "\"title\"",
    "message_box@200, 150:title/12345678abcdefg\\n99556",
    "\"12345678abcdefg\\n99556\"",
    "calendar@50, 340:yyyy - mm - dd",
    "\"yyyy - mm - dd\"",
};

static const char g_legacy_qrcode_truth[] = {'l', 'd', 'g', 'u', 'i', '\0'};
static const char *const g_scroll_ids[] = {"1", "10", "123", "99", "7"};
static const char *const g_icon_ids[] = {"11", "22", "33", "44", "55"};
static const char *const g_combo_ids[] = {"11", "22", "00"};
static const char *const g_message_buttons[] = {"11", "22", "33"};
static const char *const g_calendar_day_names[7] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fir",
    "Sat",
};

static struct legacy_demo0_sources g_sources;
static struct legacy_demo0_runtime g_runtime;
static int g_sources_ready;

enum {
    LEGACY_DEMO0_FRAME_INTERVAL_MS = 100,
};

static int load_source(enum tinyui_builtin_image image, struct tinyui_image_source *source)
{
    return tinyui_image_source_from_builtin(image, source);
}

static int ensure_sources(void)
{
    if (g_sources_ready != 0) {
        return 0;
    }

    if (load_source(TINYUI_BUILTIN_IMAGE_LETTER_PAPER, &g_sources.paper) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_KEY_RELEASE, &g_sources.button_release) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_KEY_PRESS, &g_sources.button_press) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_PROGRESS_BG, &g_sources.progress_bg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_PROGRESS_FG, &g_sources.progress_fg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_SLIDER_BG, &g_sources.slider_bg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_SLIDER_INDICATOR, &g_sources.slider_indicator) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_WEATHER, &g_sources.weather) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_NOTE, &g_sources.note) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_BOOK, &g_sources.book) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_CHART, &g_sources.chart) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_GAUGE_BG, &g_sources.gauge_bg) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_GAUGE_POINTER, &g_sources.gauge_pointer) != 0
        || load_source(TINYUI_BUILTIN_IMAGE_ARC_QUARTER, &g_sources.arc_quarter) != 0) {
        return -1;
    }

    g_sources_ready = 1;
    return 0;
}

static void on_button_released(struct tinyui_widget *widget, void *user_data)
{
    struct legacy_demo0_runtime *runtime = (struct legacy_demo0_runtime *)user_data;

    (void)widget;
    if (runtime == 0 || runtime->image == 0) {
        return;
    }

    tinyui_widget_set_opacity((struct tinyui_widget *)runtime->image, 128);
}

static void on_switch_toggled(struct tinyui_widget *widget, int value, void *user_data)
{
    struct tinyui_label *switch_label = (struct tinyui_label *)user_data;

    (void)widget;
    if (switch_label != 0) {
        tinyui_label_set_text(switch_label, value != 0 ? "ON" : "OFF");
    }
}

static void seed_graph(struct tinyui_graph *graph)
{
    static const int series_a[16] = {32, 8, 47, 30, 18, 25, 1, 42, 55, 38, 27, 13, 69, 4, 49, 16};
    static const int series_b[16] = {68, 6, 11, 50, 21, 43, 35, 3, 58, 40, 9, 24, 73, 17, 29, 52};
    int a;
    int b;
    int i;

    tinyui_graph_set_axis(graph, 80, 80);
    tinyui_graph_set_grid_offset(graph, 4);
    a = tinyui_graph_add_series(graph, 0xD62828U, 2, 16);
    b = tinyui_graph_add_series(graph, 0xD9D9D9U, 2, 16);
    for (i = 0; i < 16; i++) {
        if (a >= 0) {
            tinyui_graph_set_value(graph, a, i, series_a[i]);
        }
        if (b >= 0) {
            tinyui_graph_set_value(graph, b, i, series_b[i]);
        }
    }
}

static void seed_table(struct tinyui_table *table)
{
    tinyui_table_set_excel_type(table);
    tinyui_table_set_keyboard_binding(table, 22);
    tinyui_table_set_cell_text(table, 1, 1, "id");
    tinyui_table_set_cell_text(table, 1, 2, "name");
    tinyui_table_set_cell_text(table, 1, 3, "size");
    tinyui_table_set_cell_text(table, 2, 1, "1");
    tinyui_table_set_cell_text(table, 2, 2, "button");
    tinyui_table_set_cell_text(table, 2, 3, "30*20");
    tinyui_table_set_cell_text(table, 3, 1, "2");
    tinyui_table_set_cell_text(table, 3, 2, "image");
    tinyui_table_set_cell_text(table, 3, 3, "100*100");
}

static void build_legacy_demo0(struct tinyui_window *win, struct legacy_demo0_runtime *runtime)
{
    struct tinyui_image *image;
    struct tinyui_button *button;
    struct tinyui_button *list_item_button;
    struct tinyui_button *nested_button;
    struct tinyui_label *panel;
    struct tinyui_label *label;
    struct tinyui_checkbox *radio_a;
    struct tinyui_checkbox *radio_b;
    struct tinyui_checkbox *check;
    struct tinyui_switch *sw;
    struct tinyui_label *switch_label;
    struct tinyui_progress_bar *bar;
    struct tinyui_text *text;
    struct tinyui_slider *slider_h;
    struct tinyui_slider *slider_v;
    struct tinyui_radial_menu *radial_menu;
    struct tinyui_date_time *date_time;
    struct tinyui_icon_slider *icon_slider;
    struct tinyui_qrcode *qrcode;
    struct tinyui_scroll_selecter *scroll_selecter;
    struct tinyui_gauge *gauge;
    struct tinyui_combo_box *combo_box;
    struct tinyui_graph *graph;
    struct tinyui_table *table;
    struct tinyui_line_edit *line_edit;
    struct tinyui_window *child_window;
    struct tinyui_keyboard *keyboard;
    struct tinyui_arc *arc;
    struct tinyui_list *list;
    struct tinyui_message_box *message_box;
    struct tinyui_calendar *calendar;

    if (ensure_sources() != 0) {
        return;
    }

    tinyui_window_set_color(win, 0x000000U);

    image = tinyui_image_create(win, "demo0_image");
    button = tinyui_button_create(win, "demo0_button");
    panel = tinyui_label_create(win, "demo0_panel");
    label = tinyui_label_create(win, "demo0_label");
    radio_a = tinyui_checkbox_create(win, "demo0_radio_a");
    radio_b = tinyui_checkbox_create(win, "demo0_radio_b");
    check = tinyui_checkbox_create(win, "demo0_check");
    sw = tinyui_switch_create(win, "demo0_switch");
    switch_label = tinyui_label_create(win, "demo0_switch_label");
    bar = tinyui_progress_bar_create(win, "demo0_progress");
    text = tinyui_text_create(win, "demo0_text");
    slider_h = tinyui_slider_create(win, "demo0_slider_h");
    slider_v = tinyui_slider_create(win, "demo0_slider_v");
    radial_menu = tinyui_radial_menu_create((struct tinyui_widget *)win, "demo0_radial");
    date_time = tinyui_date_time_create((struct tinyui_widget *)win, "demo0_date_time");
    icon_slider = tinyui_icon_slider_create((struct tinyui_widget *)win, "demo0_icon_slider");
    qrcode = tinyui_qrcode_create((struct tinyui_widget *)win, "demo0_qrcode");
    scroll_selecter = tinyui_scroll_selecter_create(win, "demo0_scroll_selecter");
    gauge = tinyui_gauge_create((struct tinyui_widget *)win, "demo0_gauge");
    combo_box = tinyui_combo_box_create(win, "demo0_combo");
    graph = tinyui_graph_create(win, "demo0_graph", 2);
    table = tinyui_table_create(win, "demo0_table", 10, 6);
    line_edit = tinyui_line_edit_create(win, "demo0_line_edit");
    child_window = tinyui_window_create_child(win, "demo0_child_window");
    keyboard = tinyui_keyboard_create(win, "demo0_keyboard");
    arc = tinyui_arc_create((struct tinyui_widget *)win, "demo0_arc");
    list = tinyui_list_create((struct tinyui_widget *)win, "demo0_list");
    message_box = tinyui_message_box_create((struct tinyui_widget *)win, "demo0_message_box");
    calendar = tinyui_calendar_create(win, "demo0_calendar");

    if (runtime != 0) {
        runtime->image = image;
        runtime->switch_label = switch_label;
        runtime->gauge = gauge;
        runtime->arc = arc;
        runtime->angle = 120.0f;
    }

    if (image != 0) {
        tinyui_image_set_source(image, &g_sources.paper);
        tinyui_widget_set_pos((struct tinyui_widget *)image, 100, 120);
        tinyui_widget_set_size((struct tinyui_widget *)image, 50, 80);
        tinyui_widget_set_corner((struct tinyui_widget *)image, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)image, 1);
    }

    if (button != 0) {
        tinyui_button_set_text(button, "123");
        tinyui_button_set_image(button, &g_sources.button_release, &g_sources.button_press);
        tinyui_button_set_text_color(button, 0xFFFFFFU);
        tinyui_button_set_on_released(button, on_button_released, runtime);
        tinyui_widget_set_pos((struct tinyui_widget *)button, 10, 10);
        tinyui_widget_set_size((struct tinyui_widget *)button, 79, 53);
        tinyui_widget_set_selectable((struct tinyui_widget *)button, 1);
    }

    if (panel != 0) {
        tinyui_label_set_text(panel, "");
        tinyui_label_set_bg_color(panel, 0x00FF00U);
        tinyui_widget_set_pos((struct tinyui_widget *)panel, 200, 95);
        tinyui_widget_set_size((struct tinyui_widget *)panel, 20, 20);
        tinyui_widget_set_corner((struct tinyui_widget *)panel, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)panel, 1);
    }

    if (label != 0) {
        tinyui_label_set_text(label, "123");
        tinyui_label_set_bg_color(label, 0xD3D3D3U);
        tinyui_label_set_align(label, TINYUI_ALIGN_START);
        tinyui_widget_set_pos((struct tinyui_widget *)label, 100, 50);
        tinyui_widget_set_size((struct tinyui_widget *)label, 100, 50);
        tinyui_widget_set_corner((struct tinyui_widget *)label, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)label, 1);
    }

    if (radio_a != 0) {
        tinyui_checkbox_set_text(radio_a, "999");
        tinyui_checkbox_set_radio_group(radio_a, 0);
        tinyui_widget_set_pos((struct tinyui_widget *)radio_a, 220, 10);
        tinyui_widget_set_size((struct tinyui_widget *)radio_a, 50, 20);
        tinyui_widget_set_selectable((struct tinyui_widget *)radio_a, 1);
    }

    if (radio_b != 0) {
        tinyui_checkbox_set_radio_group(radio_b, 0);
        tinyui_widget_set_pos((struct tinyui_widget *)radio_b, 220, 40);
        tinyui_widget_set_size((struct tinyui_widget *)radio_b, 50, 20);
        tinyui_widget_set_selectable((struct tinyui_widget *)radio_b, 1);
    }

    if (check != 0) {
        tinyui_widget_set_pos((struct tinyui_widget *)check, 220, 70);
        tinyui_widget_set_size((struct tinyui_widget *)check, 50, 20);
        tinyui_widget_set_corner((struct tinyui_widget *)check, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)check, 1);
    }

    if (sw != 0) {
        tinyui_switch_set_checked(sw, 0);
        tinyui_switch_set_on_toggled(sw, on_switch_toggled, switch_label);
        tinyui_widget_set_pos((struct tinyui_widget *)sw, 300, 226);
        tinyui_widget_set_size((struct tinyui_widget *)sw, 48, 24);
        tinyui_widget_set_selectable((struct tinyui_widget *)sw, 1);
    }

    if (switch_label != 0) {
        tinyui_label_set_text(switch_label, "OFF");
        tinyui_label_set_align(switch_label, TINYUI_ALIGN_START);
        tinyui_widget_set_pos((struct tinyui_widget *)switch_label, 356, 218);
        tinyui_widget_set_size((struct tinyui_widget *)switch_label, 60, 40);
        tinyui_widget_set_selectable((struct tinyui_widget *)switch_label, 1);
    }

    if (bar != 0) {
        tinyui_progress_bar_set_percent(bar, 45);
        tinyui_progress_bar_set_image(bar, &g_sources.progress_bg, &g_sources.progress_fg);
        tinyui_widget_set_pos((struct tinyui_widget *)bar, 10, 500);
        tinyui_widget_set_size((struct tinyui_widget *)bar, 300, 30);
        tinyui_widget_set_selectable((struct tinyui_widget *)bar, 1);
    }

    if (text != 0) {
        tinyui_text_set_background_source(text, &g_sources.paper);
        tinyui_text_set_text(text, "123\n12333");
        tinyui_widget_set_pos((struct tinyui_widget *)text, 300, 10);
        tinyui_widget_set_size((struct tinyui_widget *)text, 150, 200);
        tinyui_widget_set_corner((struct tinyui_widget *)text, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)text, 1);
    }

    if (slider_h != 0) {
        tinyui_slider_set_percent(slider_h, 42);
        tinyui_slider_set_image(slider_h, &g_sources.slider_bg, &g_sources.slider_indicator);
        tinyui_widget_set_pos((struct tinyui_widget *)slider_h, 50, 300);
        tinyui_widget_set_size((struct tinyui_widget *)slider_h, 317, 24);
    }

    if (slider_v != 0) {
        tinyui_slider_set_horizontal(slider_v, 0);
        tinyui_slider_set_percent(slider_v, 42);
        tinyui_widget_set_pos((struct tinyui_widget *)slider_v, 400, 300);
        tinyui_widget_set_size((struct tinyui_widget *)slider_v, 30, 100);
        tinyui_widget_set_corner((struct tinyui_widget *)slider_v, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)slider_v, 1);
    }

    if (radial_menu != 0) {
        tinyui_radial_menu_add_item_with_source(radial_menu, "weather", &g_sources.weather);
        tinyui_radial_menu_add_item_with_source(radial_menu, "note", &g_sources.note);
        tinyui_radial_menu_add_item_with_source(radial_menu, "weather2", &g_sources.weather);
        tinyui_radial_menu_add_item_with_source(radial_menu, "note2", &g_sources.note);
        tinyui_widget_set_pos((struct tinyui_widget *)radial_menu, 500, 200);
        tinyui_widget_set_size((struct tinyui_widget *)radial_menu, 150, 100);
        tinyui_widget_set_corner((struct tinyui_widget *)radial_menu, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)radial_menu, 1);
    }

    if (date_time != 0) {
        tinyui_widget_set_pos((struct tinyui_widget *)date_time, 600, 100);
        tinyui_widget_set_size((struct tinyui_widget *)date_time, 200, 50);
        tinyui_widget_set_selectable((struct tinyui_widget *)date_time, 1);
    }

    if (icon_slider != 0) {
        tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[0], g_icon_ids[0], &g_sources.note);
        tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[1], g_icon_ids[1], &g_sources.book);
        tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[2], g_icon_ids[2], &g_sources.weather);
        tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[3], g_icon_ids[3], &g_sources.chart);
        tinyui_icon_slider_add_item_with_source(icon_slider, g_icon_ids[4], g_icon_ids[4], &g_sources.note);
        tinyui_widget_set_pos((struct tinyui_widget *)icon_slider, 500, 350);
        tinyui_widget_set_size((struct tinyui_widget *)icon_slider, 150, 65);
        tinyui_widget_set_corner((struct tinyui_widget *)icon_slider, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)icon_slider, 1);
    }

    if (qrcode != 0) {
        tinyui_qrcode_set_text(qrcode, g_legacy_qrcode_truth);
        tinyui_qrcode_set_qr_color(qrcode, 0x0000FFU);
        tinyui_qrcode_set_bg_color(qrcode, 0xFFFFFFU);
        tinyui_qrcode_set_max_version(qrcode, 2);
        tinyui_qrcode_set_zoom(qrcode, 5);
        tinyui_widget_set_opacity((struct tinyui_widget *)qrcode, 100);
        tinyui_widget_set_pos((struct tinyui_widget *)qrcode, 500, 10);
        tinyui_widget_set_size((struct tinyui_widget *)qrcode, 200, 200);
        tinyui_widget_set_selectable((struct tinyui_widget *)qrcode, 1);
    }

    if (scroll_selecter != 0) {
        tinyui_scroll_selecter_set_items(scroll_selecter, g_scroll_ids, g_scroll_ids, 5);
        tinyui_scroll_selecter_set_background_color(scroll_selecter, 0xFFFFFFU);
        tinyui_widget_set_pos((struct tinyui_widget *)scroll_selecter, 700, 200);
        tinyui_widget_set_size((struct tinyui_widget *)scroll_selecter, 30, 50);
        tinyui_widget_set_corner((struct tinyui_widget *)scroll_selecter, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)scroll_selecter, 1);
    }

    if (gauge != 0) {
        tinyui_gauge_set_bg_source(gauge, &g_sources.gauge_bg);
        tinyui_gauge_set_pointer_source(gauge, &g_sources.gauge_pointer);
        tinyui_gauge_set_pointer_color(gauge, 0x0000FFU);
        tinyui_gauge_set_angle(gauge, 120.0f);
        tinyui_widget_set_pos((struct tinyui_widget *)gauge, 700, 300);
        tinyui_widget_set_size((struct tinyui_widget *)gauge, 120, 98);
        tinyui_widget_set_selectable((struct tinyui_widget *)gauge, 1);
    }

    if (combo_box != 0) {
        tinyui_combo_box_set_static_items(combo_box, g_combo_ids, g_combo_ids, 3);
        tinyui_widget_set_pos((struct tinyui_widget *)combo_box, 700, 420);
        tinyui_widget_set_size((struct tinyui_widget *)combo_box, 100, 30);
        tinyui_widget_set_selectable((struct tinyui_widget *)combo_box, 1);
    }

    if (graph != 0) {
        seed_graph(graph);
        tinyui_widget_set_pos((struct tinyui_widget *)graph, 830, 10);
        tinyui_widget_set_size((struct tinyui_widget *)graph, 100, 100);
        tinyui_widget_set_selectable((struct tinyui_widget *)graph, 1);
    }

    if (table != 0) {
        seed_table(table);
        tinyui_widget_set_pos((struct tinyui_widget *)table, 780, 150);
        tinyui_widget_set_size((struct tinyui_widget *)table, 200, 100);
        tinyui_widget_set_selectable((struct tinyui_widget *)table, 1);
    }

    if (line_edit != 0) {
        tinyui_line_edit_set_text(line_edit, "123");
        tinyui_line_edit_set_keyboard_binding(line_edit, 22);
        tinyui_widget_set_pos((struct tinyui_widget *)line_edit, 850, 400);
        tinyui_widget_set_size((struct tinyui_widget *)line_edit, 100, 50);
        tinyui_widget_set_corner((struct tinyui_widget *)line_edit, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)line_edit, 1);
    }

    if (child_window != 0) {
        tinyui_widget_set_pos((struct tinyui_widget *)child_window, 850, 450);
        tinyui_widget_set_size((struct tinyui_widget *)child_window, 100, 100);
        nested_button = tinyui_button_create(child_window, "demo0_nested_button");
        if (nested_button != 0) {
            tinyui_button_set_text(nested_button, "123");
            tinyui_widget_set_pos((struct tinyui_widget *)nested_button, 8, 3);
            tinyui_widget_set_size((struct tinyui_widget *)nested_button, 30, 30);
        }
    }

    if (keyboard != 0) {
        tinyui_widget_set_pos((struct tinyui_widget *)keyboard, 780, 450);
    }

    if (arc != 0) {
        tinyui_arc_set_quarter_source(arc, &g_sources.arc_quarter);
        tinyui_arc_set_background_angle(arc, 0.0f, 350.0f);
        tinyui_arc_set_foreground_angle(arc, 30.0f);
        tinyui_arc_set_color(arc, 0xADD8E6U, 0x90EE90U);
        tinyui_widget_set_pos((struct tinyui_widget *)arc, 450, 450);
        tinyui_widget_set_size((struct tinyui_widget *)arc, 103, 103);
    }

    if (list != 0) {
        tinyui_list_set_item_height(list, 30);
        tinyui_list_add_item(list, g_scroll_ids[0], g_scroll_ids[0]);
        tinyui_list_add_item(list, g_scroll_ids[1], g_scroll_ids[1]);
        tinyui_list_add_item(list, g_scroll_ids[2], g_scroll_ids[2]);
        tinyui_list_add_item(list, g_scroll_ids[3], g_scroll_ids[3]);
        tinyui_list_add_item(list, g_scroll_ids[4], g_scroll_ids[4]);
        tinyui_list_set_align(list, TINYUI_ALIGN_START);
        tinyui_widget_set_pos((struct tinyui_widget *)list, 850, 280);
        tinyui_widget_set_size((struct tinyui_widget *)list, 100, 100);
        tinyui_widget_set_selectable((struct tinyui_widget *)list, 1);
        list_item_button = tinyui_button_create(win, "demo0_list_item_button");
        if (list_item_button != 0) {
            tinyui_button_set_text(list_item_button, "1");
            tinyui_widget_set_pos((struct tinyui_widget *)list_item_button, 10, 3);
            tinyui_widget_set_size((struct tinyui_widget *)list_item_button, 20, 20);
            tinyui_list_set_item_widget(list, 1, (struct tinyui_widget *)list_item_button);
        }
    }

    if (message_box != 0) {
        tinyui_message_box_set_title(message_box, "title");
        tinyui_message_box_set_message(message_box, "12345678abcdefg\n99556");
        tinyui_message_box_set_buttons(message_box, g_message_buttons, 3);
        tinyui_widget_set_pos((struct tinyui_widget *)message_box, 200, 150);
    }

    if (calendar != 0) {
        tinyui_calendar_set_date(calendar, 2026, 1, 1);
        tinyui_calendar_set_day_names(calendar, g_calendar_day_names);
        tinyui_calendar_set_header_visible(calendar, 1);
        tinyui_calendar_set_header_format(calendar, "yyyy - mm - dd");
        tinyui_widget_set_pos((struct tinyui_widget *)calendar, 50, 340);
        tinyui_widget_set_size((struct tinyui_widget *)calendar, 300, 150);
        tinyui_widget_set_corner((struct tinyui_widget *)calendar, 1);
        tinyui_widget_set_selectable((struct tinyui_widget *)calendar, 1);
        tinyui_widget_set_selected((struct tinyui_widget *)calendar, 1);
    }
}

static void update_runtime_angle(struct legacy_demo0_runtime *runtime)
{
    if (runtime == 0) {
        return;
    }

    tinyui_arc_set_rotation_angle(runtime->arc, runtime->angle);
    tinyui_gauge_set_angle(runtime->gauge, runtime->angle);
    runtime->angle += 1.0f;
    if (runtime->angle >= 360.0f) {
        runtime->angle = 0.0f;
    }
}

void tinyui_demo_legacy_demo0_parity_frame(unsigned int elapsed_ms)
{
    struct legacy_demo0_runtime *runtime = &g_runtime;

    if (runtime->gauge == 0 || runtime->arc == 0) {
        return;
    }

    runtime->frame_accumulated_ms += elapsed_ms;
    while (runtime->frame_accumulated_ms >= LEGACY_DEMO0_FRAME_INTERVAL_MS) {
        runtime->frame_accumulated_ms -= LEGACY_DEMO0_FRAME_INTERVAL_MS;
        update_runtime_angle(runtime);
    }
}

void tinyui_demo_legacy_demo0_parity(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    (void)g_legacy_demo0_truth_fields;

    if (win == 0) {
        return;
    }

    g_runtime.image = 0;
    g_runtime.switch_label = 0;
    g_runtime.gauge = 0;
    g_runtime.arc = 0;
    g_runtime.angle = 120.0f;
    g_runtime.frame_accumulated_ms = 0;
    build_legacy_demo0(win, &g_runtime);
    tinyui_screen_load(screen);
}
