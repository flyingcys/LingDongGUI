/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * M3 Task 10 家族场景：list / combo_box / scroll_selector / calendar /
 * icon_slider / radial_menu。固定坐标布局；list 提供 L5-E。
 */

#include "v23_selection_collection/v23_selection_collection.h"
#include "tinyui.h"

#include <stdio.h>

static void log_event_trace(const char *line)
{
    if (line == NULL || line[0] == '\0') {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=%s\n", line);
    fflush(stdout);
}

static void on_list_selected(tinyui_obj_t *list, int index, void *user_data)
{
    char line[64];

    (void)list;
    (void)user_data;
    snprintf(line, sizeof(line), "list:SELECTED:%d", index);
    log_event_trace(line);
}

static void on_combo_selected(tinyui_obj_t *combo_box, int index, void *user_data)
{
    char line[64];

    (void)combo_box;
    (void)user_data;
    snprintf(line, sizeof(line), "combo:SELECTED:%d", index);
    log_event_trace(line);
}

static int make_ui(tinyui_obj_t *screen)
{
    tinyui_obj_t *list;
    tinyui_obj_t *combo;
    tinyui_obj_t *selector;
    tinyui_obj_t *calendar;
    tinyui_obj_t *icon_slider;
    tinyui_obj_t *radial;

    if (screen == NULL) {
        return -1;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    list = tinyui_list_create(screen);
    combo = tinyui_combo_box_create(screen);
    selector = tinyui_scroll_selector_create(screen);
    calendar = tinyui_calendar_create(screen);
    icon_slider = tinyui_icon_slider_create(screen);
    radial = tinyui_radial_menu_create(screen);
    if (list == NULL || combo == NULL || selector == NULL || calendar == NULL
        || icon_slider == NULL || radial == NULL) {
        return -1;
    }

    if (tinyui_obj_set_pos(list, 16, 16) != TINYUI_OK
        || tinyui_obj_set_size(list, 180, 140) != TINYUI_OK
        || tinyui_list_set_item_height(list, 28) != 0
        || tinyui_list_add_item(list, "a", "Alpha") != 0
        || tinyui_list_add_item(list, "b", "Beta") != 0
        || tinyui_list_add_item(list, "c", "Gamma") != 0
        || tinyui_list_set_selected_index(list, 0) != 0
        || tinyui_list_set_text_color(list, 0x102030U) != 0
        || tinyui_list_set_select_color(list, 0x1F6FEBU) != 0) {
        return -1;
    }
    tinyui_list_set_on_selected(list, on_list_selected, NULL);

    if (tinyui_obj_set_pos(combo, 220, 16) != TINYUI_OK
        || tinyui_obj_set_size(combo, 160, 36) != TINYUI_OK
        || tinyui_combo_box_add_item(combo, "one", "One") != 0
        || tinyui_combo_box_add_item(combo, "two", "Two") != 0
        || tinyui_combo_box_add_item(combo, "three", "Three") != 0
        || tinyui_combo_box_set_selected_index(combo, 1) != 0) {
        return -1;
    }
    tinyui_combo_box_set_on_selected(combo, on_combo_selected, NULL);

    if (tinyui_obj_set_pos(selector, 220, 68) != TINYUI_OK
        || tinyui_obj_set_size(selector, 80, 120) != TINYUI_OK
        || tinyui_scroll_selector_add_item(selector, "0", "0") != 0
        || tinyui_scroll_selector_add_item(selector, "1", "1") != 0
        || tinyui_scroll_selector_add_item(selector, "2", "2") != 0
        || tinyui_scroll_selector_set_selected_index(selector, 1) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(calendar, 320, 16) != TINYUI_OK
        || tinyui_obj_set_size(calendar, 144, 140) != TINYUI_OK
        || tinyui_calendar_set_date(calendar, 2026, 7, 15) != 0
        || tinyui_calendar_set_text_color(calendar, 0x102030U) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(icon_slider, 16, 180) != TINYUI_OK
        || tinyui_obj_set_size(icon_slider, 200, 48) != TINYUI_OK
        || tinyui_icon_slider_add_item(icon_slider, "i0", "A") != 0
        || tinyui_icon_slider_add_item(icon_slider, "i1", "B") != 0
        || tinyui_icon_slider_add_item(icon_slider, "i2", "C") != 0
        || tinyui_icon_slider_set_selected_index(icon_slider, 0) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(radial, 260, 180) != TINYUI_OK
        || tinyui_obj_set_size(radial, 120, 120) != TINYUI_OK
        || tinyui_radial_menu_add_item(radial, "r0") != 0
        || tinyui_radial_menu_add_item(radial, "r1") != 0
        || tinyui_radial_menu_add_item(radial, "r2") != 0
        || tinyui_radial_menu_set_selected_index(radial, 0) != 0) {
        return -1;
    }

    printf("TINYUI_SCENARIO=v23_selection_collection\n");
    fflush(stdout);
    return 0;
}

void tinyui_demo_v23_selection_collection(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();

    if (screen == NULL) {
        return;
    }
    if (make_ui(screen) != 0) {
        return;
    }
    (void)tinyui_screen_load(screen, TINYUI_SCREEN_TRANSITION_NONE, 0);
}

void tinyui_demo_v23_selection_collection_frame(unsigned int elapsed_ms)
{
    (void)elapsed_ms;
}
