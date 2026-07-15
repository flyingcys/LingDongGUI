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

#include <string.h>

tinyui_result_t tinyui_demo_graph_basic_build(tinyui_obj_t *screen)
{
    tinyui_obj_t *title;
    tinyui_obj_t *graph;
    tinyui_graph_props_t graph_props;
    int cpu_series;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    result = tinyui_flex_set_flow(screen, TINYUI_FLEX_FLOW_COLUMN);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_align(screen,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START,
                                   TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_flex_set_gap(screen, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }
    if (tinyui_window_set_padding(screen, 24, 24, 24, 24) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    memset(&graph_props, 0, sizeof(graph_props));
    graph_props.fields = TINYUI_GRAPH_FIELD_SERIES_MAX | TINYUI_GRAPH_FIELD_WIDTH
        | TINYUI_GRAPH_FIELD_HEIGHT;
    graph_props.series_max = 1;
    graph_props.width = 180;
    graph_props.height = 96;

    title = tinyui_label_create(screen);
    graph = tinyui_graph_create_with_props(screen, &graph_props);
    if (title == NULL || graph == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_label_set_text(title, "Graph") != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    cpu_series = tinyui_graph_add_series(graph, 0x2057C4U, 1, 3);
    if (cpu_series < 0) {
        return TINYUI_ERROR_BACKEND;
    }
    if (tinyui_graph_set_value(graph, cpu_series, 0, 12) != 0
        || tinyui_graph_set_value(graph, cpu_series, 1, 26) != 0
        || tinyui_graph_set_value(graph, cpu_series, 2, 42) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    return TINYUI_OK;
}
