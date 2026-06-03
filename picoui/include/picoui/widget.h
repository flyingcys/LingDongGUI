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

#ifndef PICOUI_WIDGET_H
#define PICOUI_WIDGET_H

#include "picoui/layout.h"
#include "picoui/native.h"
#include "picoui/theme.h"

struct picoui_widget;
struct picoui_app;
struct picoui_font;

enum picoui_font_kind {
    PICOUI_FONT_KIND_FAMILY = 0,
    PICOUI_FONT_KIND_VRES = 1,
};

typedef void (*picoui_value_changed_cb)(struct picoui_widget *widget,
                                        int value,
                                        void *user_data);
typedef void (*picoui_event_cb)(struct picoui_widget *widget, void *user_data);

struct picoui_font {
    const char *family;
    int size;
    enum picoui_font_kind kind;
    unsigned int vres_addr;
};

struct picoui_point {
    int x;
    int y;
};

struct picoui_size {
    int width;
    int height;
};

struct picoui_rect {
    int x;
    int y;
    int width;
    int height;
};

enum picoui_widget_type {
    PICOUI_WIDGET_TYPE_UNKNOWN = 0,
    PICOUI_WIDGET_TYPE_BACKGROUND,
    PICOUI_WIDGET_TYPE_WINDOW,
    PICOUI_WIDGET_TYPE_BUTTON,
    PICOUI_WIDGET_TYPE_IMAGE,
    PICOUI_WIDGET_TYPE_TEXT,
    PICOUI_WIDGET_TYPE_LINE_EDIT,
    PICOUI_WIDGET_TYPE_GRAPH,
    PICOUI_WIDGET_TYPE_CHECKBOX,
    PICOUI_WIDGET_TYPE_SLIDER,
    PICOUI_WIDGET_TYPE_SWITCH,
    PICOUI_WIDGET_TYPE_PROGRESS_BAR,
    PICOUI_WIDGET_TYPE_GAUGE,
    PICOUI_WIDGET_TYPE_QRCODE,
    PICOUI_WIDGET_TYPE_DATE_TIME,
    PICOUI_WIDGET_TYPE_ICON_SLIDER,
    PICOUI_WIDGET_TYPE_COMBO_BOX,
    PICOUI_WIDGET_TYPE_ARC,
    PICOUI_WIDGET_TYPE_RADIAL_MENU,
    PICOUI_WIDGET_TYPE_SCROLL_SELECTER,
    PICOUI_WIDGET_TYPE_LABEL,
    PICOUI_WIDGET_TYPE_TABLE,
    PICOUI_WIDGET_TYPE_KEYBOARD,
    PICOUI_WIDGET_TYPE_ANIMATION,
    PICOUI_WIDGET_TYPE_LIST,
    PICOUI_WIDGET_TYPE_MESSAGE_BOX,
    PICOUI_WIDGET_TYPE_CALENDAR,
    PICOUI_WIDGET_TYPE_PROGRESS_WHEEL,
    PICOUI_WIDGET_TYPE_CLOCK,
    PICOUI_WIDGET_TYPE_CANVAS,
};

