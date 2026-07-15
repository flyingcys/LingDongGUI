/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * M3 Task 10 家族场景：text / line_edit / keyboard / table / graph。
 * 固定坐标；line_edit finished 与 keyboard key 走 L5-E。
 */

#include "v23_input_data/v23_input_data.h"
#include "tinyui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static tinyui_obj_t *g_line_edit;
static tinyui_obj_t *g_keyboard;
static int g_script_step;

static void log_event_trace(const char *line)
{
    if (line == NULL || line[0] == '\0') {
        return;
    }
    printf("TINYUI_EVENT_TRACE_LINE=%s\n", line);
    fflush(stdout);
}

static void on_line_edit_finished(tinyui_obj_t *line_edit, void *user_data)
{
    (void)line_edit;
    (void)user_data;
    log_event_trace("line_edit:FINISHED");
}

static void on_keyboard_key(tinyui_obj_t *keyboard,
                            unsigned int key_code,
                            tinyui_signal_t signal,
                            void *user_data)
{
    char line[64];

    (void)keyboard;
    (void)user_data;
    /* Log every key signal so L5-E can assert at least one KEY line. */
    (void)signal;
    snprintf(line,
             sizeof(line),
             "keyboard:KEY:%u",
             (unsigned int)key_code);
    log_event_trace(line);
}

static int script_events_enabled(void)
{
    const char *script = getenv("TINYUI_SCRIPT_EVENTS");
    const char *scenario = getenv("TINYUI_SCENARIO");

    if (script != NULL && script[0] != '\0' && script[0] != '0') {
        return 1;
    }
    return (scenario != NULL && strcmp(scenario, "v23_input_data") == 0) ? 1 : 0;
}

static int make_ui(tinyui_obj_t *screen)
{
    tinyui_obj_t *text;
    tinyui_obj_t *line_edit;
    tinyui_obj_t *keyboard;
    tinyui_obj_t *table;
    tinyui_obj_t *graph;
    tinyui_table_props_t table_props;
    tinyui_graph_props_t graph_props;

    if (screen == NULL) {
        return -1;
    }

    (void)tinyui_obj_set_bg_color(screen, 0xF6F8FAU);

    memset(&table_props, 0, sizeof(table_props));
    table_props.fields = TINYUI_TABLE_FIELD_ROWS | TINYUI_TABLE_FIELD_COLUMNS
        | TINYUI_TABLE_FIELD_WIDTH | TINYUI_TABLE_FIELD_HEIGHT;
    table_props.rows = 2;
    table_props.columns = 2;
    table_props.width = 144;
    table_props.height = 120;

    memset(&graph_props, 0, sizeof(graph_props));
    graph_props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX | TINYUI_GRAPH_FIELD_WIDTH
        | TINYUI_GRAPH_FIELD_HEIGHT;
    graph_props.series_max = 2;
    graph_props.width = 144;
    graph_props.height = 120;

    text = tinyui_text_create(screen);
    line_edit = tinyui_line_edit_create(screen);
    keyboard = tinyui_keyboard_create(screen);
    table = tinyui_table_create_with_props(screen, &table_props);
    graph = tinyui_graph_create_with_props(screen, &graph_props);
    if (text == NULL || line_edit == NULL || keyboard == NULL || table == NULL
        || graph == NULL) {
        return -1;
    }
    g_line_edit = line_edit;
    g_keyboard = keyboard;
    g_script_step = 0;

    if (tinyui_obj_set_pos(text, 16, 12) != TINYUI_OK
        || tinyui_obj_set_size(text, 200, 28) != TINYUI_OK
        || tinyui_text_set_text(text, "Input Data") != 0
        || tinyui_text_set_text_color(text, 0x102030U) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(line_edit, 16, 48) != TINYUI_OK
        || tinyui_obj_set_size(line_edit, 200, 32) != TINYUI_OK
        || tinyui_line_edit_set_text(line_edit, "edit-me") != 0
        || tinyui_line_edit_set_on_edit_finished(line_edit, on_line_edit_finished, NULL)
            != 0) {
        return -1;
    }

    /* Keyboard is full-screen and starts hidden in LD; bind + update unhides. */
    if (tinyui_keyboard_set_on_key_event(keyboard, on_keyboard_key, NULL) != 0
        || tinyui_line_edit_set_keyboard_widget(line_edit, keyboard) != 0
        || tinyui_keyboard_update(keyboard) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(table, 320, 12) != TINYUI_OK
        || tinyui_table_set_item_width(table, 0, 48) != 0
        || tinyui_table_set_item_width(table, 1, 48) != 0
        || tinyui_table_set_item_height(table, 0, 28) != 0
        || tinyui_table_set_item_height(table, 1, 28) != 0
        || tinyui_table_set_cell_text(table, 0, 0, "A1") != 0
        || tinyui_table_set_cell_text(table, 0, 1, "B1") != 0
        || tinyui_table_set_cell_text(table, 1, 0, "A2") != 0
        || tinyui_table_set_cell_text(table, 1, 1, "B2") != 0
        || tinyui_table_set_bg_color(table, 0xE8EEF5U) != 0) {
        return -1;
    }

    if (tinyui_obj_set_pos(graph, 320, 148) != TINYUI_OK
        || tinyui_graph_set_axis(graph, 4, 4) != 0
        || tinyui_graph_add_series(graph, 0x1F6FEBU, 2, 8) != 0
        || tinyui_graph_set_value(graph, 0, 0, 10) != 0
        || tinyui_graph_set_value(graph, 0, 1, 30) != 0
        || tinyui_graph_set_value(graph, 0, 2, 20) != 0
        || tinyui_graph_set_value(graph, 0, 3, 40) != 0) {
        return -1;
    }

    printf("TINYUI_SCENARIO=v23_input_data\n");
    fflush(stdout);
    return 0;
}

void tinyui_demo_v23_input_data(void)
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

void tinyui_demo_v23_input_data_frame(unsigned int elapsed_ms)
{
    (void)elapsed_ms;
    if (!script_events_enabled() || g_keyboard == NULL || g_line_edit == NULL) {
        return;
    }
    /* Keyboard pointer hit-test is layout/config sensitive on host.
     * Drive L5-E through public focus + key select + click APIs so the
     * real on_key_event callback path is exercised without fake printf. */
    switch (g_script_step) {
    case 2:
        (void)tinyui_focus_set(g_line_edit);
        break;
    case 4:
        (void)tinyui_focus_set(g_keyboard);
        (void)tinyui_keyboard_button_update(g_keyboard, (unsigned int)'q');
        (void)tinyui_keyboard_click(g_keyboard);
        break;
    default:
        break;
    }
    g_script_step += 1;
}
