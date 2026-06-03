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

#ifndef PICOUI_COMBO_BOX_H
#define PICOUI_COMBO_BOX_H

#include "picoui/widget.h"

struct picoui_window;
struct picoui_combo_box;
struct picoui_image_source;

struct picoui_combo_box_props {
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
 * @brief Create combo box widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_combo_box *picoui_combo_box_create(struct picoui_window *parent, const char *id);

/**
 * @brief Create combo box widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_combo_box *picoui_combo_box_create_with_props(struct picoui_window *parent,
                                                            const struct picoui_combo_box_props *props);

/**
 * @brief combo box add item
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_add_item(struct picoui_combo_box *combo_box, const char *id, const char *text);

/**
 * @brief Set select item of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_select_item(struct picoui_combo_box *combo_box, int index);

/**
 * @brief Set selected index of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_selected_index(struct picoui_combo_box *combo_box, int index);

/**
 * @brief Get select item of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @return The property value, negative on error
 */

int picoui_combo_box_get_select_item(const struct picoui_combo_box *combo_box);

/**
 * @brief Get selected index of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @return The property value, negative on error
 */

int picoui_combo_box_get_selected_index(const struct picoui_combo_box *combo_box);

/**
 * @brief Get text of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] index Index
 */

const char *picoui_combo_box_get_text(const struct picoui_combo_box *combo_box, int index);

/**
 * @brief Set static items of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] item_ids item ids
 * @param[in] texts texts
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_static_items(struct picoui_combo_box *combo_box,
                                      const char *const *item_ids,
                                      const char *const *texts,
                                      int item_count);

/**
 * @brief combo box is open
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] is_open is open
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_is_open(const struct picoui_combo_box *combo_box, int *is_open);

/**
 * @brief Set text color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_text_color(struct picoui_combo_box *combo_box, unsigned int rgb);

/**
 * @brief Set background color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_background_color(struct picoui_combo_box *combo_box, unsigned int rgb);

/**
 * @brief Set bg color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_bg_color(struct picoui_combo_box *combo_box, unsigned int rgb);

/**
 * @brief Set frame color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_frame_color(struct picoui_combo_box *combo_box, unsigned int rgb);

/**
 * @brief Set select color of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_select_color(struct picoui_combo_box *combo_box, unsigned int rgb);

/**
 * @brief Set item max of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] item_max item max
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_item_max(struct picoui_combo_box *combo_box, int item_max);

/**
 * @brief Set dropdown image of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_dropdown_image(struct picoui_combo_box *combo_box,
                                        struct picoui_image_source *source);

/**
 * @brief Set dropdown source of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_combo_box_set_dropdown_source(struct picoui_combo_box *combo_box,
                                         struct picoui_image_source *source);

/**
 * @brief Set on selected of combo box widget
 *
 * @param[in] combo_box Combo box widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void picoui_combo_box_set_on_selected(struct picoui_combo_box *combo_box,
                                      void (*callback)(struct picoui_combo_box *combo_box,
                                                       int index,
                                                       void *user_data),
                                      void *user_data);

#endif
