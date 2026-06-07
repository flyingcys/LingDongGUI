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
#include "picoui/display.h"
#include "picoui/widget.h"
#include "picoui/screen.h"
#include "picoui/window.h"

#include <stdlib.h>

struct picoui_image_source;

int picoui_backend_window_set_background_source(struct picoui_window *window,
                                                struct picoui_image_source *source);
int picoui_backend_window_set_background_offset(struct picoui_window *window,
                                                int offset_x,
                                                int offset_y);
int picoui_backend_window_set_bg_color(struct picoui_window *window, unsigned int rgb);
int picoui_backend_window_get_bg_color(struct picoui_window *window, unsigned int *rgb);
int picoui_backend_window_set_padding_group(struct picoui_window *window,
                                            int left,
                                            int top,
                                            int right,
                                            int bottom);
int picoui_backend_window_get_padding_left(struct picoui_window *window);
int picoui_backend_window_get_padding_top(struct picoui_window *window);
int picoui_backend_window_get_padding_right(struct picoui_window *window);
int picoui_backend_window_get_padding_bottom(struct picoui_window *window);
int picoui_backend_window_set_layout_type(struct picoui_window *window,
                                          enum picoui_window_layout_type type);
int picoui_backend_window_set_padding(struct picoui_window *window,
                                      int left,
                                      int top,
                                      int right,
                                      int bottom);
int picoui_backend_window_set_grid_padding(struct picoui_window *window,
                                           int left,
                                           int top,
                                           int right,
                                           int bottom);
int picoui_backend_window_set_gap(struct picoui_window *window, int gap);

static void picoui_window_destroy_root_partial(struct picoui_app *compat_app,
                                               struct picoui_window *window)
{
    if (window != 0) {
        if (window->widget.backend_widget != 0) {
            (void)picoui_backend_widget_unbind_host(window->widget.backend_widget);
            free(window->widget.backend_widget);
            window->widget.backend_widget = 0;
        }
        free(window);
    }
    picoui_app_destroy(compat_app);
}

static int picoui_window_is_valid(struct picoui_window *window)
{
    return window != 0 && window->widget.backend_widget != 0;
}

static int picoui_window_props_are_valid(const struct picoui_window_props *props)
{
    return props != 0
        && props->id != 0
        && props->radius >= 0
        && props->padding >= 0
        && props->padding_left >= 0
        && props->padding_top >= 0
        && props->padding_right >= 0
        && props->padding_bottom >= 0;
}

/**
 * @brief Create window widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id)
{
    struct picoui_window *window;
    void *backend_widget;

    if (app == 0 || id == 0) {
        return 0;
    }

    backend_widget = picoui_backend_create_window(app, id);
    if (backend_widget == 0) {
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        free(backend_widget);
        return 0;
    }

    window->id = id;
    window->widget.backend_widget = backend_widget;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    window->flex_flow = PICOUI_FLEX_FLOW_ROW;
    window->flex_main_align = PICOUI_ALIGN_START;
    window->flex_cross_align = PICOUI_ALIGN_START;
    window->flex_track_align = PICOUI_ALIGN_START;
    window->grid_col_align = PICOUI_ALIGN_START;
    window->grid_row_align = PICOUI_ALIGN_START;
    if (picoui_backend_widget_bind_host(window->widget.backend_widget, &window->widget) != 0) {
        (void)picoui_backend_widget_unbind_host(window->widget.backend_widget);
        free(window->widget.backend_widget);
        free(window);
        return 0;
    }
    return window;
}

struct picoui_window *picoui_window_create_root(struct picoui_screen *screen, const char *id)
{
    struct picoui_app *compat_app;
    struct picoui_window *window;
    struct picoui_display *display;
    struct picoui_display_config config = {0};
    int width = 0;
    int height = 0;

    if (screen == 0 || id == 0) {
        return 0;
    }

    /* P1 compatibility shim; replace with native root ownership in P1-F or later, not P7. */
    compat_app = picoui_app_create();
    if (compat_app == 0) {
        return 0;
    }

    display = picoui_display_get_default();
    if (display != 0 && picoui_display_get_size(display, &width, &height) == 0) {
        config.width = width;
        config.height = height;
        config.color_format = PICOUI_COLOR_FORMAT_RGB565;
        config.buffer_height = 0;
        config.user_data = 0;
        if (picoui_display_set_config(compat_app, &config) != 0) {
            picoui_app_destroy(compat_app);
            return 0;
        }
    }

    window = picoui_window_create(compat_app, id);
    if (window == 0) {
        picoui_app_destroy(compat_app);
        return 0;
    }

    if (picoui_screen_set_root_window(screen, window) != 0) {
        picoui_window_destroy_root_partial(compat_app, window);
        return 0;
    }

    return window;
}

