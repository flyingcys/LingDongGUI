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
 * TinyUI demo aggregator — unified build(screen) registry (M4 Task 2).
 *
 * All demos are compiled into a single tinyui_demo binary.
 * Select a demo at runtime:
 *
 *   ./tinyui_demo arc_basic
 *
 * Each demo exposes:
 *
 *   tinyui_result_t tinyui_demo_<name>_build(tinyui_obj_t *screen);
 *
 * The runner owns runtime lifecycle (init / screen create+load / process /
 * deinit). Builders only express UI intent on the provided screen.
 */

#ifndef TINYUI_DEMOS_H
#define TINYUI_DEMOS_H

#include "tinyui.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef tinyui_result_t (*tinyui_demo_build_cb_t)(tinyui_obj_t *screen);

/* ------------------------------------------------------------------ */
/* All demo builders (alphabetical; match tinyui_demo_manifest.json)  */
/* ------------------------------------------------------------------ */

tinyui_result_t tinyui_demo_animation_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_arc_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_basic_widgets_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_calendar_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_clock_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_combo_box_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_date_time_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_gauge_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_graph_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_grid_parity_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_hello_world_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_icon_slider_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_keyboard_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_layout_flex_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_layout_grid_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_layout_parity_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_legacy_demo0_parity_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_legacy_widget_parity_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_line_edit_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_list_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_message_box_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_progress_bar_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_progress_wheel_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_qrcode_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_radial_menu_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_scroll_selector_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_settings_panel_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_table_basic_build(tinyui_obj_t *screen);
tinyui_result_t tinyui_demo_theme_showcase_build(tinyui_obj_t *screen);

/* ------------------------------------------------------------------ */
/* Runtime dispatch                                                    */
/* ------------------------------------------------------------------ */

/*
 * Build the named demo onto an existing screen.
 * name == NULL selects hello_world.
 * Returns TINYUI_ERROR_INVALID_ARG for unknown name or NULL screen;
 * otherwise propagates the builder result (never swallows backend errors).
 */
tinyui_result_t tinyui_demos_build(const char *name, tinyui_obj_t *screen);

/* Resolve the selected demo's runner display size. name == NULL -> hello_world. */
bool tinyui_demos_get_display_size(const char *name,
                                   uint16_t *width,
                                   uint16_t *height);

/* Print the list of available demo names to stdout */
void tinyui_demos_show_help(void);

#ifdef __cplusplus
}
#endif

#endif /* TINYUI_DEMOS_H */
