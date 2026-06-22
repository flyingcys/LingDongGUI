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
#include "radial_menu.h"
#include "widget.h"
#include "../core/runtime_bridge.h"
#include "../../../src/gui/ldBase.h"
#include "../../../src/gui/ldRadialMenu.h"
#include "../../../src/misc/ldMsg.h"

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

#define TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS 5

/* C1: backend_widget is gone; pass struct tinyui_widget * directly */
static ldRadialMenu_t *tinyui_radial_menu_get_ld(struct tinyui_widget *w)
{
    if (w == 0 || w->ld_widget == 0 || w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU) {
        return 0;
    }

    return (ldRadialMenu_t *)w->ld_widget;
}

static int tinyui_radial_menu_backend_set_selected_index(struct tinyui_widget *w, int index);

static bool tinyui_radial_menu_native_slot(struct ld_scene_t *scene, ldMsg_t msg)
{
    /* C1-T7: pInfo now points to tinyui_widget, not tinyui_backend_widget */
    struct tinyui_widget *w;
    struct tinyui_radial_menu *radial_menu;
    int selected_index;
    int previous_selected_index;

    (void)scene;

    if (msg.ptSender == 0 || msg.signal != SIGNAL_CLICKED_ITEM) {
        return false;
    }

    w = (struct tinyui_widget *)((ldBase_t *)msg.ptSender)->pInfo;
    if (w == 0) {
        return false;
    }

    radial_menu = (struct tinyui_radial_menu *)w;
    selected_index = (int)msg.value;
    if (selected_index < 0 || selected_index >= radial_menu->item_count) {
        return false;
    }

    previous_selected_index = radial_menu->selected_index;
    if (radial_menu->widget.visible == 0 || radial_menu->widget.enabled == 0) {
        return false;
    }

    if (tinyui_widget_claim_backend_focus(w) != 0) {
        return false;
    }

    radial_menu->selected_index = selected_index;
    w->value = selected_index;
    if (previous_selected_index == selected_index) {
        return false;
    }
    if (radial_menu->cb != 0) {
        radial_menu->cb(radial_menu, selected_index, radial_menu->user_data);
    }
    return false;
}

