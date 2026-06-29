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
#include "widgets/list.h"
#include "core/widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldList.h"

#define TINYUI_HIDDEN __attribute__((visibility("hidden")))



static void tinyui_list_rollback(struct tinyui_list *list)
{
    if (list == 0) {
        return;
    }
    if (list->widget.ld_widget != 0) {
        tinyui_widget_destroy_common(&list->widget);
    } else {
        ldFree(list);
    }
}

static void *tinyui_list_ld_init(void *ctx,
                                 struct ld_scene_t *scene,
                                 uint16_t name_id,
                                 uint16_t parent_name_id)
{
    ldList_t *ld_list;

    (void)ctx;
    ld_list = ldList_init(scene, NULL, name_id, parent_name_id, 0, 0, 220, 96);
    if (ld_list == 0) {
        return 0;
    }
    ldListSetSelectItem(ld_list, -1);
    return ld_list;
}

/**
 * @brief Create list widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_list *tinyui_list_create(struct tinyui_widget *parent, const char *id)
{
    struct tinyui_list *list;

    if (parent == 0 || id == 0 || parent->ld_widget == 0) {
        return 0;
    }
    list = (struct tinyui_list *)tinyui_widget_create_leaf(parent,
                                                           TINYUI_BACKEND_WIDGET_LIST,
                                                           tinyui_list_ld_init,
                                                           0,
                                                           sizeof(*list));
    if (list == 0) {
        return 0;
    }
    list->widget.value      = -1;
    list->id = id;
    list->selected_index = -1;

    return list;
}

/**
 * @brief Create list widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_list *tinyui_list_create_with_props(struct tinyui_widget *parent,
                                                  const struct tinyui_list_props *props)
{
    struct tinyui_list *list;

    if (props == 0) {
        return 0;
    }

    list = tinyui_list_create(parent, props->id);
    if (list == 0) {
        return 0;
    }

    if (tinyui_widget_set_style_class(&list->widget, props->style_class) != 0 ||
        tinyui_widget_set_user_data(&list->widget, props->user_data) != 0) {
        tinyui_list_rollback(list);
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

int tinyui_list_add_item(struct tinyui_list *list, const char *id, const char *text)
{
    ldList_t *ld_list;
    int index;
    int next_count;

    if (list == 0 || id == 0 || text == 0 || list->item_count >= TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (list->widget.ld_widget == 0 || list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }
    ld_list = (ldList_t *)list->widget.ld_widget;

    index = list->item_count;
    next_count = index + 1;
    list->backend_item_ids[index] = id;
    list->backend_item_texts[index] = (const unsigned char *)text;
    ldListSetText(ld_list,
                  list->backend_item_texts,
                  (uint8_t)next_count,
                  tinyui_resolve_ld_font(0, 12));
    list->widget.list_item_count = (uint16_t)next_count;

    list->items[index].id = id;
    list->items[index].text = text;
    list->item_count = next_count;
    return 0;
}

/**
 * @brief Set item height of list widget
 *
 * @param[in] list List widget instance
 * @param[in] item_height item height
 * @return -1 on failure
 */

int tinyui_list_set_item_height(struct tinyui_list *list, int item_height)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        item_height <= 0 || item_height > 255) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetItemHeight(ld_list, (uint8_t)item_height);
    return 0;
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

int tinyui_list_set_padding_group(struct tinyui_list *list, int top, int bottom, int left, int right)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        top < 0 || top > 255 ||
        bottom < 0 || bottom > 255 ||
        left < 0 || left > 255 ||
        right < 0 || right > 255) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetPadding(ld_list, (uint8_t)top, (uint8_t)bottom, (uint8_t)left, (uint8_t)right);
    return 0;
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

int tinyui_list_set_margin_group(struct tinyui_list *list, int top, int bottom, int left, int right)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        top < 0 || top > 255 ||
        bottom < 0 || bottom > 255 ||
        left < 0 || left > 255 ||
        right < 0 || right > 255) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetMargin(ld_list, (uint8_t)top, (uint8_t)bottom, (uint8_t)left, (uint8_t)right);
    return 0;
}

/**
 * @brief Set text color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_list_set_text_color(struct tinyui_list *list, unsigned int rgb)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetTextColor(ld_list, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return tinyui_widget_set_text_color(&list->widget, rgb);
}

/**
 * @brief Set bg color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_list_set_bg_color(struct tinyui_list *list, unsigned int rgb)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetBackgroundColor(ld_list, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return tinyui_widget_set_bg_color(&list->widget, rgb);
}

/**
 * @brief Set select color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_list_set_select_color(struct tinyui_list *list, unsigned int rgb)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetSelectColor(ld_list, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return tinyui_widget_set_border_color(&list->widget, rgb);
}

/**
 * @brief Set align of list widget
 *
 * @param[in] list List widget instance
 * @param[in] align align
 * @return -1 on failure
 */

