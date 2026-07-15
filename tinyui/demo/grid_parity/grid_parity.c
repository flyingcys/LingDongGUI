/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "grid_parity/grid_parity.h"
#include "tinyui.h"

static tinyui_result_t style_panel(tinyui_obj_t *button,
                                   const char *text,
                                   unsigned int bg_color)
{
    if (tinyui_button_set_text(button, text) != 0
        || tinyui_obj_set_bg_color(button, bg_color) != TINYUI_OK
        || tinyui_button_set_text_color(button, 0xFFFFFFU) != 0) {
        return TINYUI_ERROR_BACKEND;
    }
    return TINYUI_OK;
}

tinyui_result_t tinyui_demo_grid_parity_build(tinyui_obj_t *screen)
{
    static const tinyui_grid_track_t root_cols[] = {
        {TINYUI_GRID_UNIT_PX, 456},
    };
    static const tinyui_grid_track_t root_rows[] = {
        {TINYUI_GRID_UNIT_PX, 20},
        {TINYUI_GRID_UNIT_PX, 22},
        {TINYUI_GRID_UNIT_PX, 204},
        {TINYUI_GRID_UNIT_CONTENT, 0},
    };
    static const tinyui_grid_track_t canvas_cols[] = {
        {TINYUI_GRID_UNIT_PX, 92},
        {TINYUI_GRID_UNIT_CONTENT, 0},
        {TINYUI_GRID_UNIT_FR, 1},
    };
    static const tinyui_grid_track_t canvas_rows[] = {
        {TINYUI_GRID_UNIT_PX, 54},
        {TINYUI_GRID_UNIT_PX, 66},
        {TINYUI_GRID_UNIT_FR, 1},
    };
    tinyui_obj_t *title;
    tinyui_obj_t *hint;
    tinyui_obj_t *canvas;
    tinyui_obj_t *guide;
    tinyui_obj_t *panel_a;
    tinyui_obj_t *panel_b;
    tinyui_obj_t *panel_c;
    tinyui_obj_t *panel_d;
    tinyui_obj_t *panel_e;
    tinyui_obj_t *panel_f;
    tinyui_obj_t *panel_g;
    tinyui_obj_t *overlay;
    tinyui_result_t result;

    if (screen == NULL) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    if (tinyui_window_set_color(screen, 0xF5F6F8U) != 0
        || tinyui_window_set_layout_type(screen, TINYUI_WINDOW_LAYOUT_GRID) != 0
        || tinyui_window_set_padding(screen, 12, 8, 12, 8) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    result = tinyui_grid_set_columns(screen, root_cols, 1);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_rows(screen, root_rows, 4);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_gap(screen, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_align(screen, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }

    title = tinyui_text_create(screen);
    hint = tinyui_text_create(screen);
    canvas = tinyui_window_create(screen);
    guide = tinyui_text_create(screen);
    if (title == NULL || hint == NULL || canvas == NULL || guide == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (tinyui_text_set_text(title, "Grid Demo") != 0
        || tinyui_text_set_text(
               hint,
               "Uses [92, content, 1fr] x [54, 66, 1fr] tracks, spans, centered cells, auto place, ignore-layout overlay.") != 0
        || tinyui_obj_set_grid_cell(title, 0, 0, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_CENTER) != TINYUI_OK
        || tinyui_obj_set_grid_cell(hint, 0, 1, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_CENTER) != TINYUI_OK
        || tinyui_obj_set_grid_cell(canvas, 0, 2, 1, 1, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_window_set_color(canvas, 0xE8ECF2U) != 0
        || tinyui_window_set_layout_type(canvas, TINYUI_WINDOW_LAYOUT_GRID) != 0
        || tinyui_window_set_padding(canvas, 12, 12, 12, 12) != 0) {
        return TINYUI_ERROR_BACKEND;
    }

    result = tinyui_grid_set_columns(canvas, canvas_cols, 3);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_rows(canvas, canvas_rows, 3);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_gap(canvas, 12, 12);
    if (result != TINYUI_OK) {
        return result;
    }
    result = tinyui_grid_set_align(canvas, TINYUI_ALIGN_START, TINYUI_ALIGN_START);
    if (result != TINYUI_OK) {
        return result;
    }

    if (tinyui_text_set_text(
            guide,
            "Grid descriptors: [92, content, 1fr] x [54, 66, 1fr], spans, centered cells, auto place, ignore-layout overlay.") != 0
        || tinyui_obj_set_grid_cell(guide, 0, 3, 1, 1, TINYUI_ALIGN_START, TINYUI_ALIGN_CENTER) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    panel_a = tinyui_button_create(canvas);
    panel_b = tinyui_button_create(canvas);
    panel_c = tinyui_button_create(canvas);
    panel_d = tinyui_button_create(canvas);
    panel_e = tinyui_button_create(canvas);
    panel_f = tinyui_button_create(canvas);
    panel_g = tinyui_button_create(canvas);
    overlay = tinyui_text_create(canvas);
    if (panel_a == NULL || panel_b == NULL || panel_c == NULL || panel_d == NULL
        || panel_e == NULL || panel_f == NULL || panel_g == NULL || overlay == NULL) {
        return TINYUI_ERROR_NO_MEMORY;
    }

    if (style_panel(panel_a, "A", 0xE76F51U) != TINYUI_OK
        || style_panel(panel_b, "B", 0x2A9D8FU) != TINYUI_OK
        || style_panel(panel_c, "C", 0x457B9DU) != TINYUI_OK
        || style_panel(panel_d, "D", 0x264653U) != TINYUI_OK
        || style_panel(panel_e, "E", 0xF4A261U) != TINYUI_OK
        || style_panel(panel_f, "F", 0x6D597AU) != TINYUI_OK
        || style_panel(panel_g, "G", 0x8AB17DU) != TINYUI_OK
        || tinyui_obj_set_size(panel_a, 92, 54) != TINYUI_OK
        || tinyui_obj_set_size(panel_b, 112, 54) != TINYUI_OK
        || tinyui_obj_set_size(panel_c, 124, 54) != TINYUI_OK
        || tinyui_obj_set_size(panel_d, 180, 66) != TINYUI_OK
        || tinyui_obj_set_size(panel_e, 124, 132) != TINYUI_OK
        || tinyui_obj_set_size(panel_f, 56, 28) != TINYUI_OK
        || tinyui_obj_set_size(panel_g, 92, 54) != TINYUI_OK
        || tinyui_obj_set_grid_cell(panel_a, 0, 0, 1, 1, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_obj_set_grid_cell(panel_b, 1, 0, 1, 1, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_obj_set_grid_cell(panel_c, 2, 0, 1, 1, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_obj_set_grid_cell(panel_d, 0, 1, 2, 1, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_obj_set_grid_cell(panel_e, 2, 1, 1, 2, TINYUI_ALIGN_STRETCH, TINYUI_ALIGN_STRETCH) != TINYUI_OK
        || tinyui_obj_set_grid_cell(panel_f, 1, 2, 1, 1, TINYUI_ALIGN_CENTER, TINYUI_ALIGN_CENTER) != TINYUI_OK
        || tinyui_text_set_text(overlay, "Overlay\nignore-layout") != 0
        || tinyui_text_set_bg_color(overlay, 0x1D3557U) != 0
        || tinyui_text_set_text_color(overlay, 0xFFFFFFU) != 0
        || tinyui_obj_set_size(overlay, 116, 40) != TINYUI_OK
        || tinyui_obj_set_ignore_layout(overlay, 1) != TINYUI_OK
        || tinyui_obj_set_pos(overlay, 320, 148) != TINYUI_OK) {
        return TINYUI_ERROR_BACKEND;
    }

    (void)panel_g;
    return TINYUI_OK;
}
