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

#include "tinyui.h"

static void style_panel(struct picoui_button *button,
                        const char *text,
                        unsigned int bg_color)
{
    picoui_button_set_text(button, text);
    picoui_widget_set_bg_color((struct picoui_widget *)button, bg_color);
    picoui_button_set_text_color(button, 0xFFFFFFU);
    picoui_widget_set_radius((struct picoui_widget *)button, 8);
}

static void make_ui(struct picoui_window *win)
{
    static const int root_cols[] = {456, 0};
    static const int root_rows[] = {20, 22, 204, 16, 0};
    static const int canvas_cols[] = {92, -2, -3, 0};
    static const int canvas_rows[] = {54, 66, -3, 0};
    struct picoui_text *title;
    struct picoui_text *hint;
    struct picoui_window *canvas;
    struct picoui_text *guide;
    struct picoui_button *panel_a;
    struct picoui_button *panel_b;
    struct picoui_button *panel_c;
    struct picoui_button *panel_d;
    struct picoui_button *panel_e;
    struct picoui_button *panel_f;
    struct picoui_button *panel_g;
    struct picoui_text *overlay;

    picoui_window_set_color(win, 0xF5F6F8U);
    picoui_window_set_layout_type(win, PICOUI_WINDOW_LAYOUT_GRID);
    picoui_window_set_padding_group(win, 12, 8, 12, 8);
    picoui_grid_set_columns(win, root_cols, 2);
    picoui_grid_set_rows(win, root_rows, 5);
    picoui_grid_set_gap(win, 12, 12);
    picoui_grid_set_align(win, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    title = picoui_text_create(win, "grid_title");
    hint = picoui_text_create(win, "grid_hint");
    canvas = picoui_window_create_child(win, "grid_canvas");
    guide = picoui_text_create(win, "grid_guide");
    panel_a = picoui_button_create(canvas, "panel_a");
    panel_b = picoui_button_create(canvas, "panel_b");
    panel_c = picoui_button_create(canvas, "panel_c");
    panel_d = picoui_button_create(canvas, "panel_d");
    panel_e = picoui_button_create(canvas, "panel_e");
    panel_f = picoui_button_create(canvas, "panel_f");
    panel_g = picoui_button_create(canvas, "panel_g");
    overlay = picoui_text_create(canvas, "grid_overlay");

    picoui_text_set_text(title, "Grid Demo");
    picoui_text_set_text(hint, "Uses [92, content, 1fr] x [54, 66, 1fr] tracks, explicit spans, centered cells, auto placement, and ignore-layout overlay.");
    picoui_widget_set_grid_cell((struct picoui_widget *)title,
                                0, 0, 1, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);
    picoui_widget_set_grid_cell((struct picoui_widget *)hint,
                                0, 1, 1, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);

    picoui_widget_set_grid_cell((struct picoui_widget *)canvas,
                                0, 2, 1, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_window_set_color(canvas, 0xE8ECF2U);
    picoui_window_set_layout_type(canvas, PICOUI_WINDOW_LAYOUT_GRID);
    picoui_window_set_padding_group(canvas, 12, 12, 12, 12);
    picoui_grid_set_columns(canvas, canvas_cols, 4);
    picoui_grid_set_rows(canvas, canvas_rows, 4);
    picoui_grid_set_gap(canvas, 12, 12);
    picoui_grid_set_align(canvas, PICOUI_ALIGN_START, PICOUI_ALIGN_START);

    picoui_text_set_text(guide, "Grid descriptors: [92, content, 1fr] x [54, 66, 1fr], plus explicit spans, centered cells, auto placement, and one ignore-layout overlay.");
    picoui_widget_set_grid_cell((struct picoui_widget *)guide,
                                0, 3, 1, 1,
                                PICOUI_ALIGN_START,
                                PICOUI_ALIGN_CENTER);

    style_panel(panel_a, "A", 0xE76F51U);
    style_panel(panel_b, "B", 0x2A9D8FU);
    style_panel(panel_c, "C", 0x457B9DU);
    style_panel(panel_d, "D", 0x264653U);
    style_panel(panel_e, "E", 0xF4A261U);
    style_panel(panel_f, "F", 0x6D597AU);
    style_panel(panel_g, "G", 0x8AB17DU);

    picoui_widget_set_size((struct picoui_widget *)panel_a, 92, 54);
    picoui_widget_set_size((struct picoui_widget *)panel_b, 112, 54);
    picoui_widget_set_size((struct picoui_widget *)panel_c, 124, 54);
    picoui_widget_set_size((struct picoui_widget *)panel_d, 180, 66);
    picoui_widget_set_size((struct picoui_widget *)panel_e, 124, 132);
    picoui_widget_set_size((struct picoui_widget *)panel_f, 56, 28);
    picoui_widget_set_size((struct picoui_widget *)panel_g, 92, 54);

    picoui_widget_set_grid_cell((struct picoui_widget *)panel_a,
                                0, 0, 1, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_widget_set_grid_cell((struct picoui_widget *)panel_b,
                                1, 0, 1, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_widget_set_grid_cell((struct picoui_widget *)panel_c,
                                2, 0, 1, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_widget_set_grid_cell((struct picoui_widget *)panel_d,
                                0, 1, 2, 1,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_widget_set_grid_cell((struct picoui_widget *)panel_e,
                                2, 1, 1, 2,
                                PICOUI_ALIGN_STRETCH,
                                PICOUI_ALIGN_STRETCH);
    picoui_widget_set_grid_cell((struct picoui_widget *)panel_f,
                                1, 2, 1, 1,
                                PICOUI_ALIGN_CENTER,
                                PICOUI_ALIGN_CENTER);

    picoui_text_set_text(overlay, "Overlay\nignore-layout");
    picoui_text_set_bg_color(overlay, 0x1D3557U);
    picoui_text_set_text_color(overlay, 0xFFFFFFU);
    picoui_widget_set_size((struct picoui_widget *)overlay, 116, 40);
    picoui_widget_set_ignore_layout((struct picoui_widget *)overlay, 1);
    picoui_widget_set_pos((struct picoui_widget *)overlay, 320, 148);
    picoui_widget_set_radius((struct picoui_widget *)overlay, 8);
}

static int run_demo(void)
{
    struct picoui_app *app = picoui_app_create();
    struct picoui_window *win;

    if (app == 0) {
        return 1;
    }

    win = picoui_window_create(app, "root");
    if (win == 0) {
        picoui_app_destroy(app);
        return 1;
    }

    make_ui(win);
    if (picoui_app_run(app, win) != 0) {
        picoui_app_destroy(app);
        return 1;
    }

    picoui_app_destroy(app);
    return 0;
}

int main(void)
{
    return run_demo();
}
