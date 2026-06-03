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

#ifndef PICOUI_LABEL_H
#define PICOUI_LABEL_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_label;
struct picoui_image_source;

struct picoui_label_props {
    const char *id;
    const char *text;
    const struct picoui_font *font;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    unsigned int bg_color;
    unsigned int text_color;
    unsigned int border_color;
    int radius;
    int padding;
    int transparent;
    enum picoui_align align;
    struct picoui_image_source *background_source;
};

/**
 * @brief Create label widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_label *picoui_label_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create label widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_label *picoui_label_create_with_props(struct picoui_window *parent,
                                                    const struct picoui_label_props *props);

/**
 * @brief Set text of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_text(struct picoui_label *label, const char *text);

/**
 * @brief Get text of label widget
 *
 * @param[out] label Label widget instance
 */

const char *picoui_label_get_text(struct picoui_label *label);

/**
 * @brief Set font of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_font(struct picoui_label *label, const struct picoui_font *font);

/**
 * @brief Set text color of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_text_color(struct picoui_label *label, unsigned int rgb);

/**
 * @brief Get text color of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_label_get_text_color(struct picoui_label *label, unsigned int *rgb);

/**
 * @brief Set bg color of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_bg_color(struct picoui_label *label, unsigned int rgb);

/**
 * @brief Get bg color of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return The property value, negative on error
 */

int picoui_label_get_bg_color(struct picoui_label *label, unsigned int *rgb);

/**
 * @brief Set transparent of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_transparent(struct picoui_label *label, int transparent);

/**
 * @brief Get transparent of label widget
 *
 * @param[out] label Label widget instance
 * @param[in] transparent transparent
 * @return The property value, negative on error
 */

int picoui_label_get_transparent(struct picoui_label *label, int *transparent);

/**
 * @brief Set align of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_align(struct picoui_label *label, enum picoui_align align);

/**
 * @brief Get align of label widget
 *
 * @param[out] label Label widget instance
 * @param[out] align align
 * @return The property value, negative on error
 */

int picoui_label_get_align(struct picoui_label *label, enum picoui_align *align);

/**
 * @brief Set background source of label widget
 *
 * @param[in] label Label widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_label_set_background_source(struct picoui_label *label,
                                       struct picoui_image_source *source);

#endif