static int tinyui_radial_menu_backend_add_item(struct tinyui_widget *w, const char *id)
{
    ldRadialMenu_t *ld_radial_menu;
    int index;

    if (w == 0 ||
        w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        w->ld_widget == 0 ||
        id == 0 ||
        (int)w->list_item_count >= TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_radial_menu = tinyui_radial_menu_get_ld(w);
    if (ld_radial_menu == 0) {
        return -1;
    }

    index = (int)w->list_item_count;
    ldRadialMenuAddItem(ld_radial_menu,
                        g_radial_menu_tiles[index % 5],
                        g_radial_menu_masks[index % 5]);
    w->list_item_count++;
    if (w->value < 0) {
        w->value = 0;
    }
    return 0;
}

static int tinyui_radial_menu_backend_add_item_with_source(struct tinyui_widget *w,
                                                           const char *id,
                                                           struct tinyui_image_source *source)
{
    ldRadialMenu_t *ld_radial_menu;
    int index;

    if (w == 0 ||
        w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        w->ld_widget == 0 ||
        id == 0 ||
        source == 0 ||
        source->img_tile == 0 ||
        source->mask_tile == 0 ||
        (int)w->list_item_count >= TINYUI_RADIAL_MENU_NATIVE_MAX_ITEMS) {
        return -1;
    }

    ld_radial_menu = tinyui_radial_menu_get_ld(w);
    if (ld_radial_menu == 0) {
        return -1;
    }

    index = (int)w->list_item_count;
    ldRadialMenuAddItem(ld_radial_menu, source->img_tile, source->mask_tile);
    w->list_item_count++;
    if (w->value < 0) {
        w->value = 0;
    }
    return 0;
}

static int tinyui_radial_menu_backend_set_selected_index(struct tinyui_widget *w, int index)
{
    ldRadialMenu_t *ld_radial_menu;

    if (w == 0 ||
        w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        w->ld_widget == 0 ||
        index < 0 ||
        index >= (int)w->list_item_count) {
        return -1;
    }

    ld_radial_menu = tinyui_radial_menu_get_ld(w);
    if (ld_radial_menu == 0) {
        return -1;
    }

    ldRadialMenuSetDefaultItem(ld_radial_menu, (uint8_t)index);
    w->value = index;
    return 0;
}

static int tinyui_radial_menu_backend_get_selected_index(struct tinyui_widget *w)
{
    ldRadialMenu_t *ld_radial_menu = tinyui_radial_menu_get_ld(w);

    if (ld_radial_menu == 0) {
        return -1;
    }

    return (int)ld_radial_menu->selectItem;
}

static int tinyui_radial_menu_backend_offset_selection(struct tinyui_widget *w, int offset)
{
    int item_count;
    int next_index;

    if (w == 0 ||
        w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        w->ld_widget == 0 ||
        (int)w->list_item_count <= 0) {
        return -1;
    }

    item_count = (int)w->list_item_count;
    next_index = tinyui_radial_menu_backend_get_selected_index(w);
    if (next_index < 0) {
        next_index = 0;
    }
    next_index = (next_index + offset) % item_count;
    if (next_index < 0) {
        next_index += item_count;
    }

    return tinyui_radial_menu_backend_set_selected_index(w, next_index);
}

static int tinyui_radial_menu_backend_set_default_item(struct tinyui_widget *w, int index)
{
    return tinyui_radial_menu_backend_set_selected_index(w, index);
}

static int tinyui_radial_menu_backend_click_item(struct tinyui_widget *w, int index)
{
    ldRadialMenu_t *ld_radial_menu;

    if (w == 0 ||
        w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        w->ld_widget == 0 ||
        index < 0 ||
        index >= (int)w->list_item_count) {
        return -1;
    }

    ld_radial_menu = tinyui_radial_menu_get_ld(w);
    if (ld_radial_menu == 0) {
        return -1;
    }

    ldRadialMenuSetClickItem(ld_radial_menu, (uint8_t)index);
    w->value = index;
    return 0;
}

static int tinyui_radial_menu_backend_offset_item(struct tinyui_widget *w, int offset)
{
    ldRadialMenu_t *ld_radial_menu;

    if (w == 0 ||
        w->kind != TINYUI_BACKEND_WIDGET_RADIAL_MENU ||
        w->ld_widget == 0 ||
        (int)w->list_item_count <= 0) {
        return -1;
    }

    ld_radial_menu = tinyui_radial_menu_get_ld(w);
    if (ld_radial_menu == 0) {
        return -1;
    }

    ldRadialMenuSetOffsetItem(ld_radial_menu, (int8_t)offset);
    return 0;
}

static int tinyui_radial_menu_bind_host(struct tinyui_widget *w)
{
    ldRadialMenu_t *ld_radial_menu;

    if (w == 0) {
        return -1;
    }

    ld_radial_menu = tinyui_radial_menu_get_ld(w);
    if (ld_radial_menu == 0) {
        return -1;
    }

    if (!ldMsgConnect(ld_radial_menu, SIGNAL_CLICKED_ITEM, tinyui_radial_menu_native_slot)) {
        return -1;
    }
    return 0;
}

static int tinyui_radial_menu_props_are_valid(const struct tinyui_radial_menu_props *props)
{
    return props != 0 &&
           props->id != 0 &&
           props->width >= 0 &&
           props->height >= 0 &&
           props->x_axis >= 0 &&
           props->y_axis >= 0 &&
           props->item_max >= 0 &&
           props->default_index >= -1;
}

static struct tinyui_radial_menu *tinyui_radial_menu_create_with_backend_config(struct tinyui_widget *parent,
                                                                                const char *id,
                                                                                int width,
                                                                                int height,
                                                                                int x_axis,
                                                                                int y_axis,
                                                                                int item_max)
{
    struct tinyui_radial_menu *radial_menu;
    struct tinyui_app *app_state;
    ldRadialMenu_t *ld_radial_menu;
    uint16_t name_id;

    if (parent == 0 || id == 0) {
        return 0;
    }

    if (width <= 0 || height <= 0) {
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

    /* C1: app_state and ld fields are directly on parent widget */
    app_state = parent->owner;
    if (parent->ld_widget == 0 || app_state == 0 || app_state->ld_scene == 0) {
        return 0;
    }

    radial_menu = calloc(1, sizeof(*radial_menu));
    if (radial_menu == 0) {
        return 0;
    }

    name_id = ++app_state->next_ld_name_id;
    if (name_id == 0) {
        free(radial_menu);
        return 0;
    }

    ld_radial_menu = ldRadialMenu_init(app_state->ld_scene,
                                       NULL,
                                       name_id,
                                       parent->ld_name_id,
                                       0,
                                       0,
                                       (int16_t)width,
                                       (int16_t)height,
                                       (uint16_t)x_axis,
                                       (uint16_t)y_axis,
                                       (uint8_t)item_max);
    if (ld_radial_menu == 0) {
        free(radial_menu);
        return 0;
    }

    /* C1: set ld fields directly on widget (no separate backend allocation) */
    radial_menu->widget.ld_widget = ld_radial_menu;
    radial_menu->widget.ld_name_id = name_id;
    radial_menu->widget.kind = TINYUI_BACKEND_WIDGET_RADIAL_MENU;
    radial_menu->widget.owner = app_state;
    radial_menu->widget.value = -1;
    radial_menu->widget.visible = 1;
    radial_menu->widget.enabled = 1;
    ((ldBase_t *)ld_radial_menu)->pInfo = &radial_menu->widget;
    tinyui_runtime_bridge_bind_leaf_widget(&radial_menu->widget, app_state);

    radial_menu->id = id;
    radial_menu->selected_index = -1;
    radial_menu->x_axis = x_axis;
    radial_menu->y_axis = y_axis;
    radial_menu->item_max = item_max;

    if (tinyui_radial_menu_bind_host(&radial_menu->widget) != 0) {
        ldRadialMenu_depose(app_state->ld_scene, ld_radial_menu);
        free(radial_menu);
        return 0;
    }

    return radial_menu;
}

/**
 * @brief Create radial menu widget
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_radial_menu *tinyui_radial_menu_create(struct tinyui_widget *parent, const char *id)
{
    return tinyui_radial_menu_create_with_backend_config(parent, id, 194, 96, 68, 46, 5);
}

/**
 * @brief radial menu init
 *
 * @param[in] parent Parent widget
 * @param[in] id Widget identifier string
 * @return Pointer to the object
 */

struct tinyui_radial_menu *tinyui_radial_menu_init(struct tinyui_widget *parent, const char *id)
{
    return tinyui_radial_menu_create(parent, id);
}

/**
 * @brief Create radial menu widget with properties
 *
 * @param[in] parent Parent widget
 * @param[in] props Properties structure
 * @return Pointer to the object on success, NULL on failure
 */

struct tinyui_radial_menu *tinyui_radial_menu_create_with_props(
    struct tinyui_widget *parent,
    const struct tinyui_radial_menu_props *props
)
{
    struct tinyui_radial_menu *radial_menu;

    if (!tinyui_radial_menu_props_are_valid(props)) {
        return 0;
    }

    radial_menu = tinyui_radial_menu_create_with_backend_config(parent,
                                                                props->id,
                                                                props->width > 0 ? props->width : 194,
                                                                props->height > 0 ? props->height : 96,
                                                                props->x_axis > 0 ? props->x_axis : 68,
                                                                props->y_axis > 0 ? props->y_axis : 46,
                                                                props->item_max > 0 ? props->item_max : 5);
    if (radial_menu == 0) {
        return 0;
    }

    if (tinyui_widget_set_user_data(&radial_menu->widget, props->user_data) != 0 ||
        (props->style_class != 0 &&
         tinyui_widget_set_style_class(&radial_menu->widget, props->style_class) != 0) ||
        ((props->width > 0 || props->height > 0) &&
         tinyui_widget_set_size(&radial_menu->widget, props->width, props->height) != 0)) {
        free(radial_menu);
        return 0;
    }

    if (props->default_index >= 0) {
        radial_menu->selected_index = props->default_index;
    }
    return radial_menu;
}

/**
 * @brief radial menu add item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_add_item(struct tinyui_radial_menu *radial_menu, const char *id)
{
    int index;

    if (radial_menu == 0 || id == 0 || radial_menu->item_count >= TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (tinyui_radial_menu_backend_add_item(&radial_menu->widget, id) != 0) {
        return -1;
    }

    index = radial_menu->item_count++;
    radial_menu->items[index].id = id;
    radial_menu->items[index].text = id;
    if (radial_menu->selected_index < 0) {
        radial_menu->selected_index = 0;
    }
    if (radial_menu->selected_index >= 0 &&
        radial_menu->selected_index < radial_menu->item_count &&
        tinyui_radial_menu_backend_set_selected_index(&radial_menu->widget,
                                                      radial_menu->selected_index) != 0) {
        radial_menu->item_count--;
        radial_menu->items[index].id = 0;
        radial_menu->items[index].text = 0;
        if (radial_menu->item_count == 0) {
            radial_menu->selected_index = -1;
        }
        return -1;
    }
    return 0;
}

/**
 * @brief radial menu add item with source
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_add_item_with_source(struct tinyui_radial_menu *radial_menu,
                                            const char *id,
                                            struct tinyui_image_source *source)
{
    int index;

    if (radial_menu == 0
        || id == 0
        || source == 0
        || source->img_tile == 0
        || source->mask_tile == 0
        || radial_menu->item_count >= TINYUI_LIST_MAX_ITEMS) {
        return -1;
    }

    if (tinyui_radial_menu_backend_add_item_with_source(&radial_menu->widget, id, source) != 0) {
        return -1;
    }

    index = radial_menu->item_count++;
    radial_menu->items[index].id = id;
    radial_menu->items[index].text = id;
    radial_menu->item_sources[index] = source;
    if (radial_menu->selected_index < 0) {
        radial_menu->selected_index = 0;
    }
    if (radial_menu->selected_index >= 0 &&
        radial_menu->selected_index < radial_menu->item_count &&
        tinyui_radial_menu_backend_set_selected_index(&radial_menu->widget,
                                                      radial_menu->selected_index) != 0) {
        radial_menu->item_count--;
        radial_menu->items[index].id = 0;
        radial_menu->items[index].text = 0;
        radial_menu->item_sources[index] = 0;
        if (radial_menu->item_count == 0) {
            radial_menu->selected_index = -1;
        }
        return -1;
    }
    return 0;
}

/**
 * @brief radial menu add item with image
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] id Widget identifier string
 * @param[in] source Image source
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_add_item_with_image(struct tinyui_radial_menu *radial_menu,
                                           const char *id,
                                           struct tinyui_image_source *source)
{
    return tinyui_radial_menu_add_item_with_source(radial_menu, id, source);
}

/**
 * @brief Set selected index of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_set_selected_index(struct tinyui_radial_menu *radial_menu, int index)
{
    if (radial_menu == 0 || index < 0 || index >= radial_menu->item_count) {
        return -1;
    }

    if (tinyui_radial_menu_backend_set_selected_index(&radial_menu->widget, index) != 0) {
        return -1;
    }

    radial_menu->selected_index = index;
    return 0;
}

/**
 * @brief Get selected index of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @return -1 on failure
 */

int tinyui_radial_menu_get_selected_index(const struct tinyui_radial_menu *radial_menu)
{
    int selected_index;

    if (radial_menu == 0) {
        return -1;
    }

    selected_index =
        tinyui_radial_menu_backend_get_selected_index((struct tinyui_widget *)&radial_menu->widget);
    if (selected_index >= 0 && selected_index < radial_menu->item_count) {
        ((struct tinyui_radial_menu *)radial_menu)->selected_index = selected_index;
        return selected_index;
    }

    return radial_menu->selected_index;
}

/**
 * @brief radial menu offset selection
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_offset_selection(struct tinyui_radial_menu *radial_menu, int offset)
{
    int selected_index;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return -1;
    }

    if (tinyui_radial_menu_backend_offset_selection(&radial_menu->widget, offset) != 0) {
        return -1;
    }

    selected_index = tinyui_radial_menu_backend_get_selected_index(&radial_menu->widget);
    if (selected_index >= 0 && selected_index < radial_menu->item_count) {
        radial_menu->selected_index = selected_index;
    }
    return 0;
}

/**
 * @brief Set default item of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_set_default_item(struct tinyui_radial_menu *radial_menu, int index)
{
    if (radial_menu == 0 || index < 0 || index >= radial_menu->item_count) {
        return -1;
    }

    if (tinyui_radial_menu_backend_set_default_item(&radial_menu->widget, index) != 0) {
        return -1;
    }

    radial_menu->selected_index = index;
    return 0;
}

/**
 * @brief radial menu click item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_click_item(struct tinyui_radial_menu *radial_menu, int index)
{
    if (radial_menu == 0 || index < 0 || index >= radial_menu->item_count) {
        return -1;
    }

    if (tinyui_radial_menu_backend_click_item(&radial_menu->widget, index) != 0) {
        return -1;
    }

    radial_menu->selected_index = index;
    return 0;
}

/**
 * @brief Set click item of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] index Index
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_set_click_item(struct tinyui_radial_menu *radial_menu, int index)
{
    return tinyui_radial_menu_click_item(radial_menu, index);
}

/**
 * @brief radial menu offset item
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] offset Offset
 * @return 0 on success, -1 on failure
 */

int tinyui_radial_menu_offset_item(struct tinyui_radial_menu *radial_menu, int offset)
{
    int selected_index;

    if (radial_menu == 0 || radial_menu->item_count <= 0) {
        return -1;
    }

    if (tinyui_radial_menu_backend_offset_item(&radial_menu->widget, offset) != 0) {
        return -1;
    }

    selected_index = tinyui_radial_menu_backend_get_selected_index(&radial_menu->widget);
    if (selected_index >= 0 && selected_index < radial_menu->item_count) {
        radial_menu->selected_index = selected_index;
    }
    return 0;
}

/**
 * @brief Set on selected of radial menu widget
 *
 * @param[in] radial_menu Radial menu widget instance
 * @param[in] user_data) user data)
 * @param[in] user_data User data pointer
 */

void tinyui_radial_menu_set_on_selected(struct tinyui_radial_menu *radial_menu,
                                        void (*callback)(struct tinyui_radial_menu *radial_menu,
                                                         int index,
                                                         void *user_data),
                                        void *user_data)
{
    if (radial_menu == 0) {
        return;
    }

    radial_menu->cb = callback;
    radial_menu->user_data = user_data;
}
