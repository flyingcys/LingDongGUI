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
#include "runtime_bridge.h"
#include "ldBase.h"
#include "ldRadialMenu.h"

#include <stdlib.h>

extern const arm_2d_tile_t c_tileQuaterArcGRAY8;
extern const arm_2d_tile_t c_tileQuaterArcMask;
extern const arm_2d_tile_t c_tilePointerSecGRAY8;
extern const arm_2d_tile_t c_tilePointerSecMask;

static arm_2d_tile_t *const g_radial_menu_tiles[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
    (arm_2d_tile_t *)&c_tilePointerSecGRAY8,
    (arm_2d_tile_t *)&c_tileQuaterArcGRAY8,
};

static arm_2d_tile_t *const g_radial_menu_masks[] = {
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
    (arm_2d_tile_t *)&c_tilePointerSecMask,
    (arm_2d_tile_t *)&c_tileQuaterArcMask,
};

#define PICOUI_BACKEND_RADIAL_MENU_NATIVE_MAX_ITEMS 5

static ldRadialMenu_t *picoui_backend_radial_menu_get_ld(void *backend_widget)
{
    struct picoui_backend_widget *widget = backend_widget;

    if (widget == NULL || widget->ld_widget == NULL) {
        return NULL;
    }

    return (ldRadialMenu_t *)widget->ld_widget;
}

static bool picoui_backend_radial_menu_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    struct picoui_backend_widget *backend;
    struct picoui_radial_menu *radial_menu;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == NULL || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    backend = (struct picoui_backend_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (backend == NULL || backend->host_widget == NULL) {
        return false;
    }

    radial_menu = (struct picoui_radial_menu *)backend->host_widget;
    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= radial_menu->item_count) {
        return false;
    }

    previous_selected_index = radial_menu->selected_index;
    if (radial_menu->widget.visible == 0 || radial_menu->widget.enabled == 0) {
        return false;
    }

    if (picoui_backend_widget_claim_focus(backend) != 0) {
        return false;
    }

    radial_menu->selected_index = selected_index;
    backend->value = selected_index;
    if (previous_selected_index == selected_index) {
        return false;
    }
    backend->data_model_epoch++;
    backend->last_data_source = PICOUI_BACKEND_DATA_SOURCE_NATIVE_EVENT;
    backend->last_signal = PICOUI_BACKEND_SIGNAL_VALUE_CHANGED;
    backend->dispatch_count++;
    if (radial_menu->cb != 0) {
        radial_menu->cb(radial_menu, selected_index, radial_menu->user_data);
    }
    return false;
}

/**
 * @brief Create backend for radial menu
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @param[in] width Width in pixels
 * @param[in] height Height in pixels
 * @param[in] x_axis x axis
 * @param[in] y_axis y axis
 * @param[in] item_max item max
 */

void *picoui_backend_create_radial_menu(void *parent,
                                        const char *id,
                                        int width,
                                        int height,
                                        int x_axis,
                                        int y_axis,
                                        int item_max)
{
    struct picoui_backend_widget *widget;
    struct picoui_backend_widget *parent_widget = parent;
    struct picoui_backend_app_state *app_state;
    ldRadialMenu_t *ld_radial_menu;
    uint16_t name_id;

    if (parent == 0 || id == 0 || width <= 0 || height <= 0) {
        return 0;
    }

    if (x_axis <= 0) {
        x_axis = 1;
    }
    if (y_axis <= 0) {
        y_axis = 1;
    }
    if (item_max <= 0) {
        item_max = 1;
    }

    app_state = picoui_runtime_bridge_backend_state_from_parent(parent);
    if (app_state == NULL || app_state->ld_scene == NULL || parent_widget->ld_widget == NULL) {
        return 0;
    }

    widget = calloc(1, sizeof(*widget));
    if (widget == 0) {
        return 0;
    }

    name_id = picoui_runtime_bridge_next_name_id(parent);
    if (name_id == 0) {
        free(widget);
        return 0;
    }
    ld_radial_menu = ldRadialMenu_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent_widget->ld_name_id,
                                       0,
                                       0,
                                       (int16_t)width,
                                       (int16_t)height,
                                       (uint16_t)x_axis,
                                       (uint16_t)y_axis,
                                       (uint8_t)item_max);
    if (ld_radial_menu == NULL) {
        free(widget);
        return 0;
    }

    if (picoui_backend_widget_init_child(widget,
                                         parent,
                                         PICOUI_BACKEND_WIDGET_RADIAL_MENU,
                                         id,
                                         parent_widget->theme) != 0) {
        ldRadialMenu_depose(app_state->ld_scene, ld_radial_menu);
        free(widget);
        return 0;
    }
    widget->ld_widget = ld_radial_menu;
    widget->ld_name_id = name_id;
    widget->value = -1;
    widget->last_signal = PICOUI_BACKEND_SIGNAL_NONE;
    if (picoui_backend_widget_attach_child(parent, widget) != 0) {
        ldRadialMenu_depose(app_state->ld_scene, ld_radial_menu);
        free(widget);
        return 0;
    }
    return widget;
}

