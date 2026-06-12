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

#ifndef PICOUI_SCROLL_SELECTER_H
#define PICOUI_SCROLL_SELECTER_H

#include "../widget.h"

struct picoui_window;
struct picoui_scroll_selecter;
struct picoui_image_source;

struct picoui_scroll_selecter_props {
    const char *id;
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
 * @brief Create scroll selecter widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_scroll_selecter *picoui_scroll_selecter_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create scroll selecter widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_scroll_selecter *picoui_scroll_selecter_create_with_props(
    struct picoui_window *parent,
    const struct picoui_scroll_selecter_props *props
);

/**
 * @brief Set items of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] item_ids item ids
 * @param[in] texts texts
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_items(struct picoui_scroll_selecter *scroll_selecter,
                                     const char *const *item_ids,
                                     const char *const *texts,
                                     int item_count);

/**
 * @brief scroll selecter add item
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_add_item(struct picoui_scroll_selecter *scroll_selecter,
                                    const char *id,
                                    const char *text);

/**
 * @brief Set select item num of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_select_item_num(struct picoui_scroll_selecter *scroll_selecter, int index);

/**
 * @brief Set selected index of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_selected_index(struct picoui_scroll_selecter *scroll_selecter, int index);

/**
 * @brief Get select item num of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @return The property value, negative on error
 */

int picoui_scroll_selecter_get_select_item_num(const struct picoui_scroll_selecter *scroll_selecter);

/**
 * @brief Get selected index of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @return The property value, negative on error
 */

int picoui_scroll_selecter_get_selected_index(const struct picoui_scroll_selecter *scroll_selecter);

/**
 * @brief Get select text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 */

const char *picoui_scroll_selecter_get_select_text(const struct picoui_scroll_selecter *scroll_selecter);

/**
 * @brief Set text color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_text_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb);

/**
 * @brief Set background color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_background_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb);

/**
 * @brief Set bg color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_bg_color(struct picoui_scroll_selecter *scroll_selecter, unsigned int rgb);

/**
 * @brief Set indicator color of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_indicator_color(struct picoui_scroll_selecter *scroll_selecter,
                                               unsigned int rgb);

/**
 * @brief Set background image of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_background_image(struct picoui_scroll_selecter *scroll_selecter,
                                                struct picoui_image_source *source);

/**
 * @brief Set bg source of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_bg_source(struct picoui_scroll_selecter *scroll_selecter,
                                         struct picoui_image_source *source);

/**
 * @brief Set indicator image of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_indicator_image(struct picoui_scroll_selecter *scroll_selecter,
                                               struct picoui_image_source *source);

/**
 * @brief Set indicator source of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_indicator_source(struct picoui_scroll_selecter *scroll_selecter,
                                                struct picoui_image_source *source);

/**
 * @brief Set transparent of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] transparent transparent
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_transparent(struct picoui_scroll_selecter *scroll_selecter, int transparent);

/**
 * @brief Set speed of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] speed speed
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_speed(struct picoui_scroll_selecter *scroll_selecter, int speed);

/**
 * @brief Set select text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_select_text(struct picoui_scroll_selecter *scroll_selecter, const char *text);

/**
 * @brief Set edit mode of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] is_edit is edit
 * @return 0 on success, -1 on failure
 */

int picoui_scroll_selecter_set_edit_mode(struct picoui_scroll_selecter *scroll_selecter, int is_edit);

/**
 * @brief Get edit mode of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 * @param[in] is_edit is edit
 * @return The property value, negative on error
 */

int picoui_scroll_selecter_get_edit_mode(const struct picoui_scroll_selecter *scroll_selecter, int *is_edit);

/**
 * @brief Get selected text of scroll selecter widget
 *
 * @param[in] scroll_selecter Scroll selecter widget instance
 */

const char *picoui_scroll_selecter_get_selected_text(const struct picoui_scroll_selecter *scroll_selecter);

#endif
