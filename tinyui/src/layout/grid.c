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

#include "internal/window_internal.h"
#include "layout.h"
#include <string.h>

/* ── grid track / align mappings ────────────────────────────────────────────
 * grid-private mappings, kept next to the grid setters. */
static ldGridAlign_t s_grid_align_to_ld(enum tinyui_align align)
{
    return (ldGridAlign_t)tinyui_native_align_to_ld_grid((enum tinyui_native_align)align);
}

static int16_t s_grid_track_to_ld(int value)
{
    if (value == 0) {
        return LD_GRID_TEMPLATE_LAST;
    }
    if (value == -2) {
        return LD_GRID_CONTENT;
    }
    if (value < 0) {
        return LD_GRID_FR((-value) - 1);
    }
    return (int16_t)value;
}

static int s_copy_grid_tracks(int *dst, int16_t *backend_dst, const int *src, int count)
{
    int i;

    if (dst == 0 || backend_dst == 0 || src == 0 || count <= 0 || count > TINYUI_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
        backend_dst[i] = s_grid_track_to_ld(src[i]);
    }
    for (; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
        dst[i] = 0;
        backend_dst[i] = LD_GRID_TEMPLATE_LAST;
    }
    return 0;
}

/**
 * @brief Set columns of grid widget
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return -1 on failure
 */
int tinyui_grid_set_columns(struct tinyui_window *window, const int *tracks, int count)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);
    int window_tracks[TINYUI_LAYOUT_MAX_TRACKS];
    int i;

    if (window == 0 || ld_window == 0
        || s_copy_grid_tracks(window_tracks, window->backend_grid_cols, tracks, count) != 0) {
        return -1;
    }

    memcpy(window->grid_cols, window_tracks, sizeof(window->grid_cols));
    window->grid_col_count = count;

    {
        for (i = 0; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
            window->backend_grid_rows[i] = s_grid_track_to_ld(window->grid_rows[i]);
        }
        ldWindowSetGridDscArray(ld_window,
                                window->backend_grid_cols,
                                window->grid_row_count > 0 ? window->backend_grid_rows : NULL);
    }
    tinyui_window_sync_padding(window);
    return 0;
}

/**
 * @brief Set rows of grid widget
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return -1 on failure
 */
int tinyui_grid_set_rows(struct tinyui_window *window, const int *tracks, int count)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);
    int window_tracks[TINYUI_LAYOUT_MAX_TRACKS];
    int i;

    if (window == 0 || ld_window == 0
        || s_copy_grid_tracks(window_tracks, window->backend_grid_rows, tracks, count) != 0) {
        return -1;
    }

    memcpy(window->grid_rows, window_tracks, sizeof(window->grid_rows));
    window->grid_row_count = count;

    {
        for (i = 0; i < TINYUI_LAYOUT_MAX_TRACKS; ++i) {
            window->backend_grid_cols[i] = s_grid_track_to_ld(window->grid_cols[i]);
        }
        ldWindowSetGridDscArray(ld_window,
                                window->grid_col_count > 0 ? window->backend_grid_cols : NULL,
                                window->backend_grid_rows);
    }
    tinyui_window_sync_padding(window);
    return 0;
}

/**
 * @brief Set gap of grid widget
 *
 * @param[in] window Window instance
 * @param[in] row_gap row gap
 * @param[in] col_gap col gap
 * @return -1 on failure
 */
int tinyui_grid_set_gap(struct tinyui_window *window, int row_gap, int col_gap)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0 || row_gap < 0 || col_gap < 0) {
        return -1;
    }

    window->grid_row_gap = row_gap;
    window->grid_col_gap = col_gap;
    ldWindowSetGridGap(ld_window, (int16_t)row_gap, (int16_t)col_gap);
    tinyui_window_sync_padding(window);
    return 0;
}

/**
 * @brief Set align of grid widget
 *
 * @param[in] window Window instance
 * @param[in] col_align col align
 * @param[in] row_align row align
 * @return -1 on failure
 */
int tinyui_grid_set_align(struct tinyui_window *window,
                          enum tinyui_align col_align,
                          enum tinyui_align row_align)
{
    ldWindow_t *ld_window = tinyui_window_ld_of(window);

    if (window == 0 || ld_window == 0) {
        return -1;
    }

    window->grid_col_align = col_align;
    window->grid_row_align = row_align;
    ldWindowSetGridAlign(ld_window,
                         s_grid_align_to_ld(col_align),
                         s_grid_align_to_ld(row_align));
    tinyui_window_sync_padding(window);
    return 0;
}