struct picoui_window *picoui_window_create_child(struct picoui_window *parent, const char *id)
{
    struct picoui_window *window;
    void *backend_widget;
    struct picoui_backend_widget *parent_backend;

    if (parent == 0 || id == 0 || parent->widget.backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->widget.backend_widget;
    backend_widget = picoui_backend_create_child_window(parent_backend, id);
    if (backend_widget == 0) {
        return 0;
    }

    window = calloc(1, sizeof(*window));
    if (window == 0) {
        free(backend_widget);
        return 0;
    }

    window->id = id;
    window->widget.backend_widget = backend_widget;
    window->widget.visible = 1;
    window->widget.enabled = 1;
    window->flex_flow = PICOUI_FLEX_FLOW_ROW;
    window->flex_main_align = PICOUI_ALIGN_START;
    window->flex_cross_align = PICOUI_ALIGN_START;
    window->flex_track_align = PICOUI_ALIGN_START;
    window->grid_col_align = PICOUI_ALIGN_START;
    window->grid_row_align = PICOUI_ALIGN_START;
    if (picoui_backend_widget_bind_host(window->widget.backend_widget, &window->widget) != 0) {
        free(window);
        return 0;
    }
    return window;
}

/**
 * @brief Create window widget with properties
 *
 * @param[in] app Application instance
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_window *picoui_window_create_with_props(struct picoui_app *app,
                                                      const struct picoui_window_props *props)
{
    struct picoui_window *window;

    if (!picoui_window_props_are_valid(props)) {
        return 0;
    }

    window = picoui_window_create(app, props->id);
    if (window == 0) {
        return 0;
    }

    if (props->style_class != 0
        && picoui_widget_set_style_class(&window->widget, props->style_class) != 0) {
        free(window);
        return 0;
    }
    if (picoui_widget_set_user_data(&window->widget, props->user_data) != 0
        || picoui_widget_set_bg_color(&window->widget, props->bg_color) != 0
        || picoui_backend_window_set_bg_color(window, props->bg_color) != 0
        || picoui_widget_set_text_color(&window->widget, props->text_color) != 0
        || picoui_widget_set_border_color(&window->widget, props->border_color) != 0
        || picoui_widget_set_radius(&window->widget, props->radius) != 0
        || picoui_widget_set_padding(&window->widget, props->padding) != 0
        || picoui_window_set_background_source(window, props->background_source) != 0
        || (props->has_padding_group != 0
            && picoui_window_set_padding_group(window,
                                               props->padding_left,
                                               props->padding_top,
                                               props->padding_right,
                                               props->padding_bottom) != 0)) {
        free(window);
        return 0;
    }

    return window;
}

/**
 * @brief Set background source of window
 *
 * @param[in] window Window instance
 * @param[in] source Image source
 * @return -1 on failure
 */

int picoui_window_set_background_source(struct picoui_window *window,
                                        struct picoui_image_source *source)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    return picoui_backend_window_set_background_source(window, source);
}