/**
 * @brief radial: menu add item
 *
 * @param[in] backend_widget backend widget
 * @param[in] id Widget identifier string
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_add_item(void *backend_widget, const char *id)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;
    int index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        id == NULL ||
        widget->list_item_count >= PICOUI_BACKEND_RADIAL_MENU_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    index = widget->list_item_count;
    ldRadialMenuAddItem(ld_radial_menu,
                        g_radial_menu_tiles[index % 5],
                        g_radial_menu_masks[index % 5]);
    widget->list_item_ids[index] = id;
    widget->list_item_count++;
    if (widget->value < 0) {
        widget->value = 0;
    }
    return 0;
}

/**
 * @brief radial: menu add item with source
 *
 * @param[in] backend_widget backend widget
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_add_item_with_source(void *backend_widget,
                                                    const char *id,
                                                    struct picoui_image_source *source)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;
    int index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        id == NULL ||
        source == NULL ||
        source->img_tile == NULL ||
        source->mask_tile == NULL ||
        widget->list_item_count >= PICOUI_BACKEND_RADIAL_MENU_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    index = widget->list_item_count;
    ldRadialMenuAddItem(ld_radial_menu, source->img_tile, source->mask_tile);
    widget->list_item_ids[index] = id;
    widget->list_item_count++;
    if (widget->value < 0) {
        widget->value = 0;
    }
    return 0;
}

/**
 * @brief radial: menu set selected index
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_set_selected_index(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)index);
    widget->value = index;
    return 0;
}

/**
 * @brief radial: menu get selected index
 *
 * @param[in] backend_widget backend widget
 * @return -1 on failure
 */

int picoui_backend_radial_menu_get_selected_index(void *backend_widget)
{
    ldRadialMenu_t *ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);

    if (ld_radial_menu == NULL) {
        return -1;
    }

    return (int)ld_radial_menu->selectItem;
}

/**
 * @brief radial: menu offset selection
 *
 * @param[in] backend_widget backend widget
 * @param[in] offset Offset
 * @return -1 on failure
 */

int picoui_backend_radial_menu_offset_selection(void *backend_widget, int offset)
{
    struct picoui_backend_widget *widget = backend_widget;
    int item_count;
    int next_index;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        widget->list_item_count <= 0) {
        return -1;
    }

    item_count = widget->list_item_count;
    next_index = picoui_backend_radial_menu_get_selected_index(backend_widget);
    if (next_index < 0) {
        next_index = 0;
    }
    next_index = (next_index + offset) % item_count;
    if (next_index < 0) {
        next_index += item_count;
    }

    return picoui_backend_radial_menu_set_selected_index(backend_widget, next_index);
}

/**
 * @brief radial: menu set default item
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_set_default_item(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)index);
    widget->value = index;
    return 0;
}

/**
 * @brief radial: menu click item
 *
 * @param[in] backend_widget backend widget
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_click_item(void *backend_widget, int index)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        index < 0 ||
        index >= widget->list_item_count) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    ldRadialMenuSetClickItem(ld_radial_menu, (uint8_t)index);
    widget->value = index;
    return 0;
}

/**
 * @brief radial: menu offset item
 *
 * @param[in] backend_widget backend widget
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_offset_item(void *backend_widget, int offset)
{
    struct picoui_backend_widget *widget = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (widget == NULL ||
        widget->kind != PICOUI_BACKEND_WIDGET_RADIAL_MENU ||
        widget->ld_widget == NULL ||
        widget->list_item_count <= 0) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    ldRadialMenuSetOffsetItem(ld_radial_menu, (int8_t)offset);
    return 0;
}

/**
 * @brief radial: menu bind host
 *
 * @param[in] backend_widget backend widget
 * @return 0 on success, -1 on failure
 */

int picoui_backend_radial_menu_bind_host(void *backend_widget)
{
    struct picoui_backend_widget *backend = backend_widget;
    ldRadialMenu_t *ld_radial_menu;

    if (backend == NULL) {
        return -1;
    }

    ld_radial_menu = picoui_backend_radial_menu_get_ld(backend_widget);
    if (ld_radial_menu == NULL) {
        return -1;
    }

    if (!ldMsgConnect(ld_radial_menu, SIGNAL_CLICKED_ITEM, picoui_backend_radial_menu_native_slot)) {
        return -1;
    }
    return 0;
}
