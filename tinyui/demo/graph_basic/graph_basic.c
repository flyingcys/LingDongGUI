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

#include "graph_basic/graph_basic.h"
#include "tinyui.h"

static void make_ui(struct tinyui_window *win)
{
    struct tinyui_label *title;
    struct tinyui_graph *graph;
    int cpu_series;
    static const int cols[] = {280, 0};
    static const int rows[] = {28, 120, 0};

    tinyui_grid_set_columns(win, cols, 2);
    tinyui_grid_set_rows(win, rows, 3);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    tinyui_window_set_padding_group(win, 24, 24, 24, 24);

    title = tinyui_label_create(win, "title");
    graph = tinyui_graph_create_with_props(
        win,
        &(struct tinyui_graph_props){
            .id = "graph",
            .series_max = 1,
            .width = 180,
            .height = 96,
        });

    if (title != 0) {
        tinyui_label_set_text(title, "Graph");
        tinyui_widget_set_size((struct tinyui_widget *)title, 220, 28);
        tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                    0, 0, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }

    if (graph != 0) {
        cpu_series = tinyui_graph_add_series(graph, 0x2057C4U, 1, 3);
        if (cpu_series >= 0) {
            tinyui_graph_set_value(graph, cpu_series, 0, 12);
            tinyui_graph_set_value(graph, cpu_series, 1, 26);
            tinyui_graph_set_value(graph, cpu_series, 2, 42);
        }
        tinyui_widget_set_grid_cell((struct tinyui_widget *)graph,
                                    0, 1, 1, 1,
                                    TINYUI_ALIGN_START,
                                    TINYUI_ALIGN_CENTER);
    }
}

void tinyui_demo_graph_basic(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    if (win == 0) {
        return;
    }

    make_ui(win);
    tinyui_screen_load(screen);
}
