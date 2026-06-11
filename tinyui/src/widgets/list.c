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
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldList.h"

#include <stdlib.h>

static ldColor picoui_list_rgb_to_ld_color(unsigned int rgb)
{
    return __RGB((rgb >> 16) & 0xFFU, (rgb >> 8) & 0xFFU, rgb & 0xFFU);
}

static arm_2d_align_t picoui_list_map_align(enum picoui_align align)
{
    switch (align) {
    case PICOUI_ALIGN_START:
        return ARM_2D_ALIGN_LEFT;
    case PICOUI_ALIGN_END:
        return ARM_2D_ALIGN_RIGHT;
    case PICOUI_ALIGN_CENTER:
        return ARM_2D_ALIGN_CENTRE;
    default:
        return ARM_2D_ALIGN_CENTRE;
    }
}

static struct picoui_backend_widget *picoui_list_backend(const struct picoui_list *list)
{
    if (list == 0 || list->widget.backend_widget == 0) {
        return 0;
    }

    return (struct picoui_backend_widget *)list->widget.backend_widget;
}

static ldList_t *picoui_list_ld_widget(const struct picoui_list *list)
{
    struct picoui_backend_widget *backend = picoui_list_backend(list);

    if (backend == 0 ||
        backend->kind != PICOUI_BACKEND_WIDGET_LIST ||
        backend->ld_widget == 0) {
        return 0;
    }

    return (ldList_t *)backend->ld_widget;
}

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
    struct picoui_backend_widget *backend;
    struct picoui_backend_widget *parent_backend;
    struct picoui_backend_app_state *app_state;
    ldList_t *ld_list;
    uint16_t name_id;

    if (parent == 0 || id == 0 || parent->backend_widget == 0) {
        return 0;
    }

    parent_backend = (struct picoui_backend_widget *)parent->backend_widget;
    app_state = picoui_runtime_bridge_backend_state_from_parent(parent_backend);
    if (parent_backend->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    list = calloc(1, sizeof(*list));
    if (list == 0) {
        return 0;
    }

    backend = calloc(1, sizeof(*backend));
    if (backend == 0) {
        free(list);
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent_backend);
    if (name_id == 0) {
        free(backend);
        free(list);
        return 0;
    }

    ld_list = ldList_init(app_state->ld_scene,
                          NULL,
                          name_id,
                          parent_backend->ld_name_id,
                          0,
                          0,
                          220,
                          96);
    if (ld_list == 0) {
        free(backend);
        free(list);
        return 0;
    }

    ldListSetSelectItem(ld_list, -1);
    if (picoui_backend_widget_init_child(backend,
                                         parent_backend,
                                         PICOUI_BACKEND_WIDGET_LIST,
                                         id,
                                         parent_backend->theme) != 0) {
        ldList_depose(app_state->ld_scene, ld_list);
        free(backend);
        free(list);
        return 0;
    }
    backend->ld_widget = ld_list;
    backend->ld_name_id = name_id;
    backend->value = -1;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent_backend, backend) != 0) {
        ldList_depose(app_state->ld_scene, ld_list);
        free(backend);
        free(list);
        return 0;
    }

    list->widget.backend_widget = backend;
    list->id = id;
    list->selected_index = -1;
    list->widget.visible = 1;
    list->widget.enabled = 1;
    if (picoui_backend_widget_bind_host(list->widget.backend_widget, &list->widget) != 0) {
        (void)picoui_backend_widget_detach_from_parent(list->widget.backend_widget);
        ldList_depose(app_state->ld_scene, ld_list);
        free(backend);
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
    ldList_t *ld_list;
    int index;
    int next_count;

    if (list == 0 || id == 0 || text == 0 || list->item_count >= PICOUI_LIST_MAX_ITEMS) {
        return -1;
    }

    backend = picoui_list_backend(list);
    ld_list = picoui_list_ld_widget(list);
    if (backend == 0 || ld_list == 0) {
        return -1;
    }

    index = list->item_count;
    next_count = index + 1;
    list->backend_item_ids[index] = id;
    list->backend_item_texts[index] = (const unsigned char *)text;
    ldListSetText(ld_list, list->backend_item_texts, (uint8_t)next_count, NULL);
    backend->list_item_ids[index] = id;
    backend->list_item_count = next_count;

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

int picoui_list_set_item_height(struct picoui_list *list, int item_height)
{
    ldList_t *ld_list;

    if (list == 0 || item_height <= 0 || item_height > 255) {
        return -1;
    }

    ld_list = picoui_list_ld_widget(list);
    if (ld_list == 0) {
        return -1;
    }

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

int picoui_list_set_padding_group(struct picoui_list *list, int top, int bottom, int left, int right)
{
    ldList_t *ld_list;

    if (list == 0 ||
        top < 0 || top > 255 ||
        bottom < 0 || bottom > 255 ||
        left < 0 || left > 255 ||
        right < 0 || right > 255) {
        return -1;
    }

    ld_list = picoui_list_ld_widget(list);
    if (ld_list == 0) {
        return -1;
    }

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

int picoui_list_set_margin_group(struct picoui_list *list, int top, int bottom, int left, int right)
{
    ldList_t *ld_list;

    if (list == 0 ||
        top < 0 || top > 255 ||
        bottom < 0 || bottom > 255 ||
        left < 0 || left > 255 ||
        right < 0 || right > 255) {
        return -1;
    }

    ld_list = picoui_list_ld_widget(list);
    if (ld_list == 0) {
        return -1;
    }

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

int picoui_list_set_text_color(struct picoui_list *list, unsigned int rgb)
{
    ldList_t *ld_list = picoui_list_ld_widget(list);

    if (list == 0 || ld_list == 0) {
        return -1;
    }

    ldListSetTextColor(ld_list, picoui_list_rgb_to_ld_color(rgb));
    return picoui_widget_set_text_color(&list->widget, rgb);
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
    ldList_t *ld_list = picoui_list_ld_widget(list);

    if (list == 0 || ld_list == 0) {
        return -1;
    }

    ldListSetBackgroundColor(ld_list, picoui_list_rgb_to_ld_color(rgb));
    return picoui_widget_set_bg_color(&list->widget, rgb);
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
    ldList_t *ld_list = picoui_list_ld_widget(list);

    if (list == 0 || ld_list == 0) {
        return -1;
    }

    ldListSetSelectColor(ld_list, picoui_list_rgb_to_ld_color(rgb));
    return picoui_widget_set_border_color(&list->widget, rgb);
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
    ldList_t *ld_list;

    if (list == 0 ||
        (align != PICOUI_ALIGN_START &&
         align != PICOUI_ALIGN_CENTER &&
         align != PICOUI_ALIGN_END)) {
        return -1;
    }

    ld_list = picoui_list_ld_widget(list);
    if (ld_list == 0) {
        return -1;
    }

    ldListSetAlign(ld_list, picoui_list_map_align(align));
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

int picoui_list_set_item_widget(struct picoui_list *list,
                                int index,
                                struct picoui_widget *item_widget)
{
    struct picoui_backend_widget *list_backend;
    struct picoui_backend_widget *item_backend;
    ldList_t *ld_list;
    ldBase_t *ld_child;

    if (list == 0 || item_widget == 0 || item_widget->backend_widget == 0) {
        return -1;
    }

    list_backend = picoui_list_backend(list);
    item_backend = (struct picoui_backend_widget *)item_widget->backend_widget;
    ld_list = picoui_list_ld_widget(list);
    if (list_backend == 0 ||
        item_backend == 0 ||
        ld_list == 0 ||
        item_backend->ld_widget == 0 ||
        item_backend->kind == PICOUI_BACKEND_WIDGET_WINDOW ||
        item_backend->kind == PICOUI_BACKEND_WIDGET_BACKGROUND ||
        item_backend->owner != list_backend->owner ||
        index < 0 ||
        index >= list->item_count) {
        return -1;
    }

    ld_child = (ldBase_t *)item_backend->ld_widget;
    if (item_backend->parent != NULL && item_backend->parent != list_backend) {
        if (picoui_widget_backend_detach(item_backend) != 0) {
            return -1;
        }
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_child);
    } else if (item_backend->parent == list_backend) {
        ldBaseNodeRemove((arm_2d_control_node_t *)ld_child);
        if (picoui_widget_backend_detach(item_backend) != 0) {
            return -1;
        }
    }

    if (picoui_backend_widget_attach_child(list_backend, item_backend) != 0) {
        return -1;
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

int picoui_list_set_selected_index(struct picoui_list *list, int index)
{
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;

    if (list == 0 || index < 0 || index >= list->item_count) {
        return -1;
    }

    backend = picoui_list_backend(list);
    ld_list = picoui_list_ld_widget(list);
    if (backend == 0 || ld_list == 0) {
        return -1;
    }

    ldListSetSelectItem(ld_list, (int8_t)index);
    backend->value = index;
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
    struct picoui_backend_widget *backend;
    ldList_t *ld_list;
    int selected_index;

    if (list == 0) {
        return -1;
    }

    backend = picoui_list_backend(list);
    ld_list = picoui_list_ld_widget(list);
    if (backend == 0 || ld_list == 0) {
        return list->selected_index;
    }

    selected_index = ldListGetSelectItem(ld_list);
    if (selected_index < -1 || selected_index >= list->item_count) {
        return list->selected_index;
    }

    ((struct picoui_list *)list)->selected_index = selected_index;
    backend->value = selected_index;
    return selected_index;
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
