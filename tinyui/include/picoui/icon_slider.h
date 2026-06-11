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

#ifndef PICOUI_ICON_SLIDER_H
#define PICOUI_ICON_SLIDER_H

struct picoui_widget;
struct picoui_icon_slider;
struct picoui_image_source;

struct picoui_icon_slider_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int icon_width;
    int icon_space;
    int columns;
    int rows;
    int pages;
    int horizontal;
};

/**
 * @brief Create icon slider widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_icon_slider *picoui_icon_slider_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create icon slider widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_icon_slider *picoui_icon_slider_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_icon_slider_props *props
);

/**
 * @brief icon slider init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_icon_slider *picoui_icon_slider_init(struct picoui_widget *parent, const char *id);

/**
 * @brief icon slider add item
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_add_item(struct picoui_icon_slider *icon_slider,
                                const char *id,
                                const char *text);

/**
 * @brief icon slider add item with source
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_add_item_with_source(struct picoui_icon_slider *icon_slider,
                                            const char *id,
                                            const char *text,
                                            struct picoui_image_source *source);

/**
 * @brief icon slider add icon
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_add_icon(struct picoui_icon_slider *icon_slider,
                                const char *id,
                                const char *text,
                                struct picoui_image_source *source);

/**
 * @brief Set selected index of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_selected_index(struct picoui_icon_slider *icon_slider, int index);

/**
 * @brief Get selected index of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @return The property value, negative on error
 */

int picoui_icon_slider_get_selected_index(const struct picoui_icon_slider *icon_slider);

/**
 * @brief Set horizontal of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_horizontal(struct picoui_icon_slider *icon_slider, int horizontal);

/**
 * @brief Set horizontal scroll of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] horizontal horizontal
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_horizontal_scroll(struct picoui_icon_slider *icon_slider, int horizontal);

/**
 * @brief Get horizontal of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] horizontal horizontal
 * @return The property value, negative on error
 */

int picoui_icon_slider_get_horizontal(const struct picoui_icon_slider *icon_slider, int *horizontal);

/**
 * @brief Set speed of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] speed speed
 * @return 0 on success, -1 on failure
 */

int picoui_icon_slider_set_speed(struct picoui_icon_slider *icon_slider, int speed);

/**
 * @brief Set on selected of icon slider widget
 *
 * @param[in] icon_slider Icon slider widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void picoui_icon_slider_set_on_selected(struct picoui_icon_slider *icon_slider,
                                        void (*callback)(struct picoui_icon_slider *icon_slider,
                                                         int index,
                                                         void *user_data),
                                        void *user_data);

#endif
