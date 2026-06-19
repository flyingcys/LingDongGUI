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

#include "grid_parity/grid_parity.h"
#include "tinyui.h"

static void style_panel(struct tinyui_button *button,
                        const char *text,
                        unsigned int bg_color)
{
    tinyui_button_set_text(button, text);
    tinyui_widget_set_bg_color((struct tinyui_widget *)button, bg_color);
    tinyui_button_set_text_color(button, 0xFFFFFFU);
    tinyui_widget_set_radius((struct tinyui_widget *)button, 8);
}

static void make_ui(struct tinyui_window *win)
{
    static const int root_cols[] = {456, 0};
    static const int root_rows[] = {20, 22, 204, 16, 0};
    static const int canvas_cols[] = {92, -2, -3, 0};
    static const int canvas_rows[] = {54, 66, -3, 0};
    struct tinyui_text *title;
    struct tinyui_text *hint;
    struct tinyui_window *canvas;
    struct tinyui_text *guide;
    struct tinyui_button *panel_a;
    struct tinyui_button *panel_b;
    struct tinyui_button *panel_c;
    struct tinyui_button *panel_d;
    struct tinyui_button *panel_e;
    struct tinyui_button *panel_f;
    struct tinyui_button *panel_g;
    struct tinyui_text *overlay;

    tinyui_window_set_color(win, 0xF5F6F8U);
    tinyui_window_set_layout_type(win, TINYUI_WINDOW_LAYOUT_GRID);
    tinyui_window_set_padding(win, 12, 8, 12, 8);
    tinyui_grid_set_columns(win, root_cols, 2);
    tinyui_grid_set_rows(win, root_rows, 5);
    tinyui_grid_set_gap(win, 12, 12);
    tinyui_grid_set_align(win, TINYUI_ALIGN_START, TINYUI_ALIGN_START);

    title = tinyui_text_create(win, "grid_title");
    hint = tinyui_text_create(win, "grid_hint");
    canvas = tinyui_window_create_child(win, "grid_canvas");
    guide = tinyui_text_create(win, "grid_guide");
    panel_a = tinyui_button_create(canvas, "panel_a");
    panel_b = tinyui_button_create(canvas, "panel_b");
    panel_c = tinyui_button_create(canvas, "panel_c");
    panel_d = tinyui_button_create(canvas, "panel_d");
    panel_e = tinyui_button_create(canvas, "panel_e");
    panel_f = tinyui_button_create(canvas, "panel_f");
    panel_g = tinyui_button_create(canvas, "panel_g");
    overlay = tinyui_text_create(canvas, "grid_overlay");

    tinyui_text_set_text(title, "Grid Demo");
    tinyui_text_set_text(hint, "Uses [92, content, 1fr] x [54, 66, 1fr] tracks, explicit spans, centered cells, auto placement, and ignore-layout overlay.");
    tinyui_widget_set_grid_cell((struct tinyui_widget *)title,
                                0, 0, 1, 1,
                                TINYUI_ALIGN_START,
                                TINYUI_ALIGN_CENTER);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)hint,
                                0, 1, 1, 1,
                                TINYUI_ALIGN_START,
                                TINYUI_ALIGN_CENTER);

    tinyui_widget_set_grid_cell((struct tinyui_widget *)canvas,
                                0, 2, 1, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_STRETCH);
    tinyui_window_set_color(canvas, 0xE8ECF2U);
    tinyui_window_set_layout_type(canvas, TINYUI_WINDOW_LAYOUT_GRID);
    tinyui_window_set_padding(canvas, 12, 12, 12, 12);
    tinyui_grid_set_columns(canvas, canvas_cols, 4);
    tinyui_grid_set_rows(canvas, canvas_rows, 4);
    tinyui_grid_set_gap(canvas, 12, 12);
    tinyui_grid_set_align(canvas, TINYUI_ALIGN_START, TINYUI_ALIGN_START);

    tinyui_text_set_text(guide, "Grid descriptors: [92, content, 1fr] x [54, 66, 1fr], plus explicit spans, centered cells, auto placement, and one ignore-layout overlay.");
    tinyui_widget_set_grid_cell((struct tinyui_widget *)guide,
                                0, 3, 1, 1,
                                TINYUI_ALIGN_START,
                                TINYUI_ALIGN_CENTER);

    style_panel(panel_a, "A", 0xE76F51U);
    style_panel(panel_b, "B", 0x2A9D8FU);
    style_panel(panel_c, "C", 0x457B9DU);
    style_panel(panel_d, "D", 0x264653U);
    style_panel(panel_e, "E", 0xF4A261U);
    style_panel(panel_f, "F", 0x6D597AU);
    style_panel(panel_g, "G", 0x8AB17DU);

    tinyui_widget_set_size((struct tinyui_widget *)panel_a, 92, 54);
    tinyui_widget_set_size((struct tinyui_widget *)panel_b, 112, 54);
    tinyui_widget_set_size((struct tinyui_widget *)panel_c, 124, 54);
    tinyui_widget_set_size((struct tinyui_widget *)panel_d, 180, 66);
    tinyui_widget_set_size((struct tinyui_widget *)panel_e, 124, 132);
    tinyui_widget_set_size((struct tinyui_widget *)panel_f, 56, 28);
    tinyui_widget_set_size((struct tinyui_widget *)panel_g, 92, 54);

    tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_a,
                                0, 0, 1, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_STRETCH);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_b,
                                1, 0, 1, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_STRETCH);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_c,
                                2, 0, 1, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_STRETCH);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_d,
                                0, 1, 2, 1,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_STRETCH);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_e,
                                2, 1, 1, 2,
                                TINYUI_ALIGN_STRETCH,
                                TINYUI_ALIGN_STRETCH);
    tinyui_widget_set_grid_cell((struct tinyui_widget *)panel_f,
                                1, 2, 1, 1,
                                TINYUI_ALIGN_CENTER,
                                TINYUI_ALIGN_CENTER);

    tinyui_text_set_text(overlay, "Overlay\nignore-layout");
    tinyui_text_set_bg_color(overlay, 0x1D3557U);
    tinyui_text_set_text_color(overlay, 0xFFFFFFU);
    tinyui_widget_set_size((struct tinyui_widget *)overlay, 116, 40);
    tinyui_widget_set_ignore_layout((struct tinyui_widget *)overlay, 1);
    tinyui_widget_set_pos((struct tinyui_widget *)overlay, 320, 148);
    tinyui_widget_set_radius((struct tinyui_widget *)overlay, 8);
}

void tinyui_demo_grid_parity(void)
{
    tinyui_obj_t *screen = tinyui_screen_create();
    struct tinyui_window *win = (struct tinyui_window *)screen;

    if (win == 0) {
        return;
    }

    make_ui(win);
    tinyui_screen_load(screen);
}
