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

#include "internal.h"
#include "ldBase.h"
#include "ldWindow.h"

static struct picoui_backend_widget *picoui_backend_window_get(struct picoui_window *window)
{
    if (window == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)window->widget.backend_widget;
}

static struct picoui_backend_widget *picoui_backend_widget_get(struct picoui_widget *widget)
{
    if (widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)widget->backend_widget;
}

static ldFlexFlow_t picoui_backend_map_flex_flow(enum picoui_flex_flow flow)
{
    switch (flow) {
    case PICOUI_FLEX_FLOW_COLUMN:
        return ldFlexFlowColumn;
    case PICOUI_FLEX_FLOW_ROW_WRAP:
        return ldFlexFlowRowWrap;
    case PICOUI_FLEX_FLOW_COLUMN_WRAP:
        return ldFlexFlowColumnWrap;
    case PICOUI_FLEX_FLOW_ROW_REVERSE:
        return ldFlexFlowRowReverse;
    case PICOUI_FLEX_FLOW_COLUMN_REVERSE:
        return ldFlexFlowColumnReverse;
    case PICOUI_FLEX_FLOW_ROW_WRAP_REVERSE:
        return ldFlexFlowRowWrapReverse;
    case PICOUI_FLEX_FLOW_COLUMN_WRAP_REVERSE:
        return ldFlexFlowColumnWrapReverse;
    case PICOUI_FLEX_FLOW_ROW:
    default:
        return ldFlexFlowRow;
    }
}

static ldFlexMainAlign_t picoui_backend_map_flex_main_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ldFlexMainAlignCenter;
    case PICOUI_ALIGN_END:
        return ldFlexMainAlignEnd;
    case PICOUI_ALIGN_SPACE_EVENLY:
        return ldFlexMainAlignSpaceEvenly;
    case PICOUI_ALIGN_SPACE_AROUND:
        return ldFlexMainAlignSpaceAround;
    case PICOUI_ALIGN_SPACE_BETWEEN:
        return ldFlexMainAlignSpaceBetween;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_START:
    default:
        return ldFlexMainAlignStart;
    }
}

static ldFlexCrossAlign_t picoui_backend_map_flex_cross_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ldFlexCrossAlignCenter;
    case PICOUI_ALIGN_END:
        return ldFlexCrossAlignEnd;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_SPACE_EVENLY:
    case PICOUI_ALIGN_SPACE_AROUND:
    case PICOUI_ALIGN_SPACE_BETWEEN:
    case PICOUI_ALIGN_START:
    default:
        return ldFlexCrossAlignStart;
    }
}

static ldFlexTrackAlign_t picoui_backend_map_flex_track_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_CENTER:
        return ldFlexTrackAlignCenter;
    case PICOUI_ALIGN_END:
        return ldFlexTrackAlignEnd;
    case PICOUI_ALIGN_SPACE_BETWEEN:
        return ldFlexTrackAlignSpaceBetween;
    case PICOUI_ALIGN_SPACE_AROUND:
        return ldFlexTrackAlignSpaceAround;
    case PICOUI_ALIGN_SPACE_EVENLY:
        return ldFlexTrackAlignSpaceEvenly;
    case PICOUI_ALIGN_STRETCH:
    case PICOUI_ALIGN_START:
    default:
        return ldFlexTrackAlignStart;
    }
}

