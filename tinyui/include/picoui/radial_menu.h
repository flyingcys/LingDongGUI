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

#ifndef PICOUI_RADIAL_MENU_H
#define PICOUI_RADIAL_MENU_H

struct picoui_widget;
struct picoui_radial_menu;
struct picoui_image_source;

struct picoui_radial_menu_props {
    const char *id;
    const char *style_class;
    void *user_data;
    int width;
    int height;
    int x_axis;
    int y_axis;
    int item_max;
    int default_index;
};

/**
 * @brief Create radial menu widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_radial_menu *picoui_radial_menu_create(struct picoui_widget *parent, const char *id);

/**
 * @brief Create radial menu widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_radial_menu *picoui_radial_menu_create_with_props(
    struct picoui_widget *parent,
    const struct picoui_radial_menu_props *props
);

/**
 * @brief radial menu init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_radial_menu *picoui_radial_menu_init(struct picoui_widget *parent, const char *id);

/**
 * @brief radial menu add item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_add_item(struct picoui_radial_menu *radial_menu, const char *id);

/**
 * @brief radial menu add item with source
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_add_item_with_source(struct picoui_radial_menu *radial_menu,
                                            const char *id,
                                            struct picoui_image_source *source);

/**
 * @brief radial menu add item with image
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_add_item_with_image(struct picoui_radial_menu *radial_menu,
                                           const char *id,
                                           struct picoui_image_source *source);

/**
 * @brief Set selected index of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_set_selected_index(struct picoui_radial_menu *radial_menu, int index);

/**
 * @brief Get selected index of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @return The property value, negative on error
 */

int picoui_radial_menu_get_selected_index(const struct picoui_radial_menu *radial_menu);

/**
 * @brief radial menu offset selection
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_offset_selection(struct picoui_radial_menu *radial_menu, int offset);

/**
 * @brief Set default item of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_set_default_item(struct picoui_radial_menu *radial_menu, int index);

/**
 * @brief radial menu click item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_click_item(struct picoui_radial_menu *radial_menu, int index);

/**
 * @brief Set click item of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_set_click_item(struct picoui_radial_menu *radial_menu, int index);

/**
 * @brief radial menu offset item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int picoui_radial_menu_offset_item(struct picoui_radial_menu *radial_menu, int offset);

/**
 * @brief Set on selected of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void picoui_radial_menu_set_on_selected(struct picoui_radial_menu *radial_menu,
                                        void (*callback)(struct picoui_radial_menu *radial_menu,
                                                         int index,
                                                         void *user_data),
                                        void *user_data);

#endif
