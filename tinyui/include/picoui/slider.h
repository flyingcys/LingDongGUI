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

#ifndef PICOUI_SLIDER_H
#define PICOUI_SLIDER_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_slider;
struct picoui_image_source;

struct picoui_slider_props {
    const char *id;
    int min_value;
    int max_value;
    int value;
    picoui_value_changed_cb on_value_changed;
    void *user_data;
    const char *style_class;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;

    int horizontal;
    struct picoui_image_source *background_source;
    struct picoui_image_source *indicator_source;
    int indicator_width;
    int slim_size;

    int has_horizontal;
    int has_background_source;
    int has_indicator_source;
    int has_indicator_width;
    int has_slim_size;
};

/**
 * @brief Create slider widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_slider *picoui_slider_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create slider widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_slider *picoui_slider_create_with_props(struct picoui_window *parent,
                                                      const struct picoui_slider_props *props);

/**
 * @brief slider init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_slider *picoui_slider_init(struct picoui_window *parent, const char *id);

/**
 * @brief Set value of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_value(struct picoui_slider *slider, int value);

/**
 * @brief Set range of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] min_value min value
 * @param[in] max_value max value
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_range(struct picoui_slider *slider, int min_value, int max_value);

/**
 * @brief Set percent of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] percent percent
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_percent(struct picoui_slider *slider, int percent);

/**
 * @brief Set horizontal of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_horizontal(struct picoui_slider *slider, int horizontal);

/**
 * @brief Get horizontal of slider widget
 *
 * @param[out] slider Slider widget instance
 * @param[in] horizontal horizontal
 * @return The property value, negative on error
 */

int picoui_slider_get_horizontal(struct picoui_slider *slider, int *horizontal);

/**
 * @brief Set background source of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_background_source(struct picoui_slider *slider,
                                        struct picoui_image_source *source);

/**
 * @brief Set indicator source of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_indicator_source(struct picoui_slider *slider,
                                       struct picoui_image_source *source);

/**
 * @brief Set image of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] background_source background source
 * @param[in] indicator_source indicator source
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_image(struct picoui_slider *slider,
                            struct picoui_image_source *background_source,
                            struct picoui_image_source *indicator_source);

/**
 * @brief Set color of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] bg_color Background color
 * @param[in] frame_color frame color
 * @param[in] indicator_color indicator color
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_color(struct picoui_slider *slider,
                            unsigned int bg_color,
                            unsigned int frame_color,
                            unsigned int indicator_color);

/**
 * @brief Set indicator width of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] indicator_width indicator width
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_indicator_width(struct picoui_slider *slider, int indicator_width);

/**
 * @brief Set slim size of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] slim_size slim size
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_slim_size(struct picoui_slider *slider, int slim_size);

/**
 * @brief Get percent of slider widget
 *
 * @param[out] slider Slider widget instance
 * @param[in] percent percent
 * @return The property value, negative on error
 */

int picoui_slider_get_percent(struct picoui_slider *slider, int *percent);

/**
 * @brief Set on value changed of slider widget
 *
 * @param[in] slider Slider widget instance
 * @param[in] cb cb
 * @param[in] user_data User data pointer
 * @return 0 on success, -1 on failure
 */

int picoui_slider_set_on_value_changed(struct picoui_slider *slider,
                                       picoui_value_changed_cb cb,
                                       void *user_data);

#endif
