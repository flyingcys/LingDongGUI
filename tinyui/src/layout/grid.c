/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "internal/window_internal.h"
#include "layout/layout.h"

#include <limits.h>

static struct tinyui_window *s_window_of(tinyui_obj_t *container)
{
    struct tinyui_widget *widget = (struct tinyui_widget *)container;

    if (widget == 0
        || (widget->kind != TINYUI_BACKEND_WIDGET_WINDOW
            && widget->kind != TINYUI_BACKEND_WIDGET_BACKGROUND)) {
        return 0;
    }
    return (struct tinyui_window *)widget;
}

static int s_track_to_ld(const tinyui_grid_track_t *track, int16_t *out)
{
    if (track == 0 || out == 0) {
        return -1;
    }
    switch (track->unit) {
    case TINYUI_GRID_UNIT_PX:
        if (track->value < 1 || track->value > INT16_MAX) {
            return -1;
        }
        *out = (int16_t)track->value;
        return 0;
    case TINYUI_GRID_UNIT_FR:
        if (track->value < 1 || track->value > UINT8_MAX) {
            return -1;
        }
        *out = LD_GRID_FR(track->value);
        return 0;
    case TINYUI_GRID_UNIT_CONTENT:
        if (track->value != 0) {
            return -1;
        }
        *out = LD_GRID_CONTENT;
        return 0;
    default:
        return -1;
    }
}

static tinyui_result_t s_copy_tracks(int *raw,
                                     int16_t *backend,
                                     const tinyui_grid_track_t *tracks,
                                     uint8_t count)
{
    uint8_t i;

    if (count > TINYUI_GRID_MAX_TRACKS) {
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
    if (count != 0 && tracks == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    for (i = 0; i < count; ++i) {
        if (s_track_to_ld(&tracks[i], &backend[i]) != 0) {
            return TINYUI_ERROR_OUT_OF_RANGE;
        }
        raw[i] = backend[i];
    }
    for (; i < TINYUI_GRID_MAX_TRACKS; ++i) {
        raw[i] = 0;
        backend[i] = LD_GRID_TEMPLATE_LAST;
    }
    return TINYUI_OK;
}

static void s_apply_grid(struct tinyui_window *window, ldWindow_t *ld_window)
{
    ldWindowSetGridDscArray(ld_window,
                            window->grid_col_count > 0 ? window->backend_grid_cols : 0,
                            window->grid_row_count > 0 ? window->backend_grid_rows : 0);
    tinyui_window_sync_padding(window);
}

tinyui_result_t tinyui_grid_set_columns(tinyui_obj_t *container,
                                        const tinyui_grid_track_t *tracks,
                                        uint8_t count)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;
    tinyui_result_t result;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    result = s_copy_tracks(window->grid_cols,
                           window->backend_grid_cols,
                           tracks,
                           count);
    if (result != TINYUI_OK) {
        return result;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->grid_col_count = count;
    s_apply_grid(window, ld_window);
    return TINYUI_OK;
}

tinyui_result_t tinyui_grid_set_rows(tinyui_obj_t *container,
                                     const tinyui_grid_track_t *tracks,
                                     uint8_t count)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;
    tinyui_result_t result;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    result = s_copy_tracks(window->grid_rows,
                           window->backend_grid_rows,
                           tracks,
                           count);
    if (result != TINYUI_OK) {
        return result;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->grid_row_count = count;
    s_apply_grid(window, ld_window);
    return TINYUI_OK;
}

tinyui_result_t tinyui_grid_set_gap(tinyui_obj_t *container,
                                    int row_gap,
                                    int col_gap)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if (row_gap < 0 || col_gap < 0 || row_gap > INT16_MAX || col_gap > INT16_MAX) {
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->grid_row_gap = row_gap;
    window->grid_col_gap = col_gap;
    ldWindowSetGridGap(ld_window, (int16_t)row_gap, (int16_t)col_gap);
    tinyui_window_sync_padding(window);
    return TINYUI_OK;
}

tinyui_result_t tinyui_grid_set_align(tinyui_obj_t *container,
                                      tinyui_align_t col_align,
                                      tinyui_align_t row_align)
{
    struct tinyui_window *window = s_window_of(container);
    ldWindow_t *ld_window;

    if (window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    if ((int)col_align < TINYUI_ALIGN_START || (int)col_align > TINYUI_ALIGN_SPACE_BETWEEN
        || (int)row_align < TINYUI_ALIGN_START || (int)row_align > TINYUI_ALIGN_SPACE_BETWEEN) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    window->grid_col_align = (enum tinyui_align)col_align;
    window->grid_row_align = (enum tinyui_align)row_align;
    ldWindowSetGridAlign(ld_window,
                         (ldGridAlign_t)tinyui_native_align_to_ld_grid((enum tinyui_native_align)col_align),
                         (ldGridAlign_t)tinyui_native_align_to_ld_grid((enum tinyui_native_align)row_align));
    tinyui_window_sync_padding(window);
    return TINYUI_OK;
}