/**
 * @brief Set background offset of window
 *
 * @param[in] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_background_offset(struct picoui_window *window, int offset_x, int offset_y)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }

    if (picoui_backend_window_set_background_offset(window, offset_x, offset_y) != 0) {
        return -1;
    }

    window->background_offset_x = offset_x;
    window->background_offset_y = offset_y;
    return 0;
}

/**
 * @brief Get background offset of window
 *
 * @param[out] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_window_get_background_offset(struct picoui_window *window,
                                        int *offset_x,
                                        int *offset_y)
{
    if (!picoui_window_is_valid(window) || offset_x == 0 || offset_y == 0) {
        return -1;
    }

    *offset_x = window->background_offset_x;
    *offset_y = window->background_offset_y;
    return 0;
}

/**
 * @brief Set color of window
 *
 * @param[in] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_window_set_color(struct picoui_window *window, unsigned int rgb)
{
    if (!picoui_window_is_valid(window) || rgb > 0xFFFFFFU) {
        return -1;
    }

    window->widget.bg_color = rgb;
    return picoui_backend_window_set_bg_color(window, rgb);
}

/**
 * @brief Get color of window
 *
 * @param[out] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_window_get_color(struct picoui_window *window, unsigned int *rgb)
{
    if (!picoui_window_is_valid(window) || rgb == 0) {
        return -1;
    }

    return picoui_backend_window_get_bg_color(window, rgb);
}

/**
 * @brief Set padding group of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int picoui_window_set_padding_group(struct picoui_window *window,
                                    int left,
                                    int top,
                                    int right,
                                    int bottom)
{
    if (!picoui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return picoui_backend_window_set_padding_group(window, left, top, right, bottom);
}

/**
 * @brief Set layout type of window
 *
 * @param[in] window Window instance
 * @param[in] type Type
 * @return -1 on failure
 */

int picoui_window_set_layout_type(struct picoui_window *window,
                                  enum picoui_window_layout_type type)
{
    if (!picoui_window_is_valid(window)
        || (type != PICOUI_WINDOW_LAYOUT_NONE
            && type != PICOUI_WINDOW_LAYOUT_FLEX
            && type != PICOUI_WINDOW_LAYOUT_GRID)) {
        return -1;
    }

    return picoui_backend_window_set_layout_type(window, type);
}

/**
 * @brief Set padding of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int picoui_window_set_padding(struct picoui_window *window,
                              int left,
                              int top,
                              int right,
                              int bottom)
{
    if (!picoui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return picoui_backend_window_set_padding(window, left, top, right, bottom);
}

/**
 * @brief Set grid padding of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return -1 on failure
 */

int picoui_window_set_grid_padding(struct picoui_window *window,
                                   int left,
                                   int top,
                                   int right,
                                   int bottom)
{
    if (!picoui_window_is_valid(window)
        || left < 0
        || top < 0
        || right < 0
        || bottom < 0) {
        return -1;
    }

    return picoui_backend_window_set_grid_padding(window, left, top, right, bottom);
}

/**
 * @brief Set gap of window
 *
 * @param[in] window Window instance
 * @param[in] gap Gap in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_gap(struct picoui_window *window, int gap)
{
    if (!picoui_window_is_valid(window) || gap < 0) {
        return -1;
    }

    if (picoui_backend_window_set_gap(window, gap) != 0) {
        return -1;
    }
    window->flex_item_gap = gap;
    window->flex_track_gap = gap;
    return 0;
}

/**
 * @brief Get padding left of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_window_get_padding_left(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_left(window);
}

/**
 * @brief Get padding top of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_window_get_padding_top(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_top(window);
}

/**
 * @brief Get padding right of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_window_get_padding_right(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_right(window);
}

/**
 * @brief Get padding bottom of window
 *
 * @param[out] window Window instance
 * @return -1 on failure
 */

int picoui_window_get_padding_bottom(struct picoui_window *window)
{
    if (!picoui_window_is_valid(window)) {
        return -1;
    }
    return picoui_backend_window_get_padding_bottom(window);
}
