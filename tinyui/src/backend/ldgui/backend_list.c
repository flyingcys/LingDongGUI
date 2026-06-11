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

#include "backend.h"
#include "internal.h"
#include "ldList.h"

/**
 * @brief Set items of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_ids item ids
 * @param[in] items items
 * @param[in] item_count item count
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_items(void *backend_widget,
                                  const char *const *item_ids,
                                  const unsigned char *const *items,
                                  int item_count)
{
    (void)backend_widget;
    (void)item_ids;
    (void)items;
    (void)item_count;
    return -1;
}

/**
 * @brief Set item height of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] item_height item height
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_item_height(void *backend_widget, int item_height)
{
    (void)backend_widget;
    (void)item_height;
    return -1;
}

/**
 * @brief Set padding group of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_padding_group(void *backend_widget,
                                          int top,
                                          int bottom,
                                          int left,
                                          int right)
{
    (void)backend_widget;
    (void)top;
    (void)bottom;
    (void)left;
    (void)right;
    return -1;
}

/**
 * @brief Set margin group of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_margin_group(void *backend_widget,
                                         int top,
                                         int bottom,
                                         int left,
                                         int right)
{
    (void)backend_widget;
    (void)top;
    (void)bottom;
    (void)left;
    (void)right;
    return -1;
}

/**
 * @brief Set text color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_text_color(void *backend_widget, unsigned int rgb)
{
    (void)backend_widget;
    (void)rgb;
    return -1;
}

/**
 * @brief Set bg color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_bg_color(void *backend_widget, unsigned int rgb)
{
    (void)backend_widget;
    (void)rgb;
    return -1;
}

/**
 * @brief Set select color of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_select_color(void *backend_widget, unsigned int rgb)
{
    (void)backend_widget;
    (void)rgb;
    return -1;
}

/**
 * @brief Set align of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] align align
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_align(void *backend_widget, enum picoui_align align)
{
    (void)backend_widget;
    (void)align;
    return -1;
}

/**
 * @brief Set item widget of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @param[in] item_widget_backend item widget backend
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_item_widget(void *backend_widget, int index, void *item_widget_backend)
{
    (void)backend_widget;
    (void)index;
    (void)item_widget_backend;
    return -1;
}

/**
 * @brief Set selected index of list backend
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0 ||
        index < 0 ||
        index >= PICOUI_BACKEND_LIST_MAX_ITEMS) {
        return -1;
    }

    ldListSetSelectItem((ldList_t *)widget->ld_widget, (int8_t)index);
    widget->value = index;
    return 0;
}

/**
 * @brief Get selected index from list backend
 *
 * @param[in] backend_widget backend widget
 * @return -1 on failure
 */

int picoui_backend_list_get_selected_index(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == 0 ||
        widget->kind != PICOUI_BACKEND_WIDGET_LIST ||
        widget->ld_widget == 0) {
        return -1;
    }

    return ldListGetSelectItem((ldList_t *)widget->ld_widget);
}

/**
 * @brief list: sync selected index
 *
 * @param[in] list List widget instance
 * @param[in] selected_index_out selected index out
 * @return 0 on success, -1 on failure
 */

int picoui_backend_list_sync_selected_index(struct picoui_list *list, int *selected_index_out)
{
    struct picoui_backend_widget *backend;
    int selected_index;

    if (list == NULL || list->widget.backend_widget == NULL) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    selected_index = picoui_backend_list_get_selected_index(backend);
    if (selected_index < -1) {
        return -1;
    }
    if (selected_index >= list->item_count) {
        return -1;
    }

    list->selected_index = selected_index;
    backend->value = selected_index;
    if (selected_index_out != NULL) {
        *selected_index_out = selected_index;
    }
    return 0;
}