/**
 * @brief Set pos of widget
 *
 * @param[in] widget Widget instance
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_pos(struct picoui_widget *widget, int x, int y);

/**
 * @brief Set size of widget
 *
 * @param[in] widget Widget instance
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_size(struct picoui_widget *widget, int width, int height);

/**
 * @brief Set text of widget
 *
 * @param[in] widget Widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_text(struct picoui_widget *widget, const char *text);

/**
 * @brief Set style class of widget
 *
 * @param[in] widget Widget instance
 * @param[in] style_class style class
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_style_class(struct picoui_widget *widget, const char *style_class);

/**
 * @brief Set user data of widget
 *
 * @param[in] widget Widget instance
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_user_data(struct picoui_widget *widget, void *user_data);

/**
 * @brief Set bg color of widget
 *
 * @param[in] widget Widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_bg_color(struct picoui_widget *widget, unsigned int rgb);

/**
 * @brief Set text color of widget
 *
 * @param[in] widget Widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_text_color(struct picoui_widget *widget, unsigned int rgb);

/**
 * @brief Set border color of widget
 *
 * @param[in] widget Widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_border_color(struct picoui_widget *widget, unsigned int rgb);

/**
 * @brief Set radius of widget
 *
 * @param[in] widget Widget instance
 * @param[in] radius Radius
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_radius(struct picoui_widget *widget, int radius);

/**
 * @brief Set padding of widget
 *
 * @param[in] widget Widget instance
 * @param[in] padding padding
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_padding(struct picoui_widget *widget, int padding);

/**
 * @brief Set center of widget
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_center(struct picoui_widget *widget);

/**
 * @brief Set visible of widget
 *
 * @param[in] widget Widget instance
 * @param[in] visible Visibility state
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_visible(struct picoui_widget *widget, int visible);

/**
 * @brief Check if widget is hidden
 *
 * @param[in] widget Widget instance
 * @return 1 if hidden, 0 if visible, -1 on failure
 */

int picoui_widget_is_hidden(struct picoui_widget *widget);

/**
 * @brief Set opacity of widget
 *
 * @param[in] widget Widget instance
 * @param[in] opacity Opacity (0-255)
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_opacity(struct picoui_widget *widget, int opacity);

/**
 * @brief Set selectable of widget
 *
 * @param[in] widget Widget instance
 * @param[in] selectable selectable
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_selectable(struct picoui_widget *widget, int selectable);

/**
 * @brief Set selected of widget
 *
 * @param[in] widget Widget instance
 * @param[in] selected selected
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_selected(struct picoui_widget *widget, int selected);

/**
 * @brief Set corner of widget
 *
 * @param[in] widget Widget instance
 * @param[in] corner corner
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_corner(struct picoui_widget *widget, int corner);

/**
 * @brief Set enabled of widget
 *
 * @param[in] widget Widget instance
 * @param[in] enabled Enable state
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_enabled(struct picoui_widget *widget, int enabled);

/**
 * @brief Set flex grow of widget
 *
 * @param[in] widget Widget instance
 * @param[in] grow grow
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_flex_grow(struct picoui_widget *widget, int grow);

/**
 * @brief Set flex new track of widget
 *
 * @param[in] widget Widget instance
 * @param[in] new_track new track
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_flex_new_track(struct picoui_widget *widget, int new_track);

/**
 * @brief Set flex min width of widget
 *
 * @param[in] widget Widget instance
 * @param[in] min_width min width
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_flex_min_width(struct picoui_widget *widget, int min_width);

/**
 * @brief Set flex min height of widget
 *
 * @param[in] widget Widget instance
 * @param[in] min_height min height
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_flex_min_height(struct picoui_widget *widget, int min_height);

/**
 * @brief Set flex max width of widget
 *
 * @param[in] widget Widget instance
 * @param[in] max_width max width
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_flex_max_width(struct picoui_widget *widget, int max_width);

/**
 * @brief Set flex max height of widget
 *
 * @param[in] widget Widget instance
 * @param[in] max_height max height
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_flex_max_height(struct picoui_widget *widget, int max_height);

/**
 * @brief Set ignore layout of widget
 *
 * @param[in] widget Widget instance
 * @param[in] ignore_layout ignore layout
 * @return 0 on success, -1 on failure
 */

int picoui_widget_set_ignore_layout(struct picoui_widget *widget, int ignore_layout);

