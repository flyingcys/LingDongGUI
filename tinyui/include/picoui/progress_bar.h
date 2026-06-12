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

#ifndef PICOUI_PROGRESS_BAR_H
#define PICOUI_PROGRESS_BAR_H

#include "../widget.h"

struct picoui_widget;
struct picoui_progress_bar;
struct picoui_image_source;

struct picoui_progress_bar_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int percent;
    int horizontal;
    int inverted;
};

struct picoui_window;

/**
 * @brief Create progress bar widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_bar *picoui_progress_bar_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create progress bar widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_bar *picoui_progress_bar_create_with_props(
    struct picoui_window *parent,
    const struct picoui_progress_bar_props *props);

/**
 * @brief progress bar init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_progress_bar *picoui_progress_bar_init(struct picoui_window *parent, const char *id);

/**
 * @brief Set percent of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_percent(struct picoui_progress_bar *bar, int percent);

/**
 * @brief Get percent of progress bar widget
 *
 * @param[in] bar bar
 * @return The property value, negative on error
 */

int picoui_progress_bar_get_percent(const struct picoui_progress_bar *bar);

/**
 * @brief Set horizontal of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_horizontal(struct picoui_progress_bar *bar, int horizontal);

/**
 * @brief Get horizontal of progress bar widget
 *
 * @param[in] bar bar
 * @return The property value, negative on error
 */

int picoui_progress_bar_get_horizontal(const struct picoui_progress_bar *bar);

/**
 * @brief Set image of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] bg_source bg source
 * @param[in] fg_source fg source
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_image(struct picoui_progress_bar *bar,
                                  struct picoui_image_source *bg_source,
                                  struct picoui_image_source *fg_source);

/**
 * @brief Set bg source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_bg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source);

/**
 * @brief Set fg source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_fg_source(struct picoui_progress_bar *bar, struct picoui_image_source *source);

/**
 * @brief Set frame source of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_frame_source(struct picoui_progress_bar *bar, struct picoui_image_source *source);

/**
 * @brief Set color of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] bg_color Background color
 * @param[in] fg_color Foreground color
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_color(struct picoui_progress_bar *bar, unsigned int bg_color, unsigned int fg_color);

/**
 * @brief Set frame color of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] frame_color frame color
 * @param[in] frame_color_size frame color size
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_frame_color(struct picoui_progress_bar *bar,
                                        unsigned int frame_color,
                                        int frame_color_size);

/**
 * @brief Set inverted of progress bar widget
 *
 * @param[in] bar bar
 * @param[in] inverted inverted
 * @return 0 on success, -1 on failure
 */

int picoui_progress_bar_set_inverted(struct picoui_progress_bar *bar, int inverted);

/**
 * @brief Get inverted of progress bar widget
 *
 * @param[in] bar bar
 * @return The property value, negative on error
 */

int picoui_progress_bar_get_inverted(const struct picoui_progress_bar *bar);

#endif
