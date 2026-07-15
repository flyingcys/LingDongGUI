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
 * TinyUI demo dispatcher — build(screen) registry only (M4 Task 2).
 *
 * Registry entries hold name / builder / width / height. Runtime lifecycle
 * and per-frame hooks live in tinyui_demo/main.c (or timer callbacks inside
 * migrated builders). Until Task 3–5 migrate each demo, builders are temporary
 * NOT_SUPPORTED stubs defined here so the runner links.
 */

#include "tinyui_demos.h"

#include <stdio.h>
#include <string.h>

typedef struct tinyui_demo_entry {
    const char *name;
    tinyui_demo_build_cb_t build;
    uint16_t width;
    uint16_t height;
} tinyui_demo_entry_t;

/* Temporary weak stubs for unmigrated demos. Task 3–5 provide strong
 * tinyui_demo_<name>_build symbols in per-demo sources; the linker then
 * overrides these stubs without editing the registry. */
#if defined(__GNUC__) || defined(__clang__)
#define TINYUI_DEMO_STUB_ATTR __attribute__((weak))
#else
#define TINYUI_DEMO_STUB_ATTR
#endif

#define TINYUI_DEMO_STUB_BUILD(symbol)                                         \
    TINYUI_DEMO_STUB_ATTR tinyui_result_t symbol(tinyui_obj_t *screen)         \
    {                                                                          \
        (void)screen;                                                          \
        return TINYUI_ERROR_NOT_SUPPORTED;                                     \
    }

TINYUI_DEMO_STUB_BUILD(tinyui_demo_animation_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_arc_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_basic_widgets_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_calendar_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_clock_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_combo_box_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_date_time_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_gauge_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_graph_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_grid_parity_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_hello_world_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_icon_slider_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_keyboard_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_layout_flex_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_layout_grid_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_layout_parity_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_legacy_demo0_parity_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_legacy_widget_parity_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_line_edit_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_list_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_message_box_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_progress_bar_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_progress_wheel_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_qrcode_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_radial_menu_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_scroll_selector_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_settings_panel_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_table_basic_build)
TINYUI_DEMO_STUB_BUILD(tinyui_demo_theme_showcase_build)

static const tinyui_demo_entry_t demos_entry_info[] = {
    { "animation_basic",      tinyui_demo_animation_basic_build,      480,  320 },
    { "arc_basic",            tinyui_demo_arc_basic_build,            480,  320 },
    { "basic_widgets",        tinyui_demo_basic_widgets_build,        480,  320 },
    { "calendar_basic",       tinyui_demo_calendar_basic_build,       480,  320 },
    { "clock_basic",          tinyui_demo_clock_basic_build,          480,  320 },
    { "combo_box_basic",      tinyui_demo_combo_box_basic_build,      480,  320 },
    { "date_time_basic",      tinyui_demo_date_time_basic_build,      480,  320 },
    { "gauge_basic",          tinyui_demo_gauge_basic_build,          480,  320 },
    { "graph_basic",          tinyui_demo_graph_basic_build,          480,  320 },
    { "grid_parity",          tinyui_demo_grid_parity_build,          480,  320 },
    { "hello_world",          tinyui_demo_hello_world_build,          480,  320 },
    { "icon_slider_basic",    tinyui_demo_icon_slider_basic_build,    480,  320 },
    { "keyboard_basic",       tinyui_demo_keyboard_basic_build,       480,  320 },
    { "layout_flex",          tinyui_demo_layout_flex_build,          480,  320 },
    { "layout_grid",          tinyui_demo_layout_grid_build,          480,  320 },
    { "layout_parity",        tinyui_demo_layout_parity_build,        480,  320 },
    { "legacy_demo0_parity",  tinyui_demo_legacy_demo0_parity_build,  1024, 600 },
    { "legacy_widget_parity", tinyui_demo_legacy_widget_parity_build, 480,  320 },
    { "line_edit_basic",      tinyui_demo_line_edit_basic_build,      480,  320 },
    { "list_basic",           tinyui_demo_list_basic_build,           480,  320 },
    { "message_box_basic",    tinyui_demo_message_box_basic_build,    480,  320 },
    { "progress_bar_basic",   tinyui_demo_progress_bar_basic_build,   480,  320 },
    { "progress_wheel_basic", tinyui_demo_progress_wheel_basic_build, 480,  320 },
    { "qrcode_basic",         tinyui_demo_qrcode_basic_build,         480,  320 },
    { "radial_menu_basic",    tinyui_demo_radial_menu_basic_build,    480,  320 },
    { "scroll_selector_basic", tinyui_demo_scroll_selector_basic_build, 480, 320 },
    { "settings_panel",       tinyui_demo_settings_panel_build,       480,  320 },
    { "table_basic",          tinyui_demo_table_basic_build,          480,  320 },
    { "theme_showcase",       tinyui_demo_theme_showcase_build,       480,  320 },
};

#define TINYUI_DEMOS_COUNT \
    ((int)(sizeof(demos_entry_info) / sizeof(demos_entry_info[0])))

static const tinyui_demo_entry_t *tinyui_demos_find_entry(const char *name)
{
    const int count = TINYUI_DEMOS_COUNT;
    const char *resolved = (name == NULL) ? "hello_world" : name;
    int i;

    for (i = 0; i < count; i++) {
        if (strcmp(resolved, demos_entry_info[i].name) == 0) {
            return &demos_entry_info[i];
        }
    }
    return NULL;
}

tinyui_result_t tinyui_demos_build(const char *name, tinyui_obj_t *screen)
{
    const tinyui_demo_entry_t *entry;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    entry = tinyui_demos_find_entry(name);
    if (entry == NULL || entry->build == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    return entry->build(screen);
}

bool tinyui_demos_get_display_size(const char *name,
                                   uint16_t *width,
                                   uint16_t *height)
{
    const tinyui_demo_entry_t *entry;

    if (width == NULL || height == NULL) {
        return false;
    }

    entry = tinyui_demos_find_entry(name);
    if (entry == NULL || entry->width == 0U || entry->height == 0U) {
        return false;
    }

    *width = entry->width;
    *height = entry->height;
    return true;
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
