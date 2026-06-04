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

#ifndef PICOUI_WINDOW_H
#define PICOUI_WINDOW_H

struct picoui_app;
struct picoui_image_source;
struct picoui_window;

enum picoui_window_layout_type {
    PICOUI_WINDOW_LAYOUT_NONE,
    PICOUI_WINDOW_LAYOUT_FLEX,
    PICOUI_WINDOW_LAYOUT_GRID,
};

struct picoui_window_props {
    const char *id;
    const char *style_class;
    void *user_data;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    struct picoui_image_source *background_source;
    int has_padding_group;
    int padding_left;
    int padding_top;
    int padding_right;
    int padding_bottom;
};

/**
 * @brief Create window widget
 *
 * @param[in] app Application instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_window *picoui_window_create(struct picoui_app *app, const char *id);

/**
 * @brief Create child window widget under an existing window
 *
 * @param[in] parent Parent window instance
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_window *picoui_window_create_child(struct picoui_window *parent, const char *id);

/**
 * @brief Create window widget with properties
 *
 * @param[in] app Application instance
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_window *picoui_window_create_with_props(struct picoui_app *app,
                                                      const struct picoui_window_props *props);

/**
 * @brief Set background source of window
 *
 * @param[in] window Window instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_background_source(struct picoui_window *window,
                                        struct picoui_image_source *source);

/**
 * @brief Set background offset of window
 *
 * @param[in] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_background_offset(struct picoui_window *window, int offset_x, int offset_y);

/**
 * @brief Get background offset of window
 *
 * @param[out] window Window instance
 * @param[in] offset_x Horizontal offset
 * @param[in] offset_y Vertical offset
 * @return The property value, negative on error
 */

int picoui_window_get_background_offset(struct picoui_window *window,
                                        int *offset_x,
                                        int *offset_y);

/**
 * @brief Set color of window
 *
 * @param[in] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_color(struct picoui_window *window, unsigned int rgb);

/**
 * @brief Get color of window
 *
 * @param[out] window Window instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_window_get_color(struct picoui_window *window, unsigned int *rgb);

/**
 * @brief Set padding group of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_padding_group(struct picoui_window *window,
                                    int left,
                                    int top,
                                    int right,
                                    int bottom);

/**
 * @brief Set layout type of window
 *
 * @param[in] window Window instance
 * @param[in] type Type
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_layout_type(struct picoui_window *window,
                                  enum picoui_window_layout_type type);

/**
 * @brief Set padding of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_padding(struct picoui_window *window,
                              int left,
                              int top,
                              int right,
                              int bottom);

/**
 * @brief Set grid padding of window
 *
 * @param[in] window Window instance
 * @param[in] left Left padding
 * @param[in] top Top padding
 * @param[in] right Right padding
 * @param[in] bottom Bottom padding
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_grid_padding(struct picoui_window *window,
                                   int left,
                                   int top,
                                   int right,
                                   int bottom);

/**
 * @brief Set gap of window
 *
 * @param[in] window Window instance
 * @param[in] gap Gap in pixels
 * @return 0 on success, -1 on failure
 */

int picoui_window_set_gap(struct picoui_window *window, int gap);

/**
 * @brief Get padding left of window
 *
 * @param[out] window Window instance
 * @return The property value, negative on error
 */

int picoui_window_get_padding_left(struct picoui_window *window);

/**
 * @brief Get padding top of window
 *
 * @param[out] window Window instance
 * @return The property value, negative on error
 */

int picoui_window_get_padding_top(struct picoui_window *window);

/**
 * @brief Get padding right of window
 *
 * @param[out] window Window instance
 * @return The property value, negative on error
 */

int picoui_window_get_padding_right(struct picoui_window *window);

/**
 * @brief Get padding bottom of window
 *
 * @param[out] window Window instance
 * @return The property value, negative on error
 */

int picoui_window_get_padding_bottom(struct picoui_window *window);

#endif
