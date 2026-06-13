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

#include "legacy_widget_parity/legacy_widget_parity.h"
#include "tinyui.h"
#include "../../../examples/common/demo/widget/images/uiImages.h"

struct legacy_widget_runtime {
    struct picoui_gauge *gauge;
    struct picoui_arc *arc;
};

static struct picoui_image_source s_legacy_image_source = {
    .img_tile = IMAGE_LETTER_PAPER_BMP,
    .mask_tile = NULL,
};

static struct picoui_image_source s_legacy_button_release_source = {
    .img_tile = IMAGE_KEYRELEASE_PNG,
    .mask_tile = IMAGE_KEYRELEASE_PNG_Mask,
};

static struct picoui_image_source s_legacy_button_press_source = {
    .img_tile = IMAGE_KEYPRESS_PNG,
    .mask_tile = IMAGE_KEYPRESS_PNG_Mask,
};

static struct picoui_image_source s_legacy_progress_bg_source = {
    .img_tile = IMAGE_PROGRESSBARBG_BMP,
    .mask_tile = NULL,
};

static struct picoui_image_source s_legacy_progress_fg_source = {
    .img_tile = IMAGE_PROGRESSBARFG_BMP,
    .mask_tile = NULL,
};

static struct picoui_image_source s_legacy_text_bg_source = {
    .img_tile = IMAGE_LETTER_PAPER_BMP,
    .mask_tile = NULL,
};

static struct picoui_image_source s_legacy_slider_bg_source = {
    .img_tile = IMAGE_SLIDER_PNG,
    .mask_tile = IMAGE_SLIDER_PNG_Mask,
};

static struct picoui_image_source s_legacy_slider_indicator_source = {
    .img_tile = IMAGE_INDICATOR_PNG,
    .mask_tile = IMAGE_INDICATOR_PNG_Mask,
};

static struct picoui_image_source s_legacy_radial_weather_source = {
    .img_tile = IMAGE_WEATHER_PNG,
    .mask_tile = IMAGE_WEATHER_PNG_Mask,
};

static struct picoui_image_source s_legacy_radial_note_source = {
    .img_tile = IMAGE_NOTE_PNG,
    .mask_tile = IMAGE_NOTE_PNG_Mask,
};

static struct picoui_image_source s_legacy_icon_note_source = {
    .img_tile = IMAGE_NOTE_PNG,
    .mask_tile = IMAGE_NOTE_PNG_Mask,
};

static struct picoui_image_source s_legacy_icon_book_source = {
    .img_tile = IMAGE_BOOK_PNG,
    .mask_tile = IMAGE_BOOK_PNG_Mask,
};

static struct picoui_image_source s_legacy_icon_weather_source = {
    .img_tile = IMAGE_WEATHER_PNG,
    .mask_tile = IMAGE_WEATHER_PNG_Mask,
};

static struct picoui_image_source s_legacy_icon_chart_source = {
    .img_tile = IMAGE_CHART_PNG,
    .mask_tile = IMAGE_CHART_PNG_Mask,
};

static struct picoui_image_source s_legacy_gauge_bg_source = {
    .img_tile = IMAGE_GAUGE_PNG,
    .mask_tile = IMAGE_GAUGE_PNG_Mask,
};

static struct picoui_image_source s_legacy_gauge_pointer_source = {
    .img_tile = IMAGE_GAUGEPOINTER_PNG,
    .mask_tile = IMAGE_GAUGEPOINTER_PNG_Mask,
};

static struct picoui_image_source s_legacy_arc_quarter_source = {
    .img_tile = IMAGE_ARC_QUARTER_PNG_Mask,
    .mask_tile = IMAGE_ARC_QUARTER_MASK_PNG_Mask,
};

static void on_switch_changed(struct picoui_widget *widget, int value, void *user_data)
{
    struct picoui_label *status = (struct picoui_label *)user_data;

    (void)widget;
    if (status != 0) {
        picoui_label_set_text(status, value != 0 ? "ON" : "OFF");
    }
}

