/*
 * Copyright (c) 2023-2026 flyingcys (flyingcys@gmail.com). All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "internal/window_internal.h"
#include "layout/layout.h"
#include "extensions/ldgui_native.h"

#include <limits.h>
#include <string.h>

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

static tinyui_result_t s_track_to_ld(const tinyui_grid_track_t *track, int16_t *out)
{
    if (track == 0 || out == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    switch (track->unit) {
    case TINYUI_GRID_UNIT_PX:
        if (track->value < 1 || track->value > 32767) {
            return TINYUI_ERROR_OUT_OF_RANGE;
        }
        *out = (int16_t)track->value;
        return TINYUI_OK;
    case TINYUI_GRID_UNIT_FR:
        if (track->value < 1 || track->value > 255) {
            return TINYUI_ERROR_OUT_OF_RANGE;
        }
        *out = LD_GRID_FR(track->value);
        return TINYUI_OK;
    case TINYUI_GRID_UNIT_CONTENT:
        if (track->value != 0) {
            return TINYUI_ERROR_OUT_OF_RANGE;
        }
        *out = LD_GRID_CONTENT;
        return TINYUI_OK;
    default:
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
}

/* Convert typed tracks into a local fixed buffer, then copy into the durable
 * backend array that ldWindow keeps by pointer. No second TinyUI geometry solver
 * and no public sentinel magic numbers. */
static tinyui_result_t s_convert_tracks(const tinyui_grid_track_t *tracks,
                                        uint8_t count,
                                        int16_t *durable)
{
    int16_t local[TINYUI_GRID_MAX_TRACKS];
    uint8_t i;
    tinyui_result_t result;

    if (count < 1 || count > TINYUI_GRID_MAX_TRACKS) {
        return TINYUI_ERROR_OUT_OF_RANGE;
    }
    if (tracks == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }
    if (durable == 0) {
        return TINYUI_ERROR_INVALID_ARG;
    }

    for (i = 0; i < count; ++i) {
        result = s_track_to_ld(&tracks[i], &local[i]);
        if (result != TINYUI_OK) {
            return result;
        }
    }
    for (; i < TINYUI_GRID_MAX_TRACKS; ++i) {
        local[i] = LD_GRID_TEMPLATE_LAST;
    }
    memcpy(durable, local, sizeof(local));
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
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    result = s_convert_tracks(tracks, count, window->backend_grid_cols);
    if (result != TINYUI_OK) {
        return result;
    }
    window->grid_col_count = count;
    /* Keep legacy int mirror only as opaque cache, not geometry truth. */
    {
        uint8_t i;
        for (i = 0; i < TINYUI_GRID_MAX_TRACKS; ++i) {
            window->grid_cols[i] = window->backend_grid_cols[i];
        }
    }
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
    ld_window = tinyui_window_ld_of(window);
    if (ld_window == 0) {
        return TINYUI_ERROR_INVALID_OBJECT;
    }
    result = s_convert_tracks(tracks, count, window->backend_grid_rows);
    if (result != TINYUI_OK) {
        return result;
    }
    window->grid_row_count = count;
    {
        uint8_t i;
        for (i = 0; i < TINYUI_GRID_MAX_TRACKS; ++i) {
            window->grid_rows[i] = window->backend_grid_rows[i];
        }
    }
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
