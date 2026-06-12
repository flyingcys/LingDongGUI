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

#ifndef PICOUI_LIST_H
#define PICOUI_LIST_H

#include "../widget.h"

struct picoui_widget;
struct picoui_list;

struct picoui_list_props {
    const char *id;
    const char *style_class;
    void *user_data;
};

/**
 * @brief Create list widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_list *picoui_list_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create list widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_list *picoui_list_create_with_props(struct picoui_widget *parent,
                                                  const struct picoui_list_props *props);

/**
 * @brief list add item
 *
 * @param[in] list List widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_list_add_item(struct picoui_list *list, const char *id, const char *text);

/**
 * @brief Set item height of list widget
 *
 * @param[in] list List widget instance
 * @param[in] item_height item height
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_item_height(struct picoui_list *list, int item_height);

/**
 * @brief Set padding group of list widget
 *
 * @param[in] list List widget instance
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_padding_group(struct picoui_list *list, int top, int bottom, int left, int right);

/**
 * @brief Set margin group of list widget
 *
 * @param[in] list List widget instance
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_margin_group(struct picoui_list *list, int top, int bottom, int left, int right);

/**
 * @brief Set text color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_text_color(struct picoui_list *list, unsigned int rgb);

/**
 * @brief Set bg color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_bg_color(struct picoui_list *list, unsigned int rgb);

/**
 * @brief Set select color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_select_color(struct picoui_list *list, unsigned int rgb);

/**
 * @brief Set align of list widget
 *
 * @param[in] list List widget instance
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_align(struct picoui_list *list, enum picoui_align align);

/**
 * @brief Set item widget of list widget
 *
 * @param[in] list List widget instance
 * @param[in] index Index
 * @param[in] item_widget item widget
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_item_widget(struct picoui_list *list,
                                int index,
                                struct picoui_widget *item_widget);

/**
 * @brief Set selected index of list widget
 *
 * @param[in] list List widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_selected_index(struct picoui_list *list, int index);

/**
 * @brief Get selected index of list widget
 *
 * @param[in] list List widget instance
 * @return The property value, negative on error
 */

int picoui_list_get_selected_index(const struct picoui_list *list);

/**
 * @brief Set on selected of list widget
 *
 * @param[in] list List widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void picoui_list_set_on_selected(struct picoui_list *list,
                                 void (*callback)(struct picoui_list *list,
                                                  int index,
                                                  void *user_data),
                                 void *user_data);

#endif