int tinyui_list_set_align(struct tinyui_list *list, enum tinyui_align align)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        (align != TINYUI_ALIGN_START &&
         align != TINYUI_ALIGN_CENTER &&
         align != TINYUI_ALIGN_END)) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetAlign(ld_list, (arm_2d_align_t)tinyui_align_to_arm2d(align));
    return 0;
}

/**
 * @brief Set item widget of list widget
 *
 * @param[in] list List widget instance
 * @param[in] index Index
 * @param[in] item_widget item widget
 * @return -1 on failure
 */

int tinyui_list_set_item_widget(struct tinyui_list *list,
                                int index,
                                struct tinyui_widget *item_widget)
{
    ldList_t *ld_list;
    ldBase_t *ld_child;

    if (list == 0 || item_widget == 0 || item_widget->ld_widget == 0) {
        return -1;
    }

    if (list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        item_widget->kind == TINYUI_BACKEND_WIDGET_WINDOW ||
        item_widget->kind == TINYUI_BACKEND_WIDGET_BACKGROUND ||
        item_widget->owner != list->widget.owner ||
        index < 0 ||
        index >= list->item_count) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ld_child = (ldBase_t *)item_widget->ld_widget;
    if (ldBaseGetParent(ld_child) != NULL) {
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_child);
    }

    ldBaseNodeAdd((arm_2d_control_node_t *)ld_list, (arm_2d_control_node_t *)ld_child);
    ldListSetItemWidget(ld_list, (uint8_t)index, ld_child);
    return 0;
}

/**
 * @brief Set selected index of list widget
 *
 * @param[in] list List widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_list_set_selected_index(struct tinyui_list *list, int index)
{
    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        index < 0 || index >= list->item_count) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetSelectItem(ld_list, (int8_t)index);
    list->widget.value = index;
    list->selected_index = index;
    return 0;
}

/**
 * @brief Get selected index of list widget
 *
 * @param[in] list List widget instance
 * @return -1 on failure
 */

int tinyui_list_get_selected_index(const struct tinyui_list *list)
{
    ldList_t *ld_list;
    int selected_index;

    if (list == 0) {
        return -1;
    }

    if (list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return list->selected_index;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    selected_index = ldListGetSelectItem(ld_list);
    if (selected_index < -1 || selected_index >= list->item_count) {
        return list->selected_index;
    }

    ((struct tinyui_list *)list)->selected_index = selected_index;
    ((struct tinyui_list *)list)->widget.value = selected_index;
    return selected_index;
}

/**
 * @brief Set on selected of list widget
 *
 * @param[in] list List widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void tinyui_list_set_on_selected(struct tinyui_list *list,
                                 void (*callback)(struct tinyui_list *list,
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

TINYUI_HIDDEN int tinyui_list_set_selected_index_ld(void *widget_ptr, int index)
{
    struct tinyui_widget *widget = widget_ptr;
    struct tinyui_list *list;
    ldList_t *ld_list;

    if (widget == 0 || widget->kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    list = (struct tinyui_list *)widget;
    if (index < 0 || index >= list->item_count) {
        return -1;
    }
    if (list->widget.ld_widget == 0) {
        return -1;
    }
    ld_list = (ldList_t *)list->widget.ld_widget;

    ldListSetSelectItem(ld_list, (int8_t)index);
    widget->value = index;
    return 0;
}

TINYUI_HIDDEN int tinyui_list_get_selected_index_ld(void *widget_ptr)
{
    struct tinyui_widget *widget = widget_ptr;
    struct tinyui_list *list;
    ldList_t *ld_list;
    int selected_index;

    if (widget == 0 || widget->kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    list = (struct tinyui_list *)widget;
    if (list->widget.ld_widget == 0) {
        return -1;
    }
    ld_list = (ldList_t *)list->widget.ld_widget;

    selected_index = ldListGetSelectItem(ld_list);
    if (selected_index < -1 || selected_index >= list->item_count) {
        return -1;
    }
    return selected_index;
}

TINYUI_HIDDEN int tinyui_list_sync_selected_index(struct tinyui_list *list,
                                                          int *selected_index_out)
{
    int selected_index;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    selected_index = tinyui_list_get_selected_index(list);
    if (selected_index < -1 || selected_index >= list->item_count) {
        return -1;
    }

    list->selected_index = selected_index;
    list->widget.value = selected_index;
    if (selected_index_out != 0) {
        *selected_index_out = selected_index;
    }
    return 0;
}