/**
 * @brief Native platform: align to ld grid
 *
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_native_align_to_ld_grid(enum picoui_native_align align)
{
    switch (align) {
    case PICOUI_NATIVE_ALIGN_END:
        return ldGridAlignEnd;
    case PICOUI_NATIVE_ALIGN_CENTER:
        return ldGridAlignCenter;
    case PICOUI_NATIVE_ALIGN_STRETCH:
        return ldGridAlignStretch;
    case PICOUI_NATIVE_ALIGN_SPACE_EVENLY:
        return ldGridAlignSpaceEvenly;
    case PICOUI_NATIVE_ALIGN_SPACE_AROUND:
        return ldGridAlignSpaceAround;
    case PICOUI_NATIVE_ALIGN_SPACE_BETWEEN:
        return ldGridAlignSpaceBetween;
    case PICOUI_NATIVE_ALIGN_START:
    default:
        return ldGridAlignStart;
    }
}

static ldGridAlign_t picoui_backend_map_grid_align(enum picoui_align align)
{
    return (ldGridAlign_t)picoui_native_align_to_ld_grid((enum picoui_native_align)align);
}

static int16_t picoui_backend_map_grid_track(int value)
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

static int picoui_backend_expected_ld_widget_type(enum picoui_backend_widget_kind kind)
{
    switch (kind) {
    case PICOUI_BACKEND_WIDGET_BACKGROUND:
        return widgetTypeBackground;
    case PICOUI_BACKEND_WIDGET_WINDOW:
        return widgetTypeWindow;
    case PICOUI_BACKEND_WIDGET_LABEL:
        return widgetTypeLabel;
    case PICOUI_BACKEND_WIDGET_BUTTON:
        return widgetTypeButton;
    case PICOUI_BACKEND_WIDGET_CHECKBOX:
        return widgetTypeCheckBox;
    case PICOUI_BACKEND_WIDGET_SWITCH:
        return widgetTypeSwitch;
    case PICOUI_BACKEND_WIDGET_SLIDER:
        return widgetTypeSlider;
    case PICOUI_BACKEND_WIDGET_ARC:
        return widgetTypeArc;
    case PICOUI_BACKEND_WIDGET_GAUGE:
        return widgetTypeGauge;
    case PICOUI_BACKEND_WIDGET_ICON_SLIDER:
        return widgetTypeIconSlider;
    case PICOUI_BACKEND_WIDGET_RADIAL_MENU:
        return widgetTypeRadialMenu;
    case PICOUI_BACKEND_WIDGET_PROGRESS_BAR:
        return widgetTypeProgressBar;
    case PICOUI_BACKEND_WIDGET_QRCODE:
        return widgetTypeQRCode;
    case PICOUI_BACKEND_WIDGET_PROGRESS_WHEEL:
        return widgetTypeProgressWheel;
    case PICOUI_BACKEND_WIDGET_ANIMATION:
        return widgetTypeAnimation;
    case PICOUI_BACKEND_WIDGET_LIST:
        return widgetTypeList;
    case PICOUI_BACKEND_WIDGET_MESSAGE_BOX:
        return widgetTypeMessageBox;
    case PICOUI_BACKEND_WIDGET_DATE_TIME:
        return widgetTypeDateTime;
    case PICOUI_BACKEND_WIDGET_TEXT:
        return widgetTypeText;
    case PICOUI_BACKEND_WIDGET_KEYBOARD:
        return widgetTypeKeyboard;
    case PICOUI_BACKEND_WIDGET_COMBO_BOX:
        return widgetTypeComboBox;
    case PICOUI_BACKEND_WIDGET_SCROLL_SELECTER:
        return widgetTypeScrollSelecter;
    case PICOUI_BACKEND_WIDGET_TABLE:
        return widgetTypeTable;
    case PICOUI_BACKEND_WIDGET_GRAPH:
        return widgetTypeGraph;
    case PICOUI_BACKEND_WIDGET_IMAGE:
        return widgetTypeImage;
    case PICOUI_BACKEND_WIDGET_CALENDAR:
        return widgetTypeCalendar;
    case PICOUI_BACKEND_WIDGET_CANVAS:
        return widgetTypeCanvas;
    default:
        return -1;
    }
}

static int picoui_backend_copy_tracks(int16_t *dst, const int *src, int count)
{
    int i;

    if (dst == 0 || src == 0 || count <= 0 || count > PICOUI_BACKEND_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        dst[i] = picoui_backend_map_grid_track(src[i]);
    }
    for (; i < PICOUI_BACKEND_LAYOUT_MAX_TRACKS; ++i) {
        dst[i] = LD_GRID_TEMPLATE_LAST;
    }
    return 0;
}

static ldWindow_t *picoui_backend_get_ld_window(struct picoui_backend_widget *backend_widget)
{
    ldBase_t *ld_base;

    if (backend_widget == NULL || backend_widget->ld_widget == NULL) {
        return NULL;
    }
    if (backend_widget->kind != PICOUI_BACKEND_WIDGET_WINDOW
        && backend_widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND) {
        return NULL;
    }

    ld_base = (ldBase_t *)backend_widget->ld_widget;
    if (ld_base->widgetType != widgetTypeWindow
        && ld_base->widgetType != widgetTypeBackground) {
        return NULL;
    }
    return (ldWindow_t *)ld_base;
}

static ldBase_t *picoui_backend_get_ld_base(struct picoui_backend_widget *backend_widget)
{
    ldBase_t *ld_base;
    int expected_widget_type;

    if (backend_widget == NULL || backend_widget->ld_widget == NULL) {
        return NULL;
    }

    ld_base = (ldBase_t *)backend_widget->ld_widget;
    expected_widget_type = picoui_backend_expected_ld_widget_type(backend_widget->kind);
    if (expected_widget_type < 0 || ld_base->widgetType != expected_widget_type) {
        return NULL;
    }
    return ld_base;
}

static ldPadding_t picoui_backend_uniform_padding(int padding)
{
    int16_t value = (int16_t)padding;

    return (ldPadding_t){
        .left = value,
        .top = value,
        .right = value,
        .bottom = value,
    };
}

static ldPadding_t picoui_backend_padding_group(int left, int top, int right, int bottom)
{
    return (ldPadding_t){
        .left = (int16_t)left,
        .top = (int16_t)top,
        .right = (int16_t)right,
        .bottom = (int16_t)bottom,
    };
}

static ldLayoutType_t picoui_backend_map_window_layout_type(enum picoui_window_layout_type type)
{
    switch (type) {
    case PICOUI_WINDOW_LAYOUT_FLEX:
        return layoutFlex;
    case PICOUI_WINDOW_LAYOUT_GRID:
        return layoutGrid;
    case PICOUI_WINDOW_LAYOUT_NONE:
    default:
        return layoutNone;
    }
}

static void picoui_backend_apply_window_padding(struct picoui_backend_widget *widget, ldWindow_t *ld_window)
{
    ldPadding_t ld_padding;
    ldLayoutType_t layout_type;

    if (widget == 0 || ld_window == NULL) {
        return;
    }

    layout_type = ld_window->layoutTpye;
    if (widget->window_layout.has_explicit_flex_padding) {
        ld_padding = picoui_backend_padding_group(widget->window_layout.padding_left,
                                                  widget->window_layout.padding_top,
                                                  widget->window_layout.padding_right,
                                                  widget->window_layout.padding_bottom);
    } else {
        ld_padding = picoui_backend_uniform_padding(widget->window_layout.padding);
    }
    ldWindowSetPadding(ld_window, ld_padding);
    if (widget->window_layout.has_explicit_grid_padding) {
        ldWindowSetGridPadding(ld_window,
                               picoui_backend_padding_group(widget->window_layout.grid_padding_left,
                                                            widget->window_layout.grid_padding_top,
                                                            widget->window_layout.grid_padding_right,
                                                            widget->window_layout.grid_padding_bottom));
    } else {
        ldWindowSetGridPadding(ld_window, ld_padding);
    }
    ld_window->layoutTpye = layout_type;
}

/**
 * @brief Set padding of widget backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] padding padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_padding(void *backend_widget, int padding)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldWindow_t *ld_window;

    if (widget == 0 || padding < 0) {
        return -1;
    }

    if (widget->kind != PICOUI_BACKEND_WIDGET_WINDOW
        && widget->kind != PICOUI_BACKEND_WIDGET_BACKGROUND) {
        return 0;
    }

    widget->window_layout.padding = padding;
    widget->window_layout.padding_left = padding;
    widget->window_layout.padding_top = padding;
    widget->window_layout.padding_right = padding;
    widget->window_layout.padding_bottom = padding;
    widget->window_layout.has_explicit_flex_padding = 0;
    widget->window_layout.grid_padding_left = padding;
    widget->window_layout.grid_padding_top = padding;
    widget->window_layout.grid_padding_right = padding;
    widget->window_layout.grid_padding_bottom = padding;
    widget->window_layout.has_explicit_grid_padding = 0;
    ld_window = picoui_backend_get_ld_window(widget);
    if (ld_window == NULL) {
        return -1;
    }

    picoui_backend_apply_window_padding(widget, ld_window);
    return 0;
}

/**
 * @brief Set flex flow of window backend
 *
 * @param[in] window Window instance
 * @param[in] flow flow
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_flex_flow(struct picoui_window *window, enum picoui_flex_flow flow)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetFlexFlow(ld_window, picoui_backend_map_flex_flow(flow));
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set flex align of window backend
 *
 * @param[in] window Window instance
 * @param[in] main_align main align
 * @param[in] cross_align cross align
 * @param[in] track_align track align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_flex_align(struct picoui_window *window,
                                         enum picoui_align main_align,
                                         enum picoui_align cross_align,
                                         enum picoui_align track_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetFlexAlign(ld_window,
                         picoui_backend_map_flex_main_align(main_align),
                         picoui_backend_map_flex_cross_align(cross_align));
    ldWindowSetFlexTrackAlign(ld_window, picoui_backend_map_flex_track_align(track_align));
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set flex gap of window backend
 *
 * @param[in] window Window instance
 * @param[in] item_gap item gap
 * @param[in] track_gap track gap
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_flex_gap(struct picoui_window *window, int item_gap, int track_gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetFlexGap(ld_window, (int16_t)item_gap, (int16_t)track_gap);
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set grid columns of window backend
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_columns(struct picoui_window *window, const int *tracks, int count)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || tracks == 0 || count <= 0 || count > PICOUI_BACKEND_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    if (picoui_backend_copy_tracks(backend_widget->window_layout.grid_cols, tracks, count) != 0) {
        return -1;
    }
    backend_widget->window_layout.grid_col_count = count;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetGridDscArray(ld_window,
                            backend_widget->window_layout.grid_cols,
                            backend_widget->window_layout.grid_row_count > 0
                                ? backend_widget->window_layout.grid_rows
                                : NULL);
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set grid rows of window backend
 *
 * @param[in] window Window instance
 * @param[in] tracks tracks
 * @param[in] count Count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_rows(struct picoui_window *window, const int *tracks, int count)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || tracks == 0 || count <= 0 || count > PICOUI_BACKEND_LAYOUT_MAX_TRACKS) {
        return -1;
    }

    if (picoui_backend_copy_tracks(backend_widget->window_layout.grid_rows, tracks, count) != 0) {
        return -1;
    }
    backend_widget->window_layout.grid_row_count = count;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetGridDscArray(ld_window,
                            backend_widget->window_layout.grid_col_count > 0
                                ? backend_widget->window_layout.grid_cols
                                : NULL,
                            backend_widget->window_layout.grid_rows);
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set grid gap of window backend
 *
 * @param[in] window Window instance
 * @param[in] row_gap row gap
 * @param[in] col_gap col gap
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_gap(struct picoui_window *window, int row_gap, int col_gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetGridGap(ld_window, (int16_t)row_gap, (int16_t)col_gap);
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set grid align of window backend
 *
 * @param[in] window Window instance
 * @param[in] col_align col align
 * @param[in] row_align row align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_align(struct picoui_window *window,
                                         enum picoui_align col_align,
                                         enum picoui_align row_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0) {
        return -1;
    }

    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetGridAlign(ld_window,
                         picoui_backend_map_grid_align(col_align),
                         picoui_backend_map_grid_align(row_align));
    picoui_backend_apply_window_padding(backend_widget, ld_window);
    return 0;
}

/**
 * @brief Set layout type of window backend
 *
 * @param[in] window Window instance
 * @param[in] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_layout_type(struct picoui_window *window,
                                          enum picoui_window_layout_type type)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0
        || (type != PICOUI_WINDOW_LAYOUT_NONE
            && type != PICOUI_WINDOW_LAYOUT_FLEX
            && type != PICOUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetLayout(ld_window, picoui_backend_map_window_layout_type(type));
    return 0;
}

/**
 * @brief Set padding of window backend
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_padding(struct picoui_window *window,
                                      int left,
                                      int top,
                                      int right,
                                      int bottom)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }

    backend_widget->window_layout.padding_left = left;
    backend_widget->window_layout.padding_top = top;
    backend_widget->window_layout.padding_right = right;
    backend_widget->window_layout.padding_bottom = bottom;
    backend_widget->window_layout.has_explicit_flex_padding = 1;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    {
        ldLayoutType_t layout_type = ld_window->layoutTpye;
        ldWindowSetPadding(ld_window, picoui_backend_padding_group(left, top, right, bottom));
        ld_window->layoutTpye = layout_type;
    }
    return 0;
}

/**
 * @brief Set grid padding of window backend
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_grid_padding(struct picoui_window *window,
                                           int left,
                                           int top,
                                           int right,
                                           int bottom)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || left < 0 || top < 0 || right < 0 || bottom < 0) {
        return -1;
    }

    backend_widget->window_layout.grid_padding_left = left;
    backend_widget->window_layout.grid_padding_top = top;
    backend_widget->window_layout.grid_padding_right = right;
    backend_widget->window_layout.grid_padding_bottom = bottom;
    backend_widget->window_layout.has_explicit_grid_padding = 1;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    {
        ldLayoutType_t layout_type = ld_window->layoutTpye;
        ldWindowSetGridPadding(ld_window, picoui_backend_padding_group(left, top, right, bottom));
        ld_window->layoutTpye = layout_type;
    }
    return 0;
}

/**
 * @brief Set gap of window backend
 *
 * @param[in] window Window instance
 * @param[in] gap Gap in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_backend_window_set_gap(struct picoui_window *window, int gap)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_window_get(window);
    ldWindow_t *ld_window;

    if (backend_widget == 0 || gap < 0) {
        return -1;
    }

    backend_widget->window_layout.flex_item_gap = gap;
    backend_widget->window_layout.flex_track_gap = gap;
    ld_window = picoui_backend_get_ld_window(backend_widget);
    if (ld_window == NULL) {
        return -1;
    }
    ldWindowSetGap(ld_window, (int16_t)gap);
    return 0;
}

/**
 * @brief Set flex grow of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] grow grow
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_flex_grow(struct picoui_widget *widget, int grow)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;

    if (backend_widget == 0 || grow < 0) {
        return -1;
    }

    backend_widget->child_layout.flex_grow = grow;
    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base == NULL) {
        return -1;
    }
    ldBaseSetFlexGrow(ld_base, (uint16_t)grow);
    return 0;
}

/**
 * @brief Set flex new track of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] new_track new track
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_flex_new_track(struct picoui_widget *widget, int new_track)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->child_layout.flex_new_track = new_track != 0;
    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base == NULL) {
        return -1;
    }
    ldBaseSetFlexNewTrack(ld_base, new_track != 0);
    return 0;
}

/**
 * @brief Set ignore layout of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] ignore_layout ignore layout
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;

    if (backend_widget == 0) {
        return -1;
    }

    backend_widget->child_layout.ignore_layout = ignore_layout != 0;
    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base == NULL) {
        return -1;
    }
    ldBaseSetIgnoreLayout(ld_base, ignore_layout != 0);
    return 0;
}

/**
 * @brief Set grid cell of widget backend
 *
 * @param[in] widget Widget instance
 * @param[in] col col
 * @param[in] row Row index
 * @param[in] col_span Column span count
 * @param[in] row_span Row span count
 * @param[in] x_align x align
 * @param[in] y_align y align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_widget_set_grid_cell(struct picoui_widget *widget,
                                        int col,
                                        int row,
                                        int col_span,
                                        int row_span,
                                        enum picoui_align x_align,
                                        enum picoui_align y_align)
{
    struct picoui_backend_widget *backend_widget = picoui_backend_widget_get(widget);
    ldBase_t *ld_base;
    ldGridAlign_t grid_x_align;
    ldGridAlign_t grid_y_align;

    if (backend_widget == 0 || col_span <= 0 || row_span <= 0) {
        return -1;
    }

    backend_widget->child_layout.grid_col = col;
    backend_widget->child_layout.grid_row = row;
    backend_widget->child_layout.grid_col_span = col_span;
    backend_widget->child_layout.grid_row_span = row_span;
    backend_widget->child_layout.grid_x_align = x_align;
    backend_widget->child_layout.grid_y_align = y_align;

    ld_base = picoui_backend_get_ld_base(backend_widget);
    if (ld_base == NULL) {
        return -1;
    }
    grid_x_align = picoui_backend_map_grid_align(x_align);
    grid_y_align = picoui_backend_map_grid_align(y_align);
    ldBaseSetGridCell(ld_base,
                      grid_x_align,
                      (int16_t)col,
                      (int16_t)col_span,
                      grid_y_align,
                      (int16_t)row,
                      (int16_t)row_span);
    return 0;
}
