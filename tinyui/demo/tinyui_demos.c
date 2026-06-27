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

/*
 * TinyUI demo dispatcher — mirrors lvgl/demos/lv_demos.c.
 *
 * A name→function table maps demo name strings to their entry points.
 * tinyui_demos_create(argv+1, argc-1) picks one by name and calls it.
 *
 *   ./tinyui_demo arc_basic
 *   ./tinyui_demo hello_world
 */

#include "tinyui_demos.h"
#include "display/display.h"
#include <string.h>
#include <stdio.h>

typedef void (*demo_method_cb)(void);

typedef struct {
    const char     *name;
    demo_method_cb  entry_cb;
    tinyui_demo_frame_cb_t frame_cb;
    int             display_width;
    int             display_height;
} demo_entry_info_t;

static const demo_entry_info_t demos_entry_info[] = {
    { "animation_basic",       tinyui_demo_animation_basic,       NULL,                                  480,  320 },
    { "arc_basic",             tinyui_demo_arc_basic,             NULL,                                  480,  320 },
    { "basic_widgets",         tinyui_demo_basic_widgets,         NULL,                                  480,  320 },
    { "calendar_basic",        tinyui_demo_calendar_basic,        NULL,                                  480,  320 },
    { "clock_basic",           tinyui_demo_clock_basic,           NULL,                                  480,  320 },
    { "combo_box_basic",       tinyui_demo_combo_box_basic,       NULL,                                  480,  320 },
    { "date_time_basic",       tinyui_demo_date_time_basic,       NULL,                                  480,  320 },
    { "gauge_basic",           tinyui_demo_gauge_basic,           NULL,                                  480,  320 },
    { "graph_basic",           tinyui_demo_graph_basic,           NULL,                                  480,  320 },
    { "grid_parity",           tinyui_demo_grid_parity,           NULL,                                  480,  320 },
    { "hello_world",           tinyui_demo_hello_world,           NULL,                                  480,  320 },
    { "icon_slider_basic",     tinyui_demo_icon_slider_basic,     NULL,                                  480,  320 },
    { "keyboard_basic",        tinyui_demo_keyboard_basic,        NULL,                                  480,  320 },
    { "layout_flex",           tinyui_demo_layout_flex,           NULL,                                  480,  320 },
    { "layout_grid",           tinyui_demo_layout_grid,           NULL,                                  480,  320 },
    { "layout_parity",         tinyui_demo_layout_parity,         NULL,                                  480,  320 },
    { "legacy_demo0_parity",   tinyui_demo_legacy_demo0_parity,   tinyui_demo_legacy_demo0_parity_frame, 1024, 600 },
    { "legacy_widget_parity",  tinyui_demo_legacy_widget_parity,  NULL,                                  480,  320 },
    { "line_edit_basic",       tinyui_demo_line_edit_basic,       NULL,                                  480,  320 },
    { "list_basic",            tinyui_demo_list_basic,            NULL,                                  480,  320 },
    { "message_box_basic",     tinyui_demo_message_box_basic,     NULL,                                  480,  320 },
    { "progress_bar_basic",    tinyui_demo_progress_bar_basic,    NULL,                                  480,  320 },
    { "progress_wheel_basic",  tinyui_demo_progress_wheel_basic,  NULL,                                  480,  320 },
    { "qrcode_basic",          tinyui_demo_qrcode_basic,          NULL,                                  480,  320 },
    { "radial_menu_basic",     tinyui_demo_radial_menu_basic,     NULL,                                  480,  320 },
    { "scroll_selecter_basic", tinyui_demo_scroll_selecter_basic, NULL,                                  480,  320 },
    { "settings_panel",        tinyui_demo_settings_panel,        NULL,                                  480,  320 },
    { "table_basic",           tinyui_demo_table_basic,           NULL,                                  480,  320 },
    { "theme_showcase",        tinyui_demo_theme_showcase,        NULL,                                  480,  320 },
    { "", NULL, NULL, 0, 0 }  /* sentinel */
};

#define TINYUI_DEMOS_COUNT \
    ((int)(sizeof(demos_entry_info) / sizeof(demo_entry_info_t)) - 1)

static const demo_entry_info_t *current_demo_entry;

static const demo_entry_info_t *tinyui_demos_find_entry(char *info[], int size)
{
    const int count = TINYUI_DEMOS_COUNT;

    if (count <= 0) {
        return NULL;
    }

    if (size <= 0) {
        return &demos_entry_info[0];
    }

    if (info != NULL) {
        const char *name = info[0];
        for (int i = 0; i < count; i++) {
            if (strcmp(name, demos_entry_info[i].name) == 0) {
                return &demos_entry_info[i];
            }
        }
    }

    return NULL;
}

bool tinyui_demos_create(char *info[], int size)
{
    const int count = TINYUI_DEMOS_COUNT;
    current_demo_entry = NULL;

    if (count <= 0) {
        printf("tinyui_demos: no demos available\n");
        return false;
    }

    const demo_entry_info_t *entry = tinyui_demos_find_entry(info, size);

    if (entry == NULL) {
        printf("tinyui_demos: demo '%s' not found\n",
               (size > 0 && info) ? info[0] : "");
        return false;
    }

    if (entry->entry_cb) {
        current_demo_entry = entry;
        entry->entry_cb();
        return true;
    }

    return false;
}

bool tinyui_demos_get_display_config(char *info[],
                                     int size,
                                     struct tinyui_display_config *out_config)
{
    const demo_entry_info_t *entry;

    if (out_config == NULL) {
        return false;
    }

    entry = tinyui_demos_find_entry(info, size);
    if (entry == NULL || entry->display_width <= 0 || entry->display_height <= 0) {
        return false;
    }

    out_config->width = entry->display_width;
    out_config->height = entry->display_height;
    out_config->color_format = TINYUI_COLOR_FORMAT_RGB565;
    out_config->buffer_height = 0;
    out_config->user_data = NULL;
    return true;
}

void tinyui_demos_frame(unsigned int elapsed_ms)
{
    if (current_demo_entry != NULL && current_demo_entry->frame_cb != NULL) {
        current_demo_entry->frame_cb(elapsed_ms);
    }
}

void tinyui_demos_show_help(void)
{
    int i;
    const int count = TINYUI_DEMOS_COUNT;

    if (count == 0) {
        printf("tinyui_demos: no demo available!\n");
        return;
    }

    printf("\nUsage: tinyui_demo <demo_name>\n\ndemo list:\n");
    for (i = 0; i < count; i++) {
        printf("    %s\n", demos_entry_info[i].name);
    }
}
