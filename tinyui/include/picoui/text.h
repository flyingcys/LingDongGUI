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

#ifndef PICOUI_TEXT_H
#define PICOUI_TEXT_H

#include "../widget.h"

struct picoui_window;
struct picoui_image_source;
struct picoui_text;

struct picoui_text_props {
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
};

/**
 * @brief Create text widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_text *picoui_text_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create text widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_text *picoui_text_create_with_props(struct picoui_window *parent,
                                                  const struct picoui_text_props *props);

/**
 * @brief Set text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_text(struct picoui_text *text, const char *value);

/**
 * @brief Set static text of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] value Value
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_static_text(struct picoui_text *text, const char *value);

/**
 * @brief Set font of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_font(struct picoui_text *text, const struct picoui_font *font);

/**
 * @brief Set transparent of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_transparent(struct picoui_text *text, int transparent);

/**
 * @brief Set text color of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_text_color(struct picoui_text *text, unsigned int rgb);

/**
 * @brief Set bg color of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_bg_color(struct picoui_text *text, unsigned int rgb);

/**
 * @brief Set background source of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_background_source(struct picoui_text *text,
                                      struct picoui_image_source *source);

/**
 * @brief Set consumed font of text widget
 *
 * @param[in] text Text widget instance
 * @param[in] font font
 * @return 0 on success, -1 on failure
 */

int picoui_text_set_consumed_font(struct picoui_text *text, const struct picoui_font *font);

/**
 * @brief text scroll seek
 *
 * @param[in] text Text widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int picoui_text_scroll_seek(struct picoui_text *text, int offset);

/**
 * @brief text scroll move
 *
 * @param[in] text Text widget instance
 * @param[in] move_value move value
 * @return 0 on success, -1 on failure
 */

int picoui_text_scroll_move(struct picoui_text *text, int move_value);

#endif
