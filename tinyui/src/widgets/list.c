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
#include "internal/widget_legacy.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldList.h"


static struct tinyui_list *tinyui_list_as_list(tinyui_obj_t *obj)
{
    struct tinyui_widget *w = (struct tinyui_widget *)(void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_LIST)) {
        return 0;
    }
    return (struct tinyui_list *)w;
}

static const struct tinyui_list *tinyui_list_as_list_const(const tinyui_obj_t *obj)
{
    const struct tinyui_widget *w = (const struct tinyui_widget *)(const void *)obj;
    if (w == 0 || !tinyui_runtime_internal_widget_is_kind(w, TINYUI_BACKEND_WIDGET_LIST)) {
        return 0;
    }
    return (const struct tinyui_list *)w;
}

#define TINYUI_HIDDEN __attribute__((visibility("hidden")))



static void tinyui_list_rollback(struct tinyui_list *list)
{
    if (list == 0) {
        return;
    }
    if (list->widget.ld_widget != 0) {
        tinyui_runtime_internal_widget_destroy_common(&list->widget);
    } else {
        ldFree(list);
    }
}

static void *tinyui_runtime_internal_list_ld_init(void *ctx,
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

tinyui_obj_t *tinyui_list_create(tinyui_obj_t *parent)
{
    struct tinyui_widget *parent_w = (struct tinyui_widget *)(void *)parent;
    const char *id = "list";
    if (parent_w == 0) { return 0; }

    struct tinyui_list *list;

    if (parent_w == 0 || id == 0 || parent_w->ld_widget == 0) {
        return 0;
    }
    list = (struct tinyui_list *)tinyui_runtime_internal_widget_create_leaf(parent_w,
                                                           TINYUI_BACKEND_WIDGET_LIST,
                                                           tinyui_runtime_internal_list_ld_init,
                                                           0,
                                                           sizeof(*list));
    if (list == 0) {
        return 0;
    }
    list->widget.value      = -1;
    list->id = id;
    list->selected_index = -1;

    return (tinyui_obj_t *)list;
}

/**
 * @brief Create list widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

tinyui_obj_t *tinyui_list_create_with_props(tinyui_obj_t *parent,
                                             const tinyui_list_props_t *props)
{
    tinyui_obj_t *obj;
    struct tinyui_list *list;

    if (props == 0) {
        return tinyui_list_create(parent);
    }

    obj = tinyui_list_create(parent);
    if (obj == 0) {
        return 0;
    }
    list = (struct tinyui_list *)(void *)obj;

    if ((props->fields & TINYUI_LIST_FIELD_ID) != 0) {
        /* id=0 means runtime auto-alloc; non-zero reserved for host name_id path. */
        (void)props->id;
    }
    if ((props->fields & TINYUI_LIST_FIELD_USER_DATA) != 0) {
    if (tinyui_runtime_internal_widget_set_user_data(&list->widget, props->user_data) != 0) {
        tinyui_list_rollback(list);
        return 0;
    }
    }
    if ((props->fields & TINYUI_LIST_FIELD_STYLE_CLASS) != 0) {
    if (tinyui_runtime_internal_widget_set_style_class(&list->widget, props->style_class) != 0) {
        tinyui_list_rollback(list);
        return 0;
    }
    }

    return obj;
}


/**
 * @brief list add item
 *
 * @param[in] list List widget instance
 * @param[in] id Widget identifier string
 * @param[in] text Text widget instance
 * @return 0 on success, -1 on failure
 */

int tinyui_list_add_item(tinyui_obj_t *list_obj, const char *id, const char *text)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

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

int tinyui_list_set_item_height(tinyui_obj_t *list_obj, int item_height)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

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

int tinyui_list_set_padding_group(tinyui_obj_t *list_obj, int top, int bottom, int left, int right)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

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

int tinyui_list_set_margin_group(tinyui_obj_t *list_obj, int top, int bottom, int left, int right)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

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

int tinyui_list_set_text_color(tinyui_obj_t *list_obj, unsigned int rgb)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetTextColor(ld_list, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return tinyui_runtime_internal_widget_set_text_color(&list->widget, rgb);
}

/**
 * @brief Set bg color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_list_set_bg_color(tinyui_obj_t *list_obj, unsigned int rgb)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetBackgroundColor(ld_list, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return tinyui_runtime_internal_widget_set_bg_color(&list->widget, rgb);
}

/**
 * @brief Set select color of list widget
 *
 * @param[in] list List widget instance
 * @param[in] rgb RGB color value (0xRRGGBB)
 * @return -1 on failure
 */

int tinyui_list_set_select_color(tinyui_obj_t *list_obj, unsigned int rgb)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

    ldList_t *ld_list;

    if (list == 0 || list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ldListSetSelectColor(ld_list, (ldColor)tinyui_rgb_to_ld_color(rgb));
    return tinyui_runtime_internal_widget_set_border_color(&list->widget, rgb);
}

/**
 * @brief Set align of list widget
 *
 * @param[in] list List widget instance
 * @param[in] align align
 * @return -1 on failure
 */

int tinyui_list_set_align(tinyui_obj_t *list_obj, enum tinyui_align align)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

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

int tinyui_list_set_item_widget(tinyui_obj_t *list_obj, int index, tinyui_obj_t *item_widget)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    struct tinyui_widget *item;
    ldList_t *ld_list;
    ldBase_t *ld_child;

    if (list == 0) { return -1; }

    item = (struct tinyui_widget *)(void *)item_widget;
    if (list == 0 || item == 0 || item->ld_widget == 0) {
        return -1;
    }

    if (list->widget.ld_widget == 0 ||
        list->widget.kind != TINYUI_BACKEND_WIDGET_LIST ||
        item->kind == TINYUI_BACKEND_WIDGET_WINDOW ||
        item->kind == TINYUI_BACKEND_WIDGET_BACKGROUND ||
        item->owner != list->widget.owner ||
        index < 0 ||
        index >= list->item_count) {
        return -1;
    }

    ld_list = (ldList_t *)list->widget.ld_widget;
    ld_child = (ldBase_t *)item->ld_widget;
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

int tinyui_list_set_selected_index(tinyui_obj_t *list_obj, int index)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return -1; }

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

int tinyui_list_get_selected_index(const tinyui_obj_t *list_obj)
{
    const struct tinyui_list *list = tinyui_list_as_list_const(list_obj);
    if (list == 0) { return -1; }

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

void tinyui_list_set_on_selected(tinyui_obj_t *list_obj, void (*callback)(tinyui_obj_t *list,
                                                  int index,
                                                  void *user_data), void *user_data)
{
    struct tinyui_list *list = tinyui_list_as_list(list_obj);
    if (list == 0) { return; }

    if (list == 0) {
        return;
    }

    list->cb = (void (*)(struct tinyui_list *, int, void *))callback;
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