/**
 * @brief Set grid cell of widget
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

int picoui_widget_set_grid_cell(struct picoui_widget *widget,
                                int col,
                                int row,
                                int col_span,
                                int row_span,
                                enum picoui_align x_align,
                                enum picoui_align y_align);

/**
 * @brief Widget: remove from parent
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_remove_from_parent(struct picoui_widget *widget);

/**
 * @brief Destroy widget widget
 *
 * @param[in] widget Widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_widget_destroy(struct picoui_widget *widget);

/**
 * @brief Get x of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_x(const struct picoui_widget *widget);

/**
 * @brief Get y of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_y(const struct picoui_widget *widget);

/**
 * @brief Get width of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_width(const struct picoui_widget *widget);

/**
 * @brief Get height of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_height(const struct picoui_widget *widget);

/**
 * @brief Get visible of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_visible(const struct picoui_widget *widget);

/**
 * @brief Get opacity of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_opacity(const struct picoui_widget *widget);

/**
 * @brief Get selectable of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_selectable(const struct picoui_widget *widget);

/**
 * @brief Get selected of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_selected(const struct picoui_widget *widget);

/**
 * @brief Get corner of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_corner(const struct picoui_widget *widget);

/**
 * @brief Get parent of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_widget *picoui_widget_get_parent(const struct picoui_widget *widget);

/**
 * @brief Get first child of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_widget *picoui_widget_get_first_child(const struct picoui_widget *widget);

/**
 * @brief Get next sibling of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_widget *picoui_widget_get_next_sibling(const struct picoui_widget *widget);

/**
 * @brief Get root of widget
 *
 * @param[in] widget Widget instance
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_widget *picoui_widget_get_root(const struct picoui_widget *widget);

/**
 * @brief Get child count of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_child_count(const struct picoui_widget *widget);

/**
 * @brief Get name id of widget
 *
 * @param[in] widget Widget instance
 * @return The property value, negative on error
 */

int picoui_widget_get_name_id(const struct picoui_widget *widget);

/**
 * @brief Widget: find by name id
 *
 * @param[in] root root
 * @param[in] name_id Name identifier ID
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_widget *picoui_widget_find_by_name_id(const struct picoui_widget *root, int name_id);

/**
 * @brief Get type of widget
 *
 * @param[in] widget Widget instance
 */

enum picoui_widget_type picoui_widget_get_type(const struct picoui_widget *widget);

/**
 * @brief Get absolute pos of widget
 *
 * @param[in] widget Widget instance
 * @param[in] point Point
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_point picoui_widget_get_absolute_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point);

/**
 * @brief Get relative pos of widget
 *
 * @param[in] widget Widget instance
 * @param[in] point Point
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_point picoui_widget_get_relative_pos(const struct picoui_widget *widget,
                                                   struct picoui_point point);

/**
 * @brief rect align
 *
 * @param[in] parent Parent widget
 * @param[in] child Child widget
 * @param[in] x_align x align
 * @param[in] y_align y align
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_rect picoui_rect_align(struct picoui_rect parent,
                                     struct picoui_rect child,
                                     enum picoui_align x_align,
                                     enum picoui_align y_align);

/**
 * @brief rect center
 *
 * @param[in] parent Parent widget
 * @param[in] child Child widget
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_rect picoui_rect_center(struct picoui_rect parent,
                                      struct picoui_rect child);

/**
 * @brief vertical grid align offset
 *
 * @param[in] widget Widget instance
 * @param[in] current_offset current offset
 * @param[in] item_count item count
 * @param[in] item_height item height
 * @param[in] space Spacing
 * @return 0 on success, -1 on failure
 */

int picoui_vertical_grid_align_offset(struct picoui_rect widget,
                                      int current_offset,
                                      int item_count,
                                      int item_height,
                                      int space);

/**
 * @brief focus reset
 *
 * @param[in] app Application instance
 * @return 0 on success, -1 on failure
 */

int picoui_focus_reset(struct picoui_app *app);

/**
 * @brief focus navigate
 *
 * @param[in] app Application instance
 * @param[in] dir dir
 * @return 0 on success, -1 on failure
 */

int picoui_focus_navigate(struct picoui_app *app, enum picoui_native_nav_dir dir);

#endif