static void seed_graph(struct picoui_graph *graph)
{
    int cpu_series;
    int mem_series;

    cpu_series = picoui_graph_add_series(graph, 0xD62828U, 2, 6);
    mem_series = picoui_graph_add_series(graph, 0x457B9DU, 2, 6);
    if (cpu_series >= 0) {
        picoui_graph_set_value(graph, cpu_series, 0, 18);
        picoui_graph_set_value(graph, cpu_series, 1, 26);
        picoui_graph_set_value(graph, cpu_series, 2, 35);
        picoui_graph_set_value(graph, cpu_series, 3, 48);
        picoui_graph_set_value(graph, cpu_series, 4, 38);
        picoui_graph_set_value(graph, cpu_series, 5, 44);
    }
    if (mem_series >= 0) {
        picoui_graph_set_value(graph, mem_series, 0, 12);
        picoui_graph_set_value(graph, mem_series, 1, 19);
        picoui_graph_set_value(graph, mem_series, 2, 25);
        picoui_graph_set_value(graph, mem_series, 3, 28);
        picoui_graph_set_value(graph, mem_series, 4, 32);
        picoui_graph_set_value(graph, mem_series, 5, 30);
    }
}

static void seed_table(struct picoui_table *table)
{
    picoui_table_set_cell_text(table, 0, 0, "Mon");
    picoui_table_set_cell_text(table, 0, 1, "Tue");
    picoui_table_set_cell_text(table, 0, 2, "Wed");
    picoui_table_set_cell_text(table, 1, 0, "09:00");
    picoui_table_set_cell_text(table, 1, 1, "Build");
    picoui_table_set_cell_text(table, 1, 2, "Review");
    picoui_table_set_cell_text(table, 2, 0, "13:30");
    picoui_table_set_cell_text(table, 2, 1, "Demo");
    picoui_table_set_cell_text(table, 2, 2, "Ship");
    picoui_table_set_current_cell(table, 1, 1);
}

