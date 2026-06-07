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

#include "internal.h"
#include "picoui/list.h"
#include "picoui/widget.h"

#include <stdlib.h>

int picoui_native_list_set_selected_index(struct picoui_list *list, int index);
int picoui_native_list_select_index(struct picoui_list *list, int index);
int picoui_backend_list_set_selected_index(void *backend_widget, int index);
int picoui_backend_list_sync_selected_index(struct picoui_list *list, int *selected_index_out);

/**
 * @brief Create list widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_list *picoui_list_create(struct picoui_widget *parent, const char *id)
{
    struct picoui_list *list;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    list = calloc(1, sizeof(*list));
    if (list == 0) {
        return 0;
    }

    list->widget.backend_widget = picoui_backend_create_list(parent->backend_widget, id);
    if (list->widget.backend_widget == 0) {
        free(list);
        return 0;
    }

    list->id = id;
    list->selected_index = -1;
    list->widget.visible = 1;
    list->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(list->widget.backend_widget, &list->widget) != 0) {
        free(list);
        return 0;
    }
    return list;
}

/**
 * @brief Create list widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct picoui_list *picoui_list_create_with_props(struct picoui_widget *parent,
                                                  const struct picoui_list_props *props)
{
    struct picoui_list *list;

    if (props == 0) {
        return 0;
    }

    list = picoui_list_create(parent, props->id);
    if (list == 0) {
        return 0;
    }

    list->user_data = props->user_data;
    if (picoui_widget_set_style_class(&list->widget, props->style_class) != 0 ||
        picoui_widget_set_user_data(&list->widget, props->user_data) != 0) {
        free(list);
        return 0;
    }
    return list;
}

/**
 * @brief list add item
 *
 * @param[in] list List widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int picoui_list_add_item(struct picoui_list *list, const char *id, const char *text)
{
    struct picoui_backend_widget *backend;
    int index;
    int next_count;

    if (list == 0 || id == 0 || text == 0 || list->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    backend = (struct picoui_backend_widget *)list->widget.backend_widget;
    if (backend == 0) {
        return -1;
    }

    index = list->item_count;
    list->backend_item_ids[index] = id;
    list->backend_item_texts[index] = (const unsigned char *)text;
    next_count = index + 1;

    if (picoui_backend_list_set_items(list->widget.backend_widget,
                                      list->backend_item_ids,
                                      list->backend_item_texts,
                                      next_count) != 0) {
        list->backend_item_ids[index] = 0;
        list->backend_item_texts[index] = 0;
        return -1;
    }

    list->items[index].id = id;
    list->items[index].text = text;
    list->item_count = next_count;
    backend->list_item_count = next_count;
    return 0;
}

/**
 * @brief Set item height of list widget
 *
 * @param[in] list List widget instance
 * @param[in] item_height item height
 * @return -1 on failure
 */

int picoui_list_set_item_height(struct picoui_list *list, int item_height)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_item_height(list->widget.backend_widget, item_height);
}

/**
 * @brief Set padding group of list widget
 *
 * @param[in] list List widget instance
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return -1 on failure
 */

int picoui_list_set_padding_group(struct picoui_list *list, int top, int bottom, int left, int right)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_padding_group(list->widget.backend_widget, top, bottom, left, right);
}

/**
 * @brief Set margin group of list widget
 *
 * @param[in] list List widget instance
 * @param[in] top Top padding
 * @param[in] bottom Bottom padding
 * @param[in] left Left padding
 * @param[in] right Right padding
 * @return -1 on failure
 */

int picoui_list_set_margin_group(struct picoui_list *list, int top, int bottom, int left, int right)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_margin_group(list->widget.backend_widget, top, bottom, left, right);
}

/**
 * @brief Set text color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_list_set_text_color(struct picoui_list *list, unsigned int rgb)
{
    if (list == 0 || picoui_widget_set_text_color(&list->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_list_set_text_color(list->widget.backend_widget, rgb);
}

/**
 * @brief Set bg color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_list_set_bg_color(struct picoui_list *list, unsigned int rgb)
{
    if (list == 0 || picoui_widget_set_bg_color(&list->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_list_set_bg_color(list->widget.backend_widget, rgb);
}

/**
 * @brief Set select color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int picoui_list_set_select_color(struct picoui_list *list, unsigned int rgb)
{
    if (list == 0 || picoui_widget_set_border_color(&list->widget, rgb) != 0) {
        return -1;
    }

    return picoui_backend_list_set_select_color(list->widget.backend_widget, rgb);
}

/**
 * @brief Set align of list widget
 *
 * @param[in] list List widget instance
 * @param[in] align align
 * @return -1 on failure
 */

int picoui_list_set_align(struct picoui_list *list, enum picoui_align align)
{
    if (list == 0) {
        return -1;
    }

    return picoui_backend_list_set_align(list->widget.backend_widget, align);
}

/**
 * @brief Set item widget of list widget
 *
 * @param[in] list List widget instance
 * @param[in] index Index
 * @param[in] item_widget item widget
 * @return -1 on failure
 */

int picoui_list_set_item_widget(struct picoui_list *list,
                                int index,
                                struct picoui_widget *item_widget)
{
    if (list == 0 || item_widget == 0 || item_widget->backend_widget == 0) {
        return -1;
    }

    return picoui_backend_list_set_item_widget(list->widget.backend_widget,
                                               index,
                                               item_widget->backend_widget);
}

/**
 * @brief Set selected index of list widget
 *
 * @param[in] list List widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_list_set_selected_index(struct picoui_list *list, int index)
{
    if (list == 0 || index < 0 || index >= list->item_count) {
        return -1;
    }

    if (list->widget.backend_widget == 0) {
        return -1;
    }

    if (picoui_native_list_select_index(list, index) != 0) {
        return -1;
    }
    if (picoui_backend_list_set_selected_index(list->widget.backend_widget, index) != 0) {
        return -1;
    }
    list->selected_index = index;
    return 0;
}

/**
 * @brief Get selected index of list widget
 *
 * @param[in] list List widget instance
 * @return -1 on failure
 */

int picoui_list_get_selected_index(const struct picoui_list *list)
{
    struct picoui_list *mutable_list;
    int selected_index;

    if (list == 0) {
        return -1;
    }

    if (list->widget.backend_widget == 0) {
        return list->selected_index;
    }

    mutable_list = (struct picoui_list *)list;
    if (picoui_backend_list_sync_selected_index(mutable_list, &selected_index) == 0) {
        return selected_index;
    }

    return list->selected_index;
}

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
                                 void *user_data)
{
    if (list == 0) {
        return;
    }

    list->cb = callback;
    list->user_data = user_data;
}
