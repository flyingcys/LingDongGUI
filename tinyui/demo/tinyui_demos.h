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
 * TinyUI demo aggregator — mirrors lvgl/demos/lv_demos.h.
 *
 * All demos are compiled into a single tinyui_demo binary.
 * Select a demo at runtime:
 *
 *   ./tinyui_demo arc_basic
 *
 * Each demo follows the same convention as lv_demo_widgets():
 *
 *   void tinyui_demo_<name>(void);
 *
 * The function creates its own screen, builds the UI, and loads the
 * screen — no arguments needed at the call site.
 */

#ifndef TINYUI_DEMOS_H
#define TINYUI_DEMOS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tinyui_display_config;

/* ------------------------------------------------------------------ */
/* All demo entry points (alphabetical)                                */
/* ------------------------------------------------------------------ */

void tinyui_demo_animation_basic(void);
void tinyui_demo_arc_basic(void);
void tinyui_demo_basic_widgets(void);
void tinyui_demo_calendar_basic(void);
void tinyui_demo_clock_basic(void);
void tinyui_demo_combo_box_basic(void);
void tinyui_demo_date_time_basic(void);
void tinyui_demo_gauge_basic(void);
void tinyui_demo_graph_basic(void);
void tinyui_demo_grid_parity(void);
void tinyui_demo_hello_world(void);
void tinyui_demo_icon_slider_basic(void);
void tinyui_demo_keyboard_basic(void);
void tinyui_demo_layout_flex(void);
void tinyui_demo_layout_grid(void);
void tinyui_demo_layout_parity(void);
void tinyui_demo_legacy_demo0_parity(void);
void tinyui_demo_legacy_demo0_parity_frame(unsigned int elapsed_ms);
void tinyui_demo_legacy_widget_parity(void);
void tinyui_demo_line_edit_basic(void);
void tinyui_demo_list_basic(void);
void tinyui_demo_message_box_basic(void);
void tinyui_demo_progress_bar_basic(void);
void tinyui_demo_progress_wheel_basic(void);
void tinyui_demo_qrcode_basic(void);
void tinyui_demo_radial_menu_basic(void);
void tinyui_demo_scroll_selecter_basic(void);
void tinyui_demo_settings_panel(void);
void tinyui_demo_table_basic(void);
void tinyui_demo_theme_showcase(void);

/* ------------------------------------------------------------------ */
/* Runtime dispatch — mirrors lv_demos_create / lv_demos_show_help    */
/* ------------------------------------------------------------------ */

/*
 * Select and run a demo by name.
 *   info  : argv+1 from main (array of argument strings)
 *   size  : argc-1 (number of arguments)
 * size <= 0: run the first demo (hello_world).
 * Returns true if the demo was found and launched, false otherwise.
 */
typedef void (*tinyui_demo_frame_cb_t)(unsigned int elapsed_ms);

bool tinyui_demos_create(char *info[], int size);

/* Resolve the selected demo's runner display size. */
bool tinyui_demos_get_display_config(char *info[],
                                     int size,
                                     struct tinyui_display_config *out_config);

/* Dispatch the selected demo's optional per-frame callback. */
void tinyui_demos_frame(unsigned int elapsed_ms);

/* Print the list of available demo names to stdout */
void tinyui_demos_show_help(void);

#ifdef __cplusplus
}
#endif

#endif /* TINYUI_DEMOS_H */