static void make_ui(struct picoui_window *win, struct legacy_widget_runtime *runtime)
{
    static const char *combo_ids[] = {"low", "mid", "high"};
    static const char *combo_texts[] = {"Low", "Medium", "High"};
    static const char *scroll_ids[] = {"one", "two", "three", "four", "five"};
    static const char *scroll_texts[] = {"One", "Two", "Three", "Four", "Five"};
    static const char *message_buttons[] = {"Later", "Apply"};
    static const char *calendar_day_names[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    struct picoui_button *button;
    struct picoui_button *list_item_button;
    struct picoui_button *nested_button;
    struct picoui_image *image;
    struct picoui_label *panel;
    struct picoui_label *label;
    struct picoui_label *switch_label;
    struct picoui_checkbox *radio_a;
    struct picoui_checkbox *radio_b;
    struct picoui_checkbox *check;
    struct picoui_switch *sw;
    struct picoui_progress_bar *bar;
    struct picoui_text *text;
    struct picoui_slider *slider_h;
    struct picoui_slider *slider_v;
    struct picoui_list *list;
    struct picoui_combo_box *combo_box;
    struct picoui_calendar *calendar;
    struct picoui_scroll_selecter *scroll_selecter;
    struct picoui_date_time *date_time;
    struct picoui_message_box *message_box;
    struct picoui_graph *graph;
    struct picoui_table *table;
    struct picoui_radial_menu *radial_menu;
    struct picoui_icon_slider *icon_slider;
    struct picoui_qrcode *qrcode;
    struct picoui_gauge *gauge;
    struct picoui_line_edit *line_edit;
    struct picoui_keyboard *keyboard;
    struct picoui_arc *arc;
    struct picoui_window *child_window;

    picoui_window_set_color(win, 0xF5F6F8U);

    image = picoui_image_create(win, "legacy_image");
    button = picoui_button_create(win, "legacy_button");
    panel = picoui_label_create(win, "legacy_panel");
    label = picoui_label_create(win, "legacy_label");
    radio_a = picoui_checkbox_create(win, "legacy_radio_a");
    radio_b = picoui_checkbox_create(win, "legacy_radio_b");
    check = picoui_checkbox_create(win, "legacy_check");
    sw = picoui_switch_create(win, "legacy_switch");
    switch_label = picoui_label_create(win, "legacy_switch_label");
    bar = picoui_progress_bar_create(win, "legacy_progress");
    text = picoui_text_create(win, "legacy_text");
    slider_h = picoui_slider_create(win, "legacy_slider_h");
    slider_v = picoui_slider_create(win, "legacy_slider_v");
    list = picoui_list_create((struct picoui_widget *)win, "legacy_list");
    combo_box = picoui_combo_box_create(win, "legacy_combo");
    calendar = picoui_calendar_create(win, "legacy_calendar");
    scroll_selecter = picoui_scroll_selecter_create(win, "legacy_scroll");
    date_time = picoui_date_time_create((struct picoui_widget *)win, "legacy_date_time");
    message_box = picoui_message_box_create((struct picoui_widget *)win, "legacy_message");
    graph = picoui_graph_create(win, "legacy_graph", 2);
    table = picoui_table_create(win, "legacy_table", 3, 3);
    radial_menu = picoui_radial_menu_create((struct picoui_widget *)win, "legacy_radial_menu");
    icon_slider = picoui_icon_slider_create((struct picoui_widget *)win, "legacy_icon_slider");
    qrcode = picoui_qrcode_create((struct picoui_widget *)win, "legacy_qrcode");
    gauge = picoui_gauge_create((struct picoui_widget *)win, "legacy_gauge");
    line_edit = picoui_line_edit_create_with_props(
        win,
        &(struct picoui_line_edit_props){
            .id = "legacy_line_edit",
            .text = "123",
            .type = PICOUI_LINE_EDIT_TYPE_STRING,
            .keyboard_binding = 1U,
            .has_type = 1,
            .has_keyboard_binding = 1,
            .width = 100,
            .height = 32,
        });
    keyboard = picoui_keyboard_create_with_props(
        win,
        &(struct picoui_keyboard_props){
            .id = "legacy_keyboard",
            .width = 180,
            .height = 112,
        });
    arc = picoui_arc_create_with_props(
        (struct picoui_widget *)win,
        &(struct picoui_arc_props){
            .id = "legacy_arc",
            .bg_start_angle = 0.0f,
            .bg_end_angle = 350.0f,
            .fg_end_angle = 30.0f,
            .rotation_angle = 120.0f,
            .bg_color = 0xADD8E6U,
            .fg_color = 0x90EE90U,
        });
    child_window = picoui_window_create_child(win, "legacy_child_panel");

    if (image != 0) {
        picoui_image_set_source(image, &s_legacy_image_source);
        picoui_widget_set_pos((struct picoui_widget *)image, 100, 120);
        picoui_widget_set_size((struct picoui_widget *)image, 80, 50);
        picoui_widget_set_bg_color((struct picoui_widget *)image, 0xE9ECEFU);
        picoui_widget_set_border_color((struct picoui_widget *)image, 0x9AA0A6U);
        picoui_widget_set_radius((struct picoui_widget *)image, 6);
    }

    if (button != 0) {
        picoui_button_set_text(button, "123");
        picoui_button_set_image(button,
                                &s_legacy_button_release_source,
                                &s_legacy_button_press_source);
        picoui_widget_set_pos((struct picoui_widget *)button, 10, 10);
        picoui_widget_set_size((struct picoui_widget *)button, 88, 56);
        picoui_widget_set_bg_color((struct picoui_widget *)button, 0x1D3557U);
        picoui_button_set_text_color(button, 0xFFFFFFU);
        picoui_widget_set_radius((struct picoui_widget *)button, 8);
    }

    if (panel != 0) {
        picoui_label_set_text(panel, "");
        picoui_label_set_bg_color(panel, 0x2A9D8FU);
        picoui_widget_set_pos((struct picoui_widget *)panel, 200, 95);
        picoui_widget_set_size((struct picoui_widget *)panel, 24, 24);
        picoui_widget_set_radius((struct picoui_widget *)panel, 6);
    }

    if (label != 0) {
        picoui_label_set_text(label, "123");
        picoui_label_set_bg_color(label, 0xD9D9D9U);
        picoui_label_set_align(label, PICOUI_ALIGN_END);
        picoui_widget_set_pos((struct picoui_widget *)label, 100, 50);
        picoui_widget_set_size((struct picoui_widget *)label, 100, 50);
        picoui_widget_set_radius((struct picoui_widget *)label, 8);
    }

    if (radio_a != 0) {
        picoui_checkbox_set_text(radio_a, "999");
        picoui_checkbox_set_radio_group(radio_a, 1);
        picoui_checkbox_set_checked(radio_a, 1);
        picoui_widget_set_pos((struct picoui_widget *)radio_a, 220, 10);
        picoui_widget_set_size((struct picoui_widget *)radio_a, 72, 20);
    }

    if (radio_b != 0) {
        picoui_checkbox_set_text(radio_b, "radio");
        picoui_checkbox_set_radio_group(radio_b, 1);
        picoui_widget_set_pos((struct picoui_widget *)radio_b, 220, 40);
        picoui_widget_set_size((struct picoui_widget *)radio_b, 72, 20);
    }

    if (check != 0) {
        picoui_checkbox_set_text(check, "check");
        picoui_checkbox_set_checked(check, 1);
        picoui_widget_set_pos((struct picoui_widget *)check, 220, 70);
        picoui_widget_set_size((struct picoui_widget *)check, 72, 20);
    }

    if (sw != 0) {
        picoui_switch_set_checked(sw, 0);
        picoui_widget_set_pos((struct picoui_widget *)sw, 310, 116);
        picoui_widget_set_size((struct picoui_widget *)sw, 56, 28);
    }
    if (switch_label != 0) {
        picoui_label_set_text(switch_label, "OFF");
        picoui_widget_set_pos((struct picoui_widget *)switch_label, 374, 114);
        picoui_widget_set_size((struct picoui_widget *)switch_label, 48, 24);
    }
    if (sw != 0 && switch_label != 0) {
        picoui_switch_set_on_toggled(sw, on_switch_changed, switch_label);
    }

    if (bar != 0) {
        picoui_progress_bar_set_percent(bar, 45);
        picoui_progress_bar_set_image(bar,
                                      &s_legacy_progress_bg_source,
                                      &s_legacy_progress_fg_source);
        picoui_progress_bar_set_color(bar, 0xCED4DAU, 0x457B9DU);
        picoui_progress_bar_set_frame_color(bar, 0x6C757DU, 1);
        picoui_widget_set_pos((struct picoui_widget *)bar, 10, 500);
        picoui_widget_set_size((struct picoui_widget *)bar, 300, 30);
    }

    if (text != 0) {
        picoui_text_set_text(text, "123\n12333");
        picoui_text_set_background_source(text, &s_legacy_text_bg_source);
        picoui_text_set_bg_color(text, 0xF2E8CFU);
        picoui_widget_set_pos((struct picoui_widget *)text, 300, 10);
        picoui_widget_set_size((struct picoui_widget *)text, 150, 200);
        picoui_widget_set_radius((struct picoui_widget *)text, 10);
    }

    if (slider_h != 0) {
        picoui_slider_set_percent(slider_h, 42);
        picoui_slider_set_image(slider_h,
                                &s_legacy_slider_bg_source,
                                &s_legacy_slider_indicator_source);
        picoui_slider_set_color(slider_h, 0xCED4DAU, 0xADB5BDU, 0xD62828U);
        picoui_widget_set_pos((struct picoui_widget *)slider_h, 50, 300);
        picoui_widget_set_size((struct picoui_widget *)slider_h, 317, 24);
    }

    if (slider_v != 0) {
        picoui_slider_set_horizontal(slider_v, 0);
        picoui_slider_set_percent(slider_v, 42);
        picoui_slider_set_image(slider_v,
                                &s_legacy_slider_bg_source,
                                &s_legacy_slider_indicator_source);
        picoui_slider_set_color(slider_v, 0xCED4DAU, 0xADB5BDU, 0x2A9D8FU);
        picoui_widget_set_pos((struct picoui_widget *)slider_v, 400, 300);
        picoui_widget_set_size((struct picoui_widget *)slider_v, 30, 110);
        picoui_widget_set_radius((struct picoui_widget *)slider_v, 6);
    }

    if (list != 0) {
        picoui_list_add_item(list, "weather", "Weather");
        picoui_list_add_item(list, "note", "Note");
        picoui_list_add_item(list, "chart", "Chart");
        list_item_button = picoui_button_create(win, "legacy_list_item_button");
        if (list_item_button != 0) {
            picoui_button_set_text(list_item_button, "1");
            picoui_widget_set_size((struct picoui_widget *)list_item_button, 20, 20);
            picoui_list_set_item_widget(list, 1, (struct picoui_widget *)list_item_button);
        }
        picoui_list_set_selected_index(list, 1);
        picoui_widget_set_pos((struct picoui_widget *)list, 500, 210);
        picoui_widget_set_size((struct picoui_widget *)list, 170, 120);
    }

    if (combo_box != 0) {
        picoui_combo_box_set_static_items(combo_box, combo_ids, combo_texts, 3);
        picoui_combo_box_set_selected_index(combo_box, 1);
        picoui_widget_set_pos((struct picoui_widget *)combo_box, 700, 420);
        picoui_widget_set_size((struct picoui_widget *)combo_box, 100, 30);
    }

    if (calendar != 0) {
        picoui_calendar_set_date(calendar, 2026, 6, 4);
        picoui_calendar_set_day_names(calendar, calendar_day_names);
        picoui_calendar_set_header_visible(calendar, 1);
        picoui_calendar_set_header_format(calendar, "yyyy/mm/dd");
        picoui_widget_set_pos((struct picoui_widget *)calendar, 830, 260);
        picoui_widget_set_size((struct picoui_widget *)calendar, 180, 168);
    }

    if (scroll_selecter != 0) {
        picoui_scroll_selecter_set_items(scroll_selecter, scroll_ids, scroll_texts, 5);
        picoui_scroll_selecter_set_selected_index(scroll_selecter, 2);
        picoui_scroll_selecter_set_background_color(scroll_selecter, 0xFFFFFFU);
        picoui_widget_set_pos((struct picoui_widget *)scroll_selecter, 700, 200);
        picoui_widget_set_size((struct picoui_widget *)scroll_selecter, 60, 110);
        picoui_widget_set_radius((struct picoui_widget *)scroll_selecter, 6);
    }

    if (date_time != 0) {
        picoui_date_time_set_format(date_time, "yyyy-mm-dd hh:nn");
        picoui_date_time_set_date(date_time, 2026, 6, 4);
        picoui_date_time_set_time(date_time, 13, 45, 0);
        picoui_widget_set_pos((struct picoui_widget *)date_time, 600, 100);
        picoui_widget_set_size((struct picoui_widget *)date_time, 180, 32);
    }

    if (message_box != 0) {
        picoui_message_box_set_title(message_box, "Update");
        picoui_message_box_set_message(message_box, "Apply settings?");
        picoui_message_box_set_confirm_text(message_box, "OK");
        picoui_message_box_set_buttons(message_box, message_buttons, 2);
        picoui_widget_set_pos((struct picoui_widget *)message_box, 500, 20);
    }

    if (graph != 0) {
        picoui_graph_set_axis(graph, 80, 80);
        picoui_graph_set_grid_offset(graph, 4);
        seed_graph(graph);
        picoui_widget_set_pos((struct picoui_widget *)graph, 830, 10);
        picoui_widget_set_size((struct picoui_widget *)graph, 140, 118);
    }

    if (table != 0) {
        seed_table(table);
        picoui_widget_set_pos((struct picoui_widget *)table, 780, 150);
        picoui_widget_set_size((struct picoui_widget *)table, 200, 100);
    }

    if (radial_menu != 0) {
        picoui_radial_menu_add_item_with_source(radial_menu, "weather", &s_legacy_radial_weather_source);
        picoui_radial_menu_add_item_with_source(radial_menu, "note", &s_legacy_radial_note_source);
        picoui_radial_menu_add_item_with_source(radial_menu, "weather2", &s_legacy_radial_weather_source);
        picoui_radial_menu_add_item_with_source(radial_menu, "note2", &s_legacy_radial_note_source);
        picoui_radial_menu_set_selected_index(radial_menu, 1);
        picoui_widget_set_pos((struct picoui_widget *)radial_menu, 500, 200);
        picoui_widget_set_size((struct picoui_widget *)radial_menu, 150, 100);
    }

    if (icon_slider != 0) {
        picoui_icon_slider_add_item_with_source(icon_slider, "11", "11", &s_legacy_icon_note_source);
        picoui_icon_slider_add_item_with_source(icon_slider, "22", "22", &s_legacy_icon_book_source);
        picoui_icon_slider_add_item_with_source(icon_slider, "33", "33", &s_legacy_icon_weather_source);
        picoui_icon_slider_add_item_with_source(icon_slider, "44", "44", &s_legacy_icon_chart_source);
        picoui_icon_slider_add_item_with_source(icon_slider, "55", "55", &s_legacy_icon_note_source);
        picoui_icon_slider_set_selected_index(icon_slider, 1);
        picoui_widget_set_pos((struct picoui_widget *)icon_slider, 500, 350);
        picoui_widget_set_size((struct picoui_widget *)icon_slider, 170, 86);
    }

    if (qrcode != 0) {
        picoui_qrcode_set_text(qrcode, "gui-demo");
        picoui_qrcode_set_qr_color(qrcode, 0x0000FFU);
        picoui_qrcode_set_bg_color(qrcode, 0xFFFFFFU);
        picoui_qrcode_set_max_version(qrcode, 2);
        picoui_qrcode_set_zoom(qrcode, 5);
        picoui_widget_set_pos((struct picoui_widget *)qrcode, 500, 10);
        picoui_widget_set_size((struct picoui_widget *)qrcode, 180, 180);
    }

    if (gauge != 0) {
        picoui_gauge_set_angle(gauge, 120.0f);
        picoui_gauge_set_bg_source(gauge, &s_legacy_gauge_bg_source);
        picoui_gauge_set_pointer_source(gauge, &s_legacy_gauge_pointer_source);
        picoui_gauge_set_pointer_color(gauge, 0x0000FFU);
        picoui_widget_set_pos((struct picoui_widget *)gauge, 700, 300);
        picoui_widget_set_size((struct picoui_widget *)gauge, 120, 98);
    }

    if (line_edit != 0) {
        picoui_widget_set_pos((struct picoui_widget *)line_edit, 850, 400);
    }

    if (keyboard != 0) {
        picoui_widget_set_pos((struct picoui_widget *)keyboard, 780, 450);
    }

    if (arc != 0) {
        picoui_arc_set_quarter_source(arc, &s_legacy_arc_quarter_source);
        picoui_widget_set_pos((struct picoui_widget *)arc, 450, 450);
        picoui_widget_set_size((struct picoui_widget *)arc, 103, 103);
    }

    if (runtime != 0) {
        runtime->gauge = gauge;
        runtime->arc = arc;
    }

    if (child_window != 0) {
        picoui_window_set_color(child_window, 0xFFFFFFU);
        picoui_widget_set_pos((struct picoui_widget *)child_window, 850, 450);
        picoui_widget_set_size((struct picoui_widget *)child_window, 100, 100);
        nested_button = picoui_button_create(child_window, "legacy_nested_button");
        if (nested_button != 0) {
            picoui_button_set_text(nested_button, "123");
            picoui_widget_set_pos((struct picoui_widget *)nested_button, 8, 3);
            picoui_widget_set_size((struct picoui_widget *)nested_button, 30, 30);
        }
    }
}

void tinyui_demo_legacy_widget_parity(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct picoui_window *win = (struct picoui_window *)screen;
    if (win == 0) return;
    make_ui(win, 0);
    tinyui_screen_load(screen);
}
